// Charger les librairies JustGage et Raphael dynamiquement
function loadGaugeLibs(callback) {
  const raphael = document.createElement('script');
  raphael.src = "https://cdn.jsdelivr.net/npm/raphael@2.3.0/raphael.min.js";
  raphael.onload = () => {
    const justgage = document.createElement('script');
    justgage.src = "https://cdn.jsdelivr.net/npm/justgage@1.3.5/justgage.min.js";
    justgage.onload = callback;
    document.head.appendChild(justgage);
  };
  document.head.appendChild(raphael);
}

let motorsData = [
  { id: 1, current: 0, speed: 0, position: 0 },
  { id: 2, current: 0, speed: 0, position: 0 },
  { id: 3, current: 0, speed: 0, position: 0 },
  { id: 4, current: 0, speed: 0, position: 0 },
];

let gauges = [];

function initMotors() {
  const container = document.getElementById('motors-container');
  motorsData.forEach(motor => {
    const motorDiv = document.createElement('div');
    motorDiv.className = "bg-white p-4 rounded shadow";
    motorDiv.id = `motor${motor.id}-block`;
    motorDiv.innerHTML = `
      <h3 class="font-bold mb-2">Moteur ${motor.id}</h3>
      <p>Intensité : <span id="motor${motor.id}-current">${motor.current} A</span></p>
      <div id="motor${motor.id}-gauge" style="width:200px;height:120px;"></div>
      <p>Position : <span id="motor${motor.id}-position">${motor.position} °</span></p>
    `;
    container.appendChild(motorDiv);

    gauges[motor.id] = new JustGage({
      id: `motor${motor.id}-gauge`,
      value: motor.speed,
      min: 0,
      max: 5000,
      title: "Vitesse",
      label: "rpm",
      levelColors: ["#00ff00", "#ff0", "#f00"],
      gaugeWidthScale: 0.6,
      donut: true,
      pointer: true
    });
  });
}

function updateMotor(id, current, speed, position) {
  const currentEl = document.getElementById(`motor${id}-current`);
  const positionEl = document.getElementById(`motor${id}-position`);

  if (currentEl) currentEl.textContent = `${current.toFixed(2)} A`;
  if (positionEl) positionEl.textContent = `${position.toFixed(1)} °`;
  if (gauges[id]) gauges[id].refresh(speed);
}

function fetchMotorData() {
  motorsData.forEach(m => {
    m.current = Math.random() * 30;          
    m.speed = Math.random() * 5000;          
    m.position = Math.random() * 360;        
    updateMotor(m.id, m.current, m.speed, m.position);
  });
}

window.showMotors = function () {
  document.querySelectorAll('main section').forEach(sec => sec.classList.add('hidden'));
  document.getElementById('moteur').classList.remove('hidden');
  if (!document.getElementById('motor1-block')) initMotors();
}

// Charger la lib puis lancer la simulation
loadGaugeLibs(() => {
  initMotors();
  setInterval(fetchMotorData, 500);
});
