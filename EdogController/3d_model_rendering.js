import * as THREE from 'three';
import { STLLoader } from 'three/examples/jsm/loaders/STLLoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';

// Sélectionne le conteneur HTML
const container = document.getElementById('three-container');

// Création de la scène et de la caméra
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(
  75,
  container.clientWidth / container.clientHeight,
  0.1,
  1000
);
camera.position.z = -50;

// Rendu
const renderer = new THREE.WebGLRenderer({ antialias: true });
renderer.setSize(container.clientWidth, container.clientHeight);
renderer.setAnimationLoop(animate);
container.appendChild(renderer.domElement);

// Contrôles orbitaux
const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;
controls.dampingFactor = 0.05;
controls.enableZoom = true;

// Loader STL
const loader = new STLLoader();

// Groupes hiérarchiques
const hipGroup = new THREE.Group();
const kneeGroup = new THREE.Group();
const footGroup = new THREE.Group();

hipGroup.position.set(-10, 15.005, 13.818); // racine du squelette
hipGroup.rotation.y = Math.PI / 2;
kneeGroup.position.set(5.987, -14.892, 12.260); // relatif à la hanche
footGroup.position.set(3.713, 12.957, -12.844); // relatif au genou

scene.add(hipGroup);
hipGroup.add(kneeGroup);
kneeGroup.add(footGroup);

// Chargement des modèles STL
let stlMesh1, stlMesh2, stlMesh3;

loader.load('/hip_v1.stl', function (geometry) {
  stlMesh1 = new THREE.Mesh(geometry, new THREE.MeshNormalMaterial());
  stlMesh1.scale.set(0.1, 0.1, 0.1);
  stlMesh1.position.set(0, 0, 0);
  hipGroup.add(stlMesh1);
});

loader.load('/knee_v1.stl', function (geometry) {
  stlMesh2 = new THREE.Mesh(geometry, new THREE.MeshNormalMaterial({ color: 0xff0000 }));
  stlMesh2.scale.set(0.1, 0.1, 0.1);
  stlMesh2.position.set(-5.987, 14.892, -12.260);
  kneeGroup.add(stlMesh2);
});

loader.load('/foot_v1.stl', function (geometry) {
  stlMesh3 = new THREE.Mesh(geometry, new THREE.MeshNormalMaterial({ color: 0x00ff00 }));
  stlMesh3.scale.set(0.1, 0.1, 0.1);
  stlMesh3.position.set(-9.700, 1.935, 0.604);
  footGroup.add(stlMesh3);
});

// WebSocket : ne modifie que la rotation X
const host = window.location.hostname === "localhost" ? "localhost" : window.location.hostname;
const socket = new WebSocket(`ws://${host}:8765`);
//const socket = new WebSocket(`ws://${window.location.hostname}:8765`);

socket.addEventListener('message', function (event) {
  const msg = JSON.parse(event.data);

  if (msg.type === 'rotation' && msg.data) {
    const data = msg.data;

    if (data.rotationX1 !== undefined) hipGroup.rotation.x = data.rotationX1;
    if (data.rotationX2 !== undefined) kneeGroup.rotation.x = data.rotationX2;
    if (data.rotationX3 !== undefined) footGroup.rotation.x = data.rotationX3;
  }
});


// Animation
function animate() {
  controls.update();
  renderer.render(scene, camera);
}

// Gère le redimensionnement du conteneur
window.addEventListener('resize', () => {
  camera.aspect = container.clientWidth / container.clientHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(container.clientWidth, container.clientHeight);
});
