const host = window.location.hostname === "localhost" ? "localhost" : window.location.hostname;
const ws = new WebSocket(`ws://${host}:8765`);

const select = document.getElementById("port-select");
const selectFile = document.getElementById("firmware-select")
const connectButton = document.querySelector("button[onclick='connectESP32()']");
const disconnectButton = document.querySelector("button[onclick='disconnectESP32()']");
const flashButton = document.querySelector("button[onclick='flashFirmware()']");

const inputSerial = document.getElementById('serial-input');
const outputSerial = document.getElementById('serial-output');

let connectedPort = null;
let autoScroll = true;
let flashInProgress = false;

ws.onopen = () => {
  console.log("WebSocket connecté !");
  ws.send(JSON.stringify({ type: "get_ports" }));
  ws.send(JSON.stringify({ type: "get_files" }));
  disconnectButton.disabled = true;
  flashButton.disabled = false;
};

ws.onmessage = (event) => {
  const msg = JSON.parse(event.data);

  if (msg.type === "ports") {
    if (!connectedPort) {
      select.innerHTML = "";
      msg.ports.forEach(port => {
        const option = document.createElement("option");
        option.value = port;
        option.textContent = port;
        select.appendChild(option);
      });
      select.disabled = false;
      connectButton.disabled = false;
    }
  }

    if (msg.type === "files") {
      selectFile.innerHTML = "";
      msg.files.forEach(file => {
        const option = document.createElement("option");
        option.value = file;
        option.textContent = file;
        selectFile.appendChild(option);
      });
  }

  if (msg.type === "connect_ack") {
    if (msg.success) {
      connectedPort = msg.port;
      select.disabled = true;
      connectButton.disabled = true;
      disconnectButton.disabled = false;
      flashButton.disabled = true;
    } else {
      alert(`Erreur connexion port ${msg.port} : ${msg.error}`);
    }
  }

  if (msg.type === "disconnect_ack") {
    if (msg.success) {
      select.disabled = false;
      connectButton.disabled = false;
      disconnectButton.disabled = true;
      flashButton.disabled = false;
    } else {
      alert(`Erreur pas de port connecté`);
    }
  }

  if (msg.type === "serialMessage_error") {
    alert(`Erreur : ${msg.error}`);
  }

  if (msg.type === "serial_read") {
    outputSerial.innerHTML += `<span class="text-yellow-300">Port COM : ${msg.data}</span><br>`;
    if (autoScroll === true) {
      outputSerial.scrollTop = outputSerial.scrollHeight;
    }
  }

  if (msg.type === "serial_read_error") {
    alert(`Erreur read serial : ${msg.error}`);
  }

  if (msg.type === "file_error") {
    alert(`Erreur file : ${msg.error}`);
  }

  if (msg.type === "upload_complete") {
    //alert(`Firmware téléversé avec succès !`);
  }

  if (msg.type === "delete_file_ack") {
    if (msg.success) {
      //alert(`Firmware "${msg.filename}" supprimé avec succès.`);
      // Rafraîchir la liste des fichiers
      ws.send(JSON.stringify({ type: "get_files" }));
    } else {
      alert(`Erreur lors de la suppression : ${msg.error}`);
    }
  }

  // NOUVEAUX HANDLERS POUR LE FLASHAGE
  if (msg.type === "flash_progress") {
    console.log(`[FLASH] ${msg.message} - ${msg.progress}%`);
    outputSerial.innerHTML += `<span class="text-blue-300">[FLASH] ${msg.message}</span><br>`;
    if (autoScroll === true) {
      outputSerial.scrollTop = outputSerial.scrollHeight;
    }

    // Optionnel : mettre à jour une barre de progression si vous en avez une
    // updateProgressBar(msg.progress);
  }

  if (msg.type === "flash_log") {
    console.log(`[FLASH LOG] ${msg.message}`);
    outputSerial.innerHTML += `<span class="text-gray-300">[ESPTOOL] ${msg.message}</span><br>`;
    if (autoScroll === true) {
      outputSerial.scrollTop = outputSerial.scrollHeight;
    }
  }

  if (msg.type === "flash_complete") {
    flashInProgress = false;
    flashButton.disabled = false;
    connectButton.disabled = false;

    if (msg.success) {
      console.log("[FLASH] Flashage terminé avec succès !");
      outputSerial.innerHTML += `<span class="text-green-300">[SUCCESS] ${msg.message}</span><br>`;
      alert("Flashage terminé avec succès !");
    } else {
      console.error(`[FLASH] Erreur : ${msg.message}`);
      outputSerial.innerHTML += `<span class="text-red-300">[ERROR] ${msg.message}</span><br>`;
      alert(`Erreur de flashage : ${msg.message}`);
    }

    if (autoScroll === true) {
      outputSerial.scrollTop = outputSerial.scrollHeight;
    }
  }

};


window.connectESP32 = function () {
  const port = select.value;
  if (!port) {
    alert("Sélectionnez un port COM avant de connecter.");
  } else {
    ws.send(JSON.stringify({ type: "connect", port }));
  }
};

document.getElementById("autoscroll-toggle").addEventListener("change", (e) => {
  autoScroll = e.target.checked;
});

window.disconnectESP32 = function () {
  ws.send(JSON.stringify({ type: "disconnect" }));
};

window.sendSerialCommand = function () {
  const cmd = inputSerial.value.trim();
  if (cmd !== "") {
    outputSerial.innerHTML += `> ${cmd}<br>`;
    inputSerial.value = "";
    ws.send(JSON.stringify({ type: "serialMessage", message: cmd }));
  }
};

window.clearConsole = function () {
  document.getElementById("serial-output").innerHTML = "> Console ...<br>";
};

window.refreshPorts = function () {
  ws.send(JSON.stringify({ type: "get_ports" }));
};

window.refreshFirmware = function () {
  ws.send(JSON.stringify({ type: "get_files" }));
};

window.deleteFirmware = function () {
  const selectedFile = selectFile.value;

  if (!selectedFile || selectedFile === "NONE") {
    alert("Veuillez sélectionner un firmware à supprimer.");
    return;
  }

  if (confirm(`Êtes-vous sûr de vouloir supprimer le firmware "${selectedFile}" ?`)) {
    ws.send(JSON.stringify({
      type: "delete_file",
      filename: selectedFile
    }));
  }
};


window.flashFirmware = async function () {
  const selectedFirmware = selectFile.value;
  const selectedPort = select.value;

  // Vérifications
  if (!selectedFirmware || selectedFirmware === "NONE") {
    alert("Veuillez sélectionner un firmware à flasher.");
    return;
  }

  if (!selectedPort) {
    alert("Veuillez sélectionner un port COM.");
    return;
  }

  if (flashInProgress) {
    alert("Un flashage est déjà en cours...");
    return;
  }

  // Confirmation
  if (!confirm(`Êtes-vous sûr de vouloir flasher le firmware "${selectedFirmware}" sur le port ${selectedPort}?\n\nCela va écraser le firmware actuel de l'ESP32.`)) {
    return;
  }

  // Démarrer le flashage
  flashInProgress = true;
  flashButton.disabled = true;
  connectButton.disabled = true;

  // Message de démarrage dans la console
  outputSerial.innerHTML += `<span class="text-cyan-300">[INFO] Démarrage du flashage de "${selectedFirmware}" sur ${selectedPort}...</span><br>`;
  if (autoScroll === true) {
    outputSerial.scrollTop = outputSerial.scrollHeight;
  }

  console.log(`[DEBUG] Envoi de la commande de flashage : firmware=${selectedFirmware}, port=${selectedPort}`);

  ws.send(JSON.stringify({
    type: "flash_firmware",
    firmware: selectedFirmware,
    port: selectedPort
  }));
};

window.UploadFirmware = async function () {
  const firmware = document.getElementById('firmware-upload-file').files[0];

  if (!firmware) return alert('Aucun firmware sélectionné.');

  try {

    const formData = new FormData();
    formData.append("firmware", firmware);


    const uploadHost = window.location.hostname === "localhost" ? "localhost" : window.location.hostname;
   // const uploadHost = "192.168.80.216";
    const response = await fetch(`http://${uploadHost}:8080/upload`, {
      method: "POST",
      body: formData
    });

    const result = await response.json();

    ws.send(JSON.stringify({ type: "get_files" }));

    if (result.status !== "success") {
      alert("Erreur serveur : " + result.error);
    }

  } catch (err) {

    alert(err);

  }
};
