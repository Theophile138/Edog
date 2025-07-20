import asyncio
import websockets
import json
import os
from aiohttp import web
from websocket_handler import websocket_handler
from datetime import datetime

# Serveur HTTP pour les uploads
async def handle_upload(request):
    """Endpoint HTTP pour upload de gros fichiers"""
    try:
        # Vérifier que c'est bien du multipart
        content_type = request.headers.get('Content-Type', '')
        if not content_type.startswith('multipart/'):
            return web.json_response({
                "status": "error",
                "error": "Content-Type doit être multipart/form-data"
            }, status=400)

        reader = await request.multipart()
        firmware_data = None
        port = None
        filename = "firmware.bin"

        while True:
            part = await reader.next()
            if part is None:
                break

            if part.name == 'firmware':
                filename = part.filename or "firmware.bin"
                firmware_data = await part.read()
                print(f"[DEBUG] Firmware reçu: {filename} ({len(firmware_data)} bytes)")

        if not firmware_data:
            return web.json_response({
                "status": "error",
                "error": "Aucun firmware trouvé"
            }, status=400)

        current_date = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        dateFileName = f"{current_date}_{filename}"
        filepath = os.path.join('firmware', dateFileName)

        with open(filepath, 'wb') as f:
            f.write(firmware_data)

        print(f"[SUCCESS] Firmware sauvé: {dateFileName} ({len(firmware_data)} bytes)")

        return web.json_response({
            "status": "success",
            "message": f"Firmware uploadé avec succès",
            "filename": dateFileName,  # Retourner le nouveau nom avec la date
            "size": len(firmware_data),
        })

    except Exception as e:
        print(f"[ERROR] Upload failed: {e}")
        return web.json_response({
            "status": "error",
            "error": str(e)
        }, status=500)


async def handle_status(request):
    """Endpoint pour vérifier le status du serveur"""
    return web.json_response({
        "status": "ok",
        "message": "Serveur ESP32 opérationnel"
    })


async def create_http_server():
    """Créer le serveur HTTP"""
    app = web.Application(client_max_size=50 * 1024 * 1024)  # 50MB max

    app.router.add_post('/upload', handle_upload)
    app.router.add_get('/status', handle_status)

    @web.middleware
    async def cors_handler(request, handler):
        response = await handler(request)
        response.headers['Access-Control-Allow-Origin'] = '*'
        response.headers['Access-Control-Allow-Methods'] = 'GET, POST, OPTIONS'
        response.headers['Access-Control-Allow-Headers'] = 'Content-Type'
        return response

    app.middlewares.append(cors_handler)

    return app


async def websocket_server():
    """Serveur WebSocket"""
    async with websockets.serve(websocket_handler, "0.0.0.0", 8765):
        print("Serveur WebSocket lancé sur ws://localhost:8765")
        await asyncio.Future()  # Run forever


async def http_server():
    """Serveur HTTP"""
    app = await create_http_server()
    runner = web.AppRunner(app)
    await runner.setup()

    site = web.TCPSite(runner, '0.0.0.0', 8080)
    await site.start()
    print("Serveur HTTP lancé sur http://localhost:8080")

    await asyncio.Future()  # Run forever

async def main():
    """Lancer les deux serveurs en parallèle"""
    print("Démarrage des serveurs ESP32...")

    # Lancer WebSocket et HTTP en parallèle
    await asyncio.gather(
        websocket_server(),
        http_server()
    )


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nServeurs arrêtés")