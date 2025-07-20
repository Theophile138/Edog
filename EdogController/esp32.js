// esp32.js
//const ws = new WebSocket('ws://${window.location.hostname}:8765');

const host = window.location.hostname === "localhost" ? "localhost" : window.location.hostname;
const ws = new WebSocket(`ws://${host}:8765/ws`);

// Elements DOM
const select = document.getElementById("port-select");
const connectButton = document.querySelector("button[onclick='connectESP32()']");
const disconnectButton = document.querySelector("button[onclick='disconnectESP32()']");
const flashButton = document.querySelector("button[onclick='flashFirmware()']")

const inputSerial = document.getElementById('serial-input');
const outputSerial = document.getElementById('serial-output');

let connectedPort = null;

let autoScroll = true;

// À l'ouverture, demander la liste des ports COM
ws.onopen = () => {
  console.log("WebSocket connecté !");
  ws.send(JSON.stringify({ type: "get_ports" }));
  disconnectButton.disabled = true;
  flashButton.disabled = true;
};

ws.onmessage = (event) => {
  const msg = JSON.parse(event.data);

  if (msg.type === "ports") {
    if (!connectedPort) {
      // Remplir la liste seulement si pas encore connecté
      select.innerHTML = ""; // Vide la liste actuelle
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

  if (msg.type === "connect_ack") {
    if (msg.success) {
      connectedPort = msg.port;
      //alert(`Connecté avec succès au port ${connectedPort}`);
      select.disabled = true;          // Bloquer le select
      connectButton.disabled = true;   // Bloquer le bouton Connect.
      disconnectButton.disabled = false;
      flashButton.disabled = false;
    } else {
      alert(`Erreur connexion port ${msg.port} : ${msg.error}`);
    }
  }

  if (msg.type === "disconnect_ack"){
    if(msg.success){
      //alert("salut"):
      select.disabled = false;          // Bloquer le select
      connectButton.disabled = false;   // Bloquer le bouton Connect.
      disconnectButton.disabled = true;
      flashButton.disabled = true;
    }else{
      alert(`Erreur pas de port connecté`);
    }

  }

  if (msg.type === "serialMessage_error"){
    alert(`Erreur : ${msg.error}`);
  }

  if (msg.type === "serial_read"){
    outputSerial.innerHTML += `<span class="text-yellow-300">Port COM : ${msg.data}</span><br>`;

    if (autoScroll === true){
        outputSerial.scrollTop = outputSerial.scrollHeight;
    }

  }

  if (msg.type === "serial_read_error"){
    alert(`Erreur read serail : ${msg.error}`)
  }

    if (msg.type === "file_error"){
    alert(`Erreur file : ${msg.error}`)
  }

};

// Fonction appelée au clic sur "Connect ESP32"
window.connectESP32 = function () {
  const port = select.value;
  if (!port) {
    alert("Sélectionnez un port COM avant de connecter.");
  }else{
    ws.send(JSON.stringify({ type: "connect", port }));
  }
};

document.getElementById("autoscroll-toggle").addEventListener("change", (e) => {
  autoScroll = e.target.checked;
});

window.disconnectESP32 = function () {
     ws.send(JSON.stringify({ type: "disconnect" }));
     //alert("test")
};


      //function sendSerialCommand() {
      //  const input = document.getElementById('serial-input');
      //  const output = document.getElementById('serial-output');
      //  const cmd = input.value.trim();
      //  if (cmd === "") return;

      //  output.innerHTML += `> ${cmd}<br>`;
      //  input.value = "";

        // Simule une réponse pour le moment
      //  setTimeout(() => {
      //    output.innerHTML += `<span class="text-yellow-300">Réponse: OK</span><br>`;
      //    output.scrollTop = output.scrollHeight;
      //  }, 500);
      //}

window.sendSerialCommand = function(){
    const cmd = inputSerial.value.trim();
    if (cmd !== ""){
        outputSerial.innerHTML += `> ${cmd}<br>`;
        inputSerial.value = "";
        //outputSerial.innerHTML += `<span class="text-yellow-300">Réponse: OK</span><br>`;
        //outputSerial.scrollTop = output.scrollHeight;
        ws.send(JSON.stringify({ type: "serialMessage", message: cmd }));
    }
}

window.clearConsole = function () {
    document.getElementById("serial-output").innerHTML = "> Console ...<br>";
};

window.refreshPorts = function () {
    ws.send(JSON.stringify({ type: "get_ports" }));
};

window.flashFirmware = async function () {
  const firmware = document.getElementById('firmware-upload').files[0];
  const port = document.getElementById('port-select').value;

  if (!firmware) return alert('Aucun firmware sélectionné.');

  const formData = new FormData();
  formData.append("firmware", firmware);
  formData.append("port", port);

  try {
    const response = await fetch("/upload", {
      method: "POST",
      body: formData
    });

    const result = await response.json();

    if (result.status === "OK") {
      alert(`Firmware téléversé avec succès sur ${port} !`);
    } else {
      alert("Erreur serveur : " + result.error);
    }
  } catch (err) {
    alert("Erreur réseau : " + err.message);
  }
};
