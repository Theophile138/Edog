import asyncio
import json
import os
import tempfile
from serial_manager import SerialManager
import math
import base64
import subprocess
import platform
import shutil
import sys

serial_mgr = SerialManager()

ongoing_uploads = {}


def list_firmware_files():
    firmware_dir = 'firmware'

    if not os.path.exists(firmware_dir):
        return []

    files = []
    for item in os.listdir(firmware_dir):
        item_path = os.path.join(firmware_dir, item)
        if os.path.isfile(item_path):
            files.append(item)

    return files

async def flash_esp32_firmware(websocket, firmware_filename, port):
    """Flash l'ESP32 avec esptool, incluant le bootloader si disponible."""
    try:
        firmware_path = os.path.join('firmware', firmware_filename)
        partition_path = os.path.join('firmware', 'Environement', 'esp32dev', 'partitions.bin')
        bootloader_dir = os.path.dirname(partition_path)

        await websocket.send(json.dumps({
            "type": "flash_log",
            "message": "Vérification des fichiers..."
        }))

        # Recherche du bootloader
        bootloader_path = None
        for f in os.listdir(bootloader_dir):
            if f.startswith("bootloader") and f.endswith(".bin"):
                bootloader_path = os.path.join(bootloader_dir, f)
                break

        if not os.path.exists(firmware_path):
            raise FileNotFoundError(f"Firmware non trouvé: {firmware_path}")
        if not os.path.exists(partition_path):
            raise FileNotFoundError(f"Partition non trouvée: {partition_path}")
        if not bootloader_path or not os.path.exists(bootloader_path):
            raise FileNotFoundError("Bootloader introuvable dans le même dossier que partitions.bin")

        await websocket.send(json.dumps({
            "type": "flash_log",
            "message": f"Fichiers trouvés : bootloader={bootloader_path}, partition={partition_path}, firmware={firmware_path}"
        }))

        python_exec = sys.executable
        await websocket.send(json.dumps({
            "type": "flash_log",
            "message": f"Python utilisé: {python_exec}"
        }))

        try:
            result = await asyncio.create_subprocess_exec(
                python_exec, "-m", "esptool", "--help",
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            await result.communicate()
            if result.returncode != 0:
                raise RuntimeError("esptool introuvable")
            esptool_cmd_prefix = [python_exec, "-m", "esptool"]
        except Exception as e:
            await websocket.send(json.dumps({
                "type": "flash_complete",
                "success": False,
                "message": f"esptool non trouvé. Installez-le avec: pip install esptool. Erreur: {e}"
            }))
            return

        if serial_mgr.is_connected():
            await websocket.send(json.dumps({
                "type": "flash_log",
                "message": "Déconnexion du port série..."
            }))
            serial_mgr.disconnect()
            await asyncio.sleep(2)

        await websocket.send(json.dumps({
            "type": "flash_log",
            "message": f"Test de connexion à l'ESP32 sur {port}..."
        }))

        test_cmd = esptool_cmd_prefix + [
            "--chip", "esp32",
            "--port", port,
            "--baud", "115200",
            "chip_id"
        ]
        test_process = await asyncio.create_subprocess_exec(
            *test_cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.STDOUT
        )

        while True:
            line = await test_process.stdout.readline()
            if not line:
                break
            await websocket.send(json.dumps({
                "type": "flash_log",
                "message": f"[TEST] {line.decode().strip()}"
            }))

        await test_process.wait()
        if test_process.returncode != 0:
            await websocket.send(json.dumps({
                "type": "flash_complete",
                "success": False,
                "message": "Impossible de se connecter à l'ESP32. Mettez-le en mode bootloader."
            }))
            return

        await websocket.send(json.dumps({
            "type": "flash_log",
            "message": "Connexion ESP32 OK ! Démarrage du flashage..."
        }))

        esptool_cmd = esptool_cmd_prefix + [
            "--chip", "esp32",
            "--port", port,
            "--baud", "460800",
            "--before", "default_reset",
            "--after", "hard_reset",
            "write_flash",
            "-z",
            "--flash_mode", "dio",
            "--flash_freq", "40m",
            "--flash_size", "detect",
            "0x1000", bootloader_path,
            "0x8000", partition_path,
            "0x10000", firmware_path
        ]

        await websocket.send(json.dumps({
            "type": "flash_log",
            "message": f"Commande de flash : {' '.join(esptool_cmd)}"
        }))
        await websocket.send(json.dumps({
            "type": "flash_progress",
            "message": "Flashage en cours...",
            "progress": 0
        }))

        process = await asyncio.create_subprocess_exec(
            *esptool_cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.STDOUT
        )

        all_output = []
        while True:
            line = await process.stdout.readline()
            if not line:
                break
            line_str = line.decode(errors='ignore').strip()
            all_output.append(line_str)
            await websocket.send(json.dumps({
                "type": "flash_log",
                "message": line_str
            }))

            if "%" in line_str:
                try:
                    if "Writing" in line_str and "(" in line_str and "%" in line_str:
                        percentage = int(line_str.split("(")[1].split("%")[0])
                        await websocket.send(json.dumps({
                            "type": "flash_progress",
                            "message": f"Écriture... {percentage}%",
                            "progress": percentage
                        }))
                except (ValueError, IndexError):
                    pass

        await process.wait()
        if process.returncode == 0:
            await websocket.send(json.dumps({
                "type": "flash_progress",
                "message": "Flashage terminé !",
                "progress": 100
            }))
            await websocket.send(json.dumps({
                "type": "flash_complete",
                "success": True,
                "message": "Flashage réussi avec le bootloader !"
            }))
        else:
            output_text = "\n".join(all_output).lower()
            error_msg = "Erreur de flashage."

            if "failed to connect" in output_text:
                error_msg += " Vérifiez que l'ESP32 est bien en mode bootloader."
            elif "permission denied" in output_text:
                error_msg += " Accès refusé au port. Utilisé par un autre programme ?"
            elif "invalid head of packet" in output_text:
                error_msg += " En-tête de paquet invalide. Maintenez le bouton BOOT ?"

            await websocket.send(json.dumps({
                "type": "flash_complete",
                "success": False,
                "message": error_msg
            }))

    except FileNotFoundError as e:
        await websocket.send(json.dumps({
            "type": "flash_complete",
            "success": False,
            "message": f"Fichier manquant : {e}"
        }))
    except Exception as e:
        await websocket.send(json.dumps({
            "type": "flash_complete",
            "success": False,
            "message": f"Erreur inattendue : {str(e)}"
        }))

async def send_rotation_data(websocket):
    t = 0
    while True:
        data = {
            "rotationX1": 0,
            "rotationX2": math.pi / 4 - math.pi,
            "rotationX3": math.pi / 2
        }
        await websocket.send(json.dumps({"type": "rotation", "data": data}))
        await asyncio.sleep(0.01)
        t += 0.001


async def read_serial_loop(websocket):
    print("[DEBUG] read_serial_loop démarrée")
    while serial_mgr.is_connected():
        try:
            data = serial_mgr.read_available()
            if data:
                await websocket.send(json.dumps({
                    "type": "serial_read",
                    "data": data
                }))
        except Exception as e:
            await websocket.send(json.dumps({
                "type": "serial_read_error",
                "error": str(e)
            }))
            break
        await asyncio.sleep(0.01)


async def websocket_handler(websocket):
    send_task = asyncio.create_task(send_rotation_data(websocket))
    read_task = None
    client_id = id(websocket)

    try:
        async for message in websocket:
            try:
                msg = json.loads(message)
            except json.JSONDecodeError:
                continue

            if msg.get("type") == "get_ports":
                ports = serial_mgr.list_ports()
                await websocket.send(json.dumps({"type": "ports", "ports": ports}))

            elif msg.get("type") == "get_files":
                files = list_firmware_files()
                await websocket.send(json.dumps({"type": "files", "files": files}))

            elif msg.get("type") == "delete_file":
                try:
                    filename = msg.get("filename")
                    if not filename:
                        raise ValueError("Nom de fichier manquant")

                    filepath = os.path.join('firmware', filename)

                    if not os.path.exists(filepath):
                        raise FileNotFoundError(f"Le fichier {filename} n'existe pas")

                    os.remove(filepath)

                    await websocket.send(json.dumps({
                        "type": "delete_file_ack",
                        "success": True,
                        "filename": filename
                    }))

                    print(f"[SUCCESS] Firmware supprimé: {filename}")

                except Exception as e:
                    await websocket.send(json.dumps({
                        "type": "delete_file_ack",
                        "success": False,
                        "filename": msg.get("filename", "unknown"),
                        "error": str(e)
                    }))
                    print(f"[ERROR] Erreur suppression firmware: {e}")

            elif msg.get("type") == "connect":
                try:
                    serial_mgr.connect(msg.get("port"))
                    read_task = asyncio.create_task(read_serial_loop(websocket))
                    await websocket.send(json.dumps({
                        "type": "connect_ack",
                        "success": True,
                        "port": msg.get("port")
                    }))
                except Exception as e:
                    await websocket.send(json.dumps({
                        "type": "connect_ack",
                        "success": False,
                        "port": msg.get("port"),
                        "error": str(e)
                    }))

            elif msg.get("type") == "disconnect":
                serial_mgr.disconnect()
                if read_task:
                    read_task.cancel()
                await websocket.send(json.dumps({"type": "disconnect_ack", "success": True}))

            elif msg.get("type") == "serialMessage":
                try:
                    serial_mgr.send(msg.get("message", ""))
                except Exception as e:
                    await websocket.send(json.dumps({
                        "type": "serialMessage_error",
                        "error": str(e)
                    }))

            elif msg.get("type") == "flash_firmware":
                firmware_filename = msg.get("firmware")
                port = msg.get("port")

                if not firmware_filename:
                    await websocket.send(json.dumps({
                        "type": "flash_complete",
                        "success": False,
                        "message": "Aucun firmware sélectionné"
                    }))
                    continue

                if not port:
                    await websocket.send(json.dumps({
                        "type": "flash_complete",
                        "success": False,
                        "message": "Aucun port sélectionné"
                    }))
                    continue

                # Lancer le flashage en arrière-plan
                asyncio.create_task(flash_esp32_firmware(websocket, firmware_filename, port))

            elif msg.get("type") == "file_upload_start":
                try:
                    upload_info = {
                        "filename": msg.get("filename"),
                        "filesize": msg.get("filesize"),
                        "totalChunks": msg.get("totalChunks"),
                        "receivedChunks": 0,
                        "temp_file": tempfile.NamedTemporaryFile(delete=False),
                        "chunks_data": {}
                    }
                    ongoing_uploads[client_id] = upload_info
                    print(f"[DEBUG] Upload démarré: {upload_info['filename']} ({upload_info['filesize']} bytes)")

                except Exception as e:
                    await websocket.send(json.dumps({
                        "type": "file_error",
                        "error": f"Erreur initialisation upload: {str(e)}"
                    }))

    except Exception as e:
        print(f"[ERROR] Erreur WebSocket: {e}")
    finally:
        # Nettoyage
        serial_mgr.disconnect()
        send_task.cancel()
        if read_task:
            read_task.cancel()

        # Nettoyer les uploads en cours
        if client_id in ongoing_uploads:
            upload_info = ongoing_uploads[client_id]
            if 'temp_file' in upload_info:
                upload_info['temp_file'].close()
                try:
                    os.unlink(upload_info['temp_file'].name)
                except:
                    pass
            del ongoing_uploads[client_id]