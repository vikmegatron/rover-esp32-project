//codigo ya editado y funcionando


#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

// Configuración WiFi
const char* ssid = "Totalplay-E1B4";
const char* password = "E1B415E8k2HUT5Dh";

// PINES CORREGIDOS para tus L298N
const int ENA_LEFT = 12;    // PWM Izquierdo
const int IN1_LEFT = 13;    // Izquierdo Adelante  
const int IN2_LEFT = 14;    // Izquierdo Atrás

const int ENA_RIGHT = 15;   // PWM Derecho
const int IN1_RIGHT = 2;    // Derecho Adelante
const int IN2_RIGHT = 4;    // Derecho Atrás

#define LED_PIN 25  // LED onboard

WebServer server(80);
int ledBrightness = 128;
bool ledState = false;

// Función para controlar motores
void controlMotores(int leftSpeed, int rightSpeed) {
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);
    
    // Motor IZQUIERDO
    if(leftSpeed > 0) {
        analogWrite(ENA_LEFT, leftSpeed);
        digitalWrite(IN1_LEFT, HIGH);
        digitalWrite(IN2_LEFT, LOW);
        Serial.printf("Motor IZQ: ADELANTE velocidad=%d\n", leftSpeed);
    } else if(leftSpeed < 0) {
        analogWrite(ENA_LEFT, abs(leftSpeed));
        digitalWrite(IN1_LEFT, LOW);
        digitalWrite(IN2_LEFT, HIGH);
        Serial.printf("Motor IZQ: ATRÁS velocidad=%d\n", abs(leftSpeed));
    } else {
        analogWrite(ENA_LEFT, 0);
        digitalWrite(IN1_LEFT, LOW);
        digitalWrite(IN2_LEFT, LOW);
        Serial.println("Motor IZQ: PARADO");
    }
    
    // Motor DERECHO
    if(rightSpeed > 0) {
        analogWrite(ENA_RIGHT, rightSpeed);
        digitalWrite(IN1_RIGHT, HIGH);
        digitalWrite(IN2_RIGHT, LOW);
        Serial.printf("Motor DER: ADELANTE velocidad=%d\n", rightSpeed);
    } else if(rightSpeed < 0) {
        analogWrite(ENA_RIGHT, abs(rightSpeed));
        digitalWrite(IN1_RIGHT, LOW);
        digitalWrite(IN2_RIGHT, HIGH);
        Serial.printf("Motor DER: ATRÁS velocidad=%d\n", abs(rightSpeed));
    } else {
        analogWrite(ENA_RIGHT, 0);
        digitalWrite(IN1_RIGHT, LOW);
        digitalWrite(IN2_RIGHT, LOW);
        Serial.println("Motor DER: PARADO");
    }
}

// Página web HTML COMPLETO
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Rover Control ESP32</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            margin: 0;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 30px;
            border: 1px solid rgba(255, 255, 255, 0.2);
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
        }
        h1 {
            margin-bottom: 30px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .control-section {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 15px;
            padding: 20px;
            margin: 20px 0;
        }
        .joystick-container {
            width: 300px;
            height: 300px;
            margin: 20px auto;
            background: rgba(0, 0, 0, 0.2);
            border-radius: 50%;
            position: relative;
            border: 3px solid rgba(255, 255, 255, 0.3);
        }
        .joystick {
            width: 60px;
            height: 60px;
            background: linear-gradient(135deg, #ff8a00, #ffcc00);
            border-radius: 50%;
            position: absolute;
            top: 120px;
            left: 120px;
            cursor: pointer;
            box-shadow: 0 4px 15px rgba(255, 140, 0, 0.5);
            border: 3px solid rgba(255, 255, 255, 0.5);
            touch-action: none;
            user-select: none;
        }
        .button {
            padding: 15px 30px;
            margin: 10px;
            font-size: 18px;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            transition: all 0.3s;
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
            font-weight: bold;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
        }
        .button:hover {
            transform: translateY(-3px);
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);
        }
        .button:active {
            transform: translateY(1px);
        }
        .status {
            margin: 20px;
            padding: 15px;
            background: rgba(0, 255, 0, 0.1);
            border-radius: 10px;
            border-left: 5px solid #00ff00;
        }
        .key-control {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            max-width: 300px;
            margin: 20px auto;
        }
        .key {
            padding: 20px;
            background: rgba(255, 255, 255, 0.15);
            border-radius: 10px;
            cursor: pointer;
            user-select: none;
            transition: all 0.2s;
            border: 2px solid transparent;
            font-size: 20px;
            font-weight: bold;
        }
        .key:hover {
            background: rgba(255, 255, 255, 0.25);
        }
        .key:active, .key.active {
            background: rgba(0, 255, 0, 0.3);
            border-color: #00ff00;
            transform: scale(0.95);
        }
        .w { grid-column: 2; grid-row: 1; }
        .a { grid-column: 1; grid-row: 2; }
        .s { grid-column: 2; grid-row: 2; }
        .d { grid-column: 3; grid-row: 2; }
        
        /* Estilos para LED y slider */
        .led-container {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 20px;
            margin: 20px 0;
        }
        .led-indicator {
            width: 60px;
            height: 60px;
            border-radius: 50%;
            background: #444;
            border: 3px solid rgba(255, 255, 255, 0.3);
            transition: all 0.3s;
            box-shadow: 0 0 10px rgba(0,0,0,0.3);
        }
        .led-indicator.on {
            background: #ffcc00;
            box-shadow: 0 0 20px #ffcc00, 0 0 40px rgba(255, 204, 0, 0.5);
        }
        .slider-container {
            width: 80%;
            margin: 20px auto;
            text-align: center;
        }
        .slider {
            width: 100%;
            height: 25px;
            -webkit-appearance: none;
            appearance: none;
            background: rgba(255, 255, 255, 0.2);
            outline: none;
            border-radius: 15px;
            margin: 15px 0;
        }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: #4facfe;
            cursor: pointer;
            border: 3px solid white;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);
        }
        .slider::-moz-range-thumb {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: #4facfe;
            cursor: pointer;
            border: 3px solid white;
        }
        .brightness-value {
            font-size: 24px;
            font-weight: bold;
            margin: 10px;
            color: #ffcc00;
            text-shadow: 0 0 10px rgba(255, 204, 0, 0.5);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 Rover Control ESP32</h1>
        
        <div class="status" id="status">
            Conectando... IP: <span id="ip">-</span>
        </div>
        
        <div class="control-section">
            <h2>🎮 Joystick Virtual</h2>
            <div class="joystick-container" id="joystickContainer">
                <div class="joystick" id="joystick"></div>
            </div>
            <p id="joystickPos">Posición: x=0, y=0 | Velocidad: L=0, R=0</p>
        </div>
        
        <div class="control-section">
            <h2>💡 Control LED</h2>
            <div class="led-container">
                <div class="led-indicator" id="ledIndicator"></div>
                <button class="button" onclick="toggleLED()">ON/OFF</button>
            </div>
            
            <div class="slider-container">
                <h3>Brillo LED</h3>
                <input type="range" min="0" max="255" value="128" class="slider" id="brightnessSlider">
                <div class="brightness-value" id="brightnessValue">128</div>
            </div>
        </div>
        
        <div class="control-section">
            <h2>🎯 Teclas WASD</h2>
            <div class="key-control">
                <div class="key w" id="keyW" ontouchstart="keyDown('w')" ontouchend="keyUp('w')" 
                     onmousedown="keyDown('w')" onmouseup="keyUp('w')" onmouseleave="keyUp('w')">W</div>
                <div class="key a" id="keyA" ontouchstart="keyDown('a')" ontouchend="keyUp('a')"
                     onmousedown="keyDown('a')" onmouseup="keyUp('a')" onmouseleave="keyUp('a')">A</div>
                <div class="key s" id="keyS" ontouchstart="keyDown('s')" ontouchend="keyUp('s')"
                     onmousedown="keyDown('s')" onmouseup="keyUp('s')" onmouseleave="keyUp('s')">S</div>
                <div class="key d" id="keyD" ontouchstart="keyDown('d')" ontouchend="keyUp('d')"
                     onmousedown="keyDown('d')" onmouseup="keyUp('d')" onmouseleave="keyUp('d')">D</div>
            </div>
            <p>Presiona las teclas o haz clic/toca (usa teclas W A S D en tu teclado)</p>
        </div>
        
        <div class="control-section">
            <h2>⚙️ Comandos Manuales</h2>
            <button class="button" onclick="sendCommand('forward')">Adelante</button>
            <button class="button" onclick="sendCommand('backward')">Atrás</button>
            <button class="button" onclick="sendCommand('left')">Izquierda</button>
            <button class="button" onclick="sendCommand('right')">Derecha</button>
            <button class="button" onclick="sendCommand('stop')">Detener</button>
        </div>
    </div>
    
    <script>
        // Elementos DOM
        const joystick = document.getElementById('joystick');
        const container = document.getElementById('joystickContainer');
        const joystickPos = document.getElementById('joystickPos');
        const ledIndicator = document.getElementById('ledIndicator');
        const brightnessSlider = document.getElementById('brightnessSlider');
        const brightnessValue = document.getElementById('brightnessValue');
        
        // Variables
        let isDragging = false;
        let currentBrightness = 128;
        
        // ========== JOYSTICK ==========
        function startDrag(e) {
            e.preventDefault();
            isDragging = true;
            updateJoystick(e);
        }
        
        function stopDrag() {
            if (!isDragging) return;
            isDragging = false;
            
            // Resetear joystick a centro
            joystick.style.transition = 'all 0.3s ease';
            joystick.style.top = '120px';
            joystick.style.left = '120px';
            
            setTimeout(() => {
                joystick.style.transition = '';
            }, 300);
            
            joystickPos.textContent = 'Posición: x=0, y=0 | Velocidad: L=0, R=0';
            sendJoystick(0, 0);
        }
        
        function updateJoystick(e) {
            if (!isDragging) return;
            
            const rect = container.getBoundingClientRect();
            let clientX, clientY;
            
            if (e.type.includes('touch')) {
                clientX = e.touches[0].clientX;
                clientY = e.touches[0].clientY;
            } else {
                clientX = e.clientX;
                clientY = e.clientY;
            }
            
            // Calcular posición relativa
            let x = clientX - rect.left;
            let y = clientY - rect.top;
            
            // Limitar al círculo (radio 120px)
            const centerX = 150;
            const centerY = 150;
            const radius = 120;
            
            let dx = x - centerX;
            let dy = y - centerY;
            let distance = Math.sqrt(dx * dx + dy * dy);
            
            if (distance > radius) {
                dx = (dx / distance) * radius;
                dy = (dy / distance) * radius;
                x = centerX + dx;
                y = centerY + dy;
            }
            
            // Mover joystick visualmente
            joystick.style.left = (x - 30) + 'px';
            joystick.style.top = (y - 30) + 'px';
            
            // Calcular valores normalizados (-100 a 100)
            const normX = Math.round((dx / radius) * 100);
            const normY = Math.round((-dy / radius) * 100); // Invertir Y para intuición
            
            // Calcular velocidades de motores
            const leftSpeed = Math.round(normY + normX);
            const rightSpeed = Math.round(normY - normX);
            
            // Actualizar texto
            joystickPos.textContent = `Posición: x=${normX}, y=${normY} | Velocidad: L=${leftSpeed}, R=${rightSpeed}`;
            
            // Enviar al ESP32
            sendJoystick(normX, normY);
        }
        
        // ========== LED CONTROL ==========
        function toggleLED() {
            fetch('/led')
                .then(response => response.text())
                .then(state => {
                    if (state === 'ON') {
                        ledIndicator.classList.add('on');
                        brightnessSlider.disabled = false;
                    } else {
                        ledIndicator.classList.remove('on');
                        brightnessSlider.disabled = true;
                    }
                });
        }
        
        // Inicializar slider de brillo
        brightnessSlider.addEventListener('input', function() {
            const value = this.value;
            brightnessValue.textContent = value;
            currentBrightness = value;
            
            // Enviar brillo solo si LED está encendido
            if (ledIndicator.classList.contains('on')) {
                fetch('/brightness?value=' + value);
            }
        });
        
        // ========== TECLAS WASD ==========
        function keyDown(key) {
            const keyElement = document.getElementById('key' + key.toUpperCase());
            keyElement.classList.add('active');
            fetch('/key?key=' + key + '&action=down');
        }
        
        function keyUp(key) {
            const keyElement = document.getElementById('key' + key.toUpperCase());
            keyElement.classList.remove('active');
            fetch('/key?key=' + key + '&action=up');
        }
        
        // ========== COMANDOS ==========
        function sendCommand(cmd) {
            fetch('/command?cmd=' + cmd);
        }
        
        function sendJoystick(x, y) {
            fetch('/joystick?x=' + x + '&y=' + y);
        }
        
        // ========== EVENT LISTENERS ==========
        // Joystick eventos mouse
        joystick.addEventListener('mousedown', startDrag);
        container.addEventListener('mousemove', updateJoystick);
        document.addEventListener('mouseup', stopDrag);
        
        // Joystick eventos touch
        joystick.addEventListener('touchstart', startDrag, { passive: false });
        container.addEventListener('touchmove', updateJoystick, { passive: false });
        container.addEventListener('touchend', stopDrag);
        
        // Teclado físico
        document.addEventListener('keydown', function(e) {
            const key = e.key.toLowerCase();
            if (['w', 'a', 's', 'd'].includes(key)) {
                e.preventDefault();
                keyDown(key);
            }
        });
        
        document.addEventListener('keyup', function(e) {
            const key = e.key.toLowerCase();
            if (['w', 'a', 's', 'd'].includes(key)) {
                keyUp(key);
            }
        });
        
        // Prevenir scroll con espacio/teclas
        document.addEventListener('keydown', function(e) {
            if ([' ', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key)) {
                e.preventDefault();
            }
        });
        
        // ========== INICIALIZACIÓN ==========
        window.onload = function() {
            fetch('/ip')
                .then(response => response.text())
                .then(ip => {
                    document.getElementById('ip').textContent = ip;
                    document.getElementById('status').innerHTML = 
                        '✅ Conectado - IP: ' + ip + ' | 🎮 Control listo!';
                });
            
            // Inicializar estado LED
            fetch('/ledstatus')
                .then(response => response.json())
                .then(data => {
                    if (data.led === 'on') {
                        ledIndicator.classList.add('on');
                        brightnessSlider.value = data.brightness;
                        brightnessValue.textContent = data.brightness;
                    }
                })
                .catch(() => {
                    // Si falla, usar valores por defecto
                    ledIndicator.classList.remove('on');
                    brightnessSlider.value = 128;
                    brightnessValue.textContent = '128';
                });
        };
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n🚀 Rover ESP32 - CONTROL COMPLETO");
    Serial.println("=================================");
    
    // Configurar pines MOTORES
    pinMode(ENA_LEFT, OUTPUT);
    pinMode(IN1_LEFT, OUTPUT);
    pinMode(IN2_LEFT, OUTPUT);
    
    pinMode(ENA_RIGHT, OUTPUT);
    pinMode(IN1_RIGHT, OUTPUT);
    pinMode(IN2_RIGHT, OUTPUT);
    
    pinMode(LED_PIN, OUTPUT);
    analogWrite(LED_PIN, ledBrightness);
    
    // Inicializar motores APAGADOS
    digitalWrite(IN1_LEFT, LOW);
    digitalWrite(IN2_LEFT, LOW);
    analogWrite(ENA_LEFT, 0);
    
    digitalWrite(IN1_RIGHT, LOW);
    digitalWrite(IN2_RIGHT, LOW);
    analogWrite(ENA_RIGHT, 0);
    
    // WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi: ");
    Serial.println(ssid);
    
    int intentos = 0;
    while(WiFi.status() != WL_CONNECTED && intentos < 30) {
        delay(500);
        Serial.print(".");
        intentos++;
    }
    
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi CONECTADO");
        Serial.print("📶 IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("📡 RSSI: ");
        Serial.println(WiFi.RSSI());
    } else {
        Serial.println("\n📡 Modo ACCESS POINT");
        WiFi.softAP("Rover-ESP32", "password123");
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    }
    
    // ========== RUTAS DEL SERVIDOR ==========
    
    // Página principal
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", index_html);
        Serial.println("📄 Página web servida");
    });
    
    // Obtener IP
    server.on("/ip", HTTP_GET, []() {
        String ip = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        server.send(200, "text/plain", ip);
    });
    
    // Estado LED
    server.on("/ledstatus", HTTP_GET, []() {
        String json = "{";
        json += "\"led\":\"" + String(ledState ? "on" : "off") + "\",";
        json += "\"brightness\":\"" + String(ledBrightness) + "\"";
        json += "}";
        server.send(200, "application/json", json);
    });
    
    // Joystick
    server.on("/joystick", HTTP_GET, []() {
        if(server.hasArg("x") && server.hasArg("y")) {
            int x = server.arg("x").toInt();
            int y = server.arg("y").toInt();
            
            // Convertir joystick (-100 a 100) a velocidad de motores (-255 a 255)
            int leftSpeed = map(y + x, -200, 200, -255, 255);
            int rightSpeed = map(y - x, -200, 200, -255, 255);
            
            // Limitar valores
            leftSpeed = constrain(leftSpeed, -255, 255);
            rightSpeed = constrain(rightSpeed, -255, 255);
            
            controlMotores(leftSpeed, rightSpeed);
            
            Serial.printf("🎮 Joystick: x=%d, y=%d → L=%d, R=%d\n", x, y, leftSpeed, rightSpeed);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Faltan parámetros");
        }
    });
    
    // Control LED
    server.on("/led", HTTP_GET, []() {
        ledState = !ledState;
        analogWrite(LED_PIN, ledState ? ledBrightness : 0);
        
        Serial.printf("💡 LED: %s (brillo: %d)\n", ledState ? "ON" : "OFF", ledBrightness);
        server.send(200, "text/plain", ledState ? "ON" : "OFF");
    });
    
    // Brillo LED
    server.on("/brightness", HTTP_GET, []() {
        if(server.hasArg("value")) {
            ledBrightness = server.arg("value").toInt();
            ledBrightness = constrain(ledBrightness, 0, 255);
            
            if(ledState) {
                analogWrite(LED_PIN, ledBrightness);
            }
            
            Serial.printf("💡 Brillo LED ajustado: %d\n", ledBrightness);
            server.send(200, "text/plain", String(ledBrightness));
        } else {
            server.send(400, "text/plain", "Falta valor de brillo");
        }
    });
    
    // Teclas WASD
    server.on("/key", HTTP_GET, []() {
        if(server.hasArg("key") && server.hasArg("action")) {
            String key = server.arg("key");
            String action = server.arg("action");
            
            int leftSpeed = 0, rightSpeed = 0;
            
            if(action == "down") {
                if(key == "w") { 
                    leftSpeed = 200; 
                    rightSpeed = 200; 
                } else if(key == "s") { 
                    leftSpeed = -200; 
                    rightSpeed = -200; 
                } else if(key == "a") { 
                    leftSpeed = -200; 
                    rightSpeed = 200; 
                } else if(key == "d") { 
                    leftSpeed = 200; 
                    rightSpeed = -200; 
                }
            }
            
            controlMotores(leftSpeed, rightSpeed);
            Serial.printf("⌨️ Tecla: %s %s → L=%d, R=%d\n", key.c_str(), action.c_str(), leftSpeed, rightSpeed);
            
            server.send(200, "text/plain", "OK");
        }
    });
    
    // Comandos manuales
    server.on("/command", HTTP_GET, []() {
        if(server.hasArg("cmd")) {
            String cmd = server.arg("cmd");
            
            if(cmd == "forward") {
                controlMotores(200, 200);
                Serial.println("🔼 ADELANTE");
            } else if(cmd == "backward") {
                controlMotores(-200, -200);
                Serial.println("🔽 ATRÁS");
            } else if(cmd == "left") {
                controlMotores(-200, 200);
                Serial.println("◀️ IZQUIERDA");
            } else if(cmd == "right") {
                controlMotores(200, -200);
                Serial.println("▶️ DERECHA");
            } else if(cmd == "stop") {
                controlMotores(0, 0);
                Serial.println("⏹️ DETENER");
            }
            
            server.send(200, "text/plain", "OK");
        }
    });
    
    // Prueba de motores
    server.on("/test", HTTP_GET, []() {
        Serial.println("🔧 INICIANDO PRUEBA DE MOTORES...");
        
        // Prueba motor izquierdo
        Serial.println("Motor IZQUIERDO adelante (50%)");
        analogWrite(ENA_LEFT, 128);
        digitalWrite(IN1_LEFT, HIGH);
        digitalWrite(IN2_LEFT, LOW);
        delay(1000);
        
        Serial.println("Motor IZQUIERDO atrás (50%)");
        digitalWrite(IN1_LEFT, LOW);
        digitalWrite(IN2_LEFT, HIGH);
        delay(1000);
        
        Serial.println("Motor IZQUIERDO parar");
        digitalWrite(IN1_LEFT, LOW);
        digitalWrite(IN2_LEFT, LOW);
        analogWrite(ENA_LEFT, 0);
        delay(500);
        
        // Prueba motor derecho
        Serial.println("Motor DERECHO adelante (50%)");
        analogWrite(ENA_RIGHT, 128);
        digitalWrite(IN1_RIGHT, HIGH);
        digitalWrite(IN2_RIGHT, LOW);
        delay(1000);
        
        Serial.println("Motor DERECHO atrás (50%)");
        digitalWrite(IN1_RIGHT, LOW);
        digitalWrite(IN2_RIGHT, HIGH);
        delay(1000);
        
        Serial.println("Motor DERECHO parar");
        digitalWrite(IN1_RIGHT, LOW);
        digitalWrite(IN2_RIGHT, LOW);
        analogWrite(ENA_RIGHT, 0);
        
        Serial.println("✅ PRUEBA COMPLETADA");
        server.send(200, "text/plain", "Prueba completada exitosamente");
    });
    
    // Iniciar servidor
    server.begin();
    Serial.println("✅ Servidor HTTP INICIADO");
    
    // Mostrar información
    Serial.println("\n📱 ACCESO AL CONTROL:");
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("   🌐 http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.print("   📡 http://");
        Serial.println(WiFi.softAPIP());
        Serial.println("   🔑 Contraseña AP: password123");
    }
    
    Serial.println("\n🎯 FUNCIONES DISPONIBLES:");
    Serial.println("   🎮 Joystick virtual con arrastre");
    Serial.println("   💡 Control LED con ajuste de brillo");
    Serial.println("   ⌨️ Teclas WASD (físicas y virtuales)");
    Serial.println("   ⚙️ Botones de comandos predefinidos");
    Serial.println("   🔧 /test - Prueba manual de motores");
    Serial.println("\n🚀 SISTEMA LISTO!");
}

void loop() {
    server.handleClient();
    delay(10);
}