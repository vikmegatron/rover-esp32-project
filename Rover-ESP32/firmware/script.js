// ===== VARIABLES GLOBALES =====
let roverIP = '';
let isConnected = false;
let currentSpeed = 255;

// ===== CONEXIÓN =====
function connect() {
    roverIP = document.getElementById('ipInput').value.trim();
    
    if (!roverIP) {
        alert('Por favor ingresa la IP del ESP32');
        return;
    }
    
    // Validar formato IP básico
    if (!roverIP.match(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/)) {
        alert('Formato de IP inválido. Ejemplo: 192.168.1.100');
        return;
    }
    
    // Probar conexión con un comando de prueba
    testConnection();
}

function testConnection() {
    const status = document.getElementById('status');
    status.textContent = '⏳ Probando conexión...';
    status.className = 'status';
    
    // Intentar obtener la página principal del ESP32
    fetch(`http://${roverIP}/`)
        .then(response => {
            if (response.ok) {
                isConnected = true;
                status.textContent = '✅ Conectado';
                status.classList.add('connected');
                addLog(`Conectado al Rover en ${roverIP}`);
            } else {
                throw new Error('No response');
            }
        })
        .catch(error => {
            isConnected = false;
            status.textContent = '❌ Error de conexión';
            status.className = 'status';
            addLog(`Error conectando a ${roverIP}`);
            alert('No se pudo conectar al Rover. Verifica:\n1. IP correcta\n2. Mismo WiFi\n3. ESP32 encendido');
        });
}

// ===== ENVÍO DE COMANDOS =====
function sendCommand(cmd) {
    if (!isConnected) {
        alert('Primero conéctate al Rover');
        return;
    }
    
    addLog(`Enviando: ${getCommandName(cmd)}`);
    
    // Enviar comando al ESP32
    fetch(`http://${roverIP}/${cmd}?speed=${currentSpeed}`)
        .then(response => {
            if (response.ok) {
                addLog(`✓ Comando ${cmd} enviado`);
            } else {
                addLog(`✗ Error en comando ${cmd}`);
            }
        })
        .catch(error => {
            addLog(`✗ Error de conexión: ${error.message}`);
            isConnected = false;
            document.getElementById('status').textContent = '❌ Desconectado';
            document.getElementById('status').className = 'status';
        });
}

function sendCustomCommand() {
    const cmdInput = document.getElementById('customCommand');
    const cmd = cmdInput.value.trim().toUpperCase();
    
    if (!cmd) return;
    
    if (['F', 'B', 'L', 'R', 'S', 'A', 'X', 'H'].includes(cmd)) {
        sendCommand(cmd);
    } else {
        alert('Comando inválido. Usa: F, B, L, R, S, A, X, H');
    }
    
    cmdInput.value = '';
}

// ===== CONTROL DE VELOCIDAD =====
function updateSpeed() {
    const slider = document.getElementById('speedSlider');
    const valueDisplay = document.getElementById('speedValue');
    
    currentSpeed = parseInt(slider.value);
    const percentage = Math.round((currentSpeed / 255) * 100);
    
    valueDisplay.textContent = `${percentage}%`;
    addLog(`Velocidad ajustada: ${percentage}%`);
}

// ===== LOG DE COMANDOS =====
function addLog(message) {
    const log = document.getElementById('commandLog');
    const timestamp = new Date().toLocaleTimeString();
    const entry = document.createElement('div');
    
    entry.className = 'log-entry';
    entry.innerHTML = `<span style="color:#00d4ff">[${timestamp}]</span> ${message}`;
    
    log.appendChild(entry);
    log.scrollTop = log.scrollHeight; // Auto-scroll al final
}

function clearLog() {
    document.getElementById('commandLog').innerHTML = 
        '<div class="log-entry">▶️ Log limpiado</div>';
}

function getCommandName(cmd) {
    const commands = {
        'F': 'ADELANTE',
        'B': 'ATRÁS',
        'L': 'IZQUIERDA',
        'R': 'DERECHA',
        'S': 'DETENER',
        'A': 'MODO AUTOMÁTICO',
        'X': 'EMERGENCIA',
        'H': 'VOLVER A INICIO'
    };
    return commands[cmd] || `CMD:${cmd}`;
}

// ===== CONTROL CON TECLADO =====
document.addEventListener('keydown', (event) => {
    if (!isConnected) return;
    
    const key = event.key.toUpperCase();
    const keyMap = {
        'W': 'F', 'ARROWUP': 'F',
        'S': 'B', 'ARROWDOWN': 'B',
        'A': 'L', 'ARROWLEFT': 'L',
        'D': 'R', 'ARROWRIGHT': 'R',
        ' ': 'S', 'ESCAPE': 'S'
    };
    
    if (keyMap[key]) {
        event.preventDefault(); // Evitar scroll con flechas
        sendCommand(keyMap[key]);
        addLog(`Tecla: ${key} → ${getCommandName(keyMap[key])}`);
    }
});

// ===== INICIALIZACIÓN =====
window.onload = function() {
    addLog('Interfaz de control cargada');
    addLog('1. Conecta el ESP32 a la misma red WiFi');
    addLog('2. Ingresa la IP que muestra el Monitor Serie');
    addLog('3. Haz clic en "Conectar"');
    
    // Valor por defecto en el campo IP (para pruebas)
    document.getElementById('ipInput').value = '192.168.1.100';
};