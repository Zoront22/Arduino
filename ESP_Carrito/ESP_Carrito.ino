#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// config pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_LEFT_ADD 0x3C
#define OLED_RIGHT_ADD 0x3D

Adafruit_SSD1306 display_left(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SSD1306 display_right(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Config Servo
const int inServo = 14;
// const int inServo2 = 17;
// const int inServo3 = 16;

const int reposo = 85;
const int preDisparo = 90;
const int disparo = 20;

Servo patada;

// pines
const int in1 = 26;
const int in2 = 25;
const int in3 = 33;
const int in4 = 32;

// Acceso Wi-Fi
const char *ssid = "CARRITO:PP";
const char *password = "12345678";

// Servidor web
WebServer server(80);

// Setup function
void setup()
{
  Serial.begin(115200);

  // iniciar motores
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  patada.attach(inServo); // Único pin permitido para el servo

  patada.write(reposo);
  delay(500);
  Serial.println("Todo bien :)");

  // Iniciar Wi-Fi
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // rutas
  server.on("/", handleRoot);
  server.on("/F", forward);
  server.on("/B", backward);
  server.on("/L", left);
  server.on("/R", right);
  server.on("/S", stopCar);
  server.on("/K", kick);

  server.begin();
  Serial.println("Todo bien :)))");

  // Iniciar comunicación I2C
  Wire.begin(13, 12); // SDA=GPIO21, SCL=GPIO22

  // Inicializar pantalla izquierda
  if (!display_left.begin(SSD1306_SWITCHCAPVCC, OLED_LEFT_ADD))
  {
    Serial.println("No se encontró OLED izquierda (0x3C)");
    while (1)
      ;
  }

  // Inicializar pantalla derecha
  if (!display_right.begin(SSD1306_SWITCHCAPVCC, OLED_RIGHT_ADD))
  {
    Serial.println("No se encontró OLED derecha (0x3D)");
    while (1)
      ;
  }

  // Limpiar ambas pantallas
  display_left.clearDisplay();
  display_right.clearDisplay();

  // Dibujar los ojos en sus respectivas pantallas
  drawEyeLeft();  // función que dibuja el ojo izquierdo en display_left
  drawEyeRight(); // función que dibuja el ojo derecho en display_right

  // Mostrar en pantalla
  display_left.display();
  display_right.display();
}

// Main
void loop()
{
  server.handleClient();

  // parpadeo de ojos
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 3000)
  { // parpadea cada 3 segundos
    lastBlink = millis();
    blinkBoth();
  }
}

// HTML
void handleRoot()
{
  String html = R"rawliteral(
    <!DOCTYPE html>
  <html lang="es">

  <head>
    <meta charset="UTF-8">

    <title>ESP32 Car Control</title>

    <meta name="viewport"
          content="width=device-width,
                   initial-scale=1.0,
                   maximum-scale=1.0,
                   user-scalable=no">

    <style>

      * {
        box-sizing: border-box;
        user-select: none;
        -webkit-user-select: none;
        -webkit-tap-highlight-color: transparent;
      }

      html,
      body {
        width: 100%;
        height: 100%;
        margin: 0;
      }

      body {
        background: #111;
        color: white;
        font-family: Arial, sans-serif;

        display: flex;
        flex-direction: column;
        align-items: center;

        overflow: hidden;

        padding:
          clamp(10px, 3vh, 30px)
          15px;
      }

      h2 {
        margin:
          0 0
          clamp(10px, 3vh, 25px);

        font-size:
          clamp(20px, 6vw, 30px);
      }


      /* =========================
         JOYSTICK
         ========================= */

      #joystick {

        /*
          El tamaño depende del ancho y alto
          de la pantalla.

          Mínimo: 160px
          Preferido: 55vw
          Máximo: 280px
        */

        width: clamp(160px, 55vw, 280px);
        aspect-ratio: 1 / 1;

        position: relative;

        background: #222;

        border:
          clamp(2px, 0.7vw, 4px)
          solid #555;

        border-radius: 50%;

        box-shadow:
          inset 0 0 20px #000,
          0 0 15px rgba(255,255,255,0.1);

        touch-action: none;

        flex-shrink: 1;
      }


      /* Flecha arriba */

      #joystick::before {

        content: "▲";

        position: absolute;

        top: 4%;

        left: 50%;

        transform: translateX(-50%);

        color: #555;

        font-size:
          clamp(14px, 4vw, 22px);

      }


      /* Flecha abajo */

      #joystick::after {

        content: "▼";

        position: absolute;

        bottom: 4%;

        left: 50%;

        transform: translateX(-50%);

        color: #555;

        font-size:
          clamp(14px, 4vw, 22px);

      }


      /* =========================
         PALANCA
         ========================= */

      #stick {

        position: absolute;

        width:
          41%;

        aspect-ratio: 1 / 1;

        left: 50%;
        top: 50%;

        transform:
          translate(-50%, -50%);

        background: #444;

        border:
          clamp(2px, 0.7vw, 4px)
          solid #777;

        border-radius: 50%;

        box-shadow:
          0 5px 10px rgba(0,0,0,0.6),
          inset 0 0 10px rgba(255,255,255,0.1);

        transition:
          transform 0.06s;

        pointer-events: none;
      }


      /* =========================
         BOTÓN PATEAR
         ========================= */

      #kick {

        width:
          clamp(150px, 55vw, 220px);

        height:
          clamp(50px, 10vh, 70px);

        margin-top:
          clamp(15px, 4vh, 30px);

        border: none;

        border-radius: 15px;

        background: #e53935;

        color: white;

        font-size:
          clamp(16px, 5vw, 22px);

        font-weight: bold;

        box-shadow:
          0 5px 0 #8e1b18;

        touch-action: manipulation;

        flex-shrink: 0;
      }


      #kick:active {

        transform:
          translateY(5px);

        box-shadow:
          0 0 0 #8e1b18;

        background: #ff5555;
      }


      /* =========================
         ESTADO
         ========================= */

      #status {

        margin-top:
          clamp(10px, 2vh, 20px);

        font-size:
          clamp(14px, 4vw, 18px);

        color: #aaa;

        min-height: 22px;
      }


      /* =========================
         CELULARES MUY PEQUEÑOS
         ========================= */

      @media (max-height: 600px) {

        body {
          padding-top: 8px;
        }

        h2 {
          margin-bottom: 8px;
        }

        #joystick {
          width: clamp(140px, 45vh, 220px);
        }

        #kick {
          margin-top: 10px;
          height: 45px;
        }

        #status {
          margin-top: 8px;
        }
      }


      /* =========================
         PANTALLA HORIZONTAL
         ========================= */

      @media (orientation: landscape) {

        body {

          display: grid;

          grid-template-columns:
            1fr 1fr;

          grid-template-rows:
            auto 1fr auto;

          justify-items: center;

          align-items: center;
        }

        h2 {

          grid-column: 1 / 3;

          margin: 0;
        }

        #joystick {

          grid-column: 1;

          grid-row: 2;

          width:
            clamp(150px, 45vh, 280px);
        }

        #kick {

          grid-column: 2;

          grid-row: 2;

          margin: 0;
        }

        #status {

          grid-column: 1 / 3;

          grid-row: 3;

        }
      }

    </style>
  </head>


  <body>

    <h2>ESP32 Control 🚗</h2>


    <!-- JOYSTICK -->

    <div id="joystick">

      <div id="stick"></div>

    </div>


    <!-- BOTÓN -->

    <button id="kick">
      ⚽ PATEAR
    </button>


    <!-- ESTADO -->

    <div id="status">
      Detenido
    </div>


    <script>

function sendCommand(cmd) {
  fetch("/" + cmd).catch(() => {});
}

const joystick = document.getElementById("joystick");
const stick = document.getElementById("stick");
const status = document.getElementById("status");

let active = false;
let lastCommand = "S";        // comando enviado anteriormente

function getJoystickDimensions() {
  const rect = joystick.getBoundingClientRect();
  const radius = rect.width / 2;
  const stickRadius = stick.getBoundingClientRect().width / 2;
  return {
    radius: radius,
    maxDistance: radius - stickRadius
  };
}

function moveJoystick(clientX, clientY) {
  const rect = joystick.getBoundingClientRect();
  const centerX = rect.left + rect.width / 2;
  const centerY = rect.top + rect.height / 2;

  let x = clientX - centerX;
  let y = clientY - centerY;

  const { maxDistance } = getJoystickDimensions();
  const distance = Math.sqrt(x * x + y * y);

  // Limitar al radio máximo
  if (distance > maxDistance) {
    x = (x / distance) * maxDistance;
    y = (y / distance) * maxDistance;
  }

  // Mover visualmente la palanca
  stick.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;

  const threshold = rect.width * 0.12;

  let command = "S";
  let text = "Detenido";

  if (Math.abs(x) < threshold && Math.abs(y) < threshold) {
    command = "S";
    text = "Detenido";
  } else if (y < -threshold && Math.abs(x) < threshold) {
    command = "F";
    text = "⬆ Adelante";
  } else if (y > threshold && Math.abs(x) < threshold) {
    command = "B";
    text = "⬇ Atrás";
  } else if (x < -threshold && Math.abs(y) < threshold) {
    command = "L";
    text = "⬅ Izquierda";
  } else if (x > threshold && Math.abs(y) < threshold) {
    command = "R";
    text = "➡ Derecha";
  } else if (y < -threshold && x < -threshold) {
    command = "F";   // diagonal adelante-izquierda
    text = "↖ Adelante + Izquierda";
  } else if (y < -threshold && x > threshold) {
    command = "F";   // diagonal adelante-derecha
    text = "↗ Adelante + Derecha";
  } else if (y > threshold && x < -threshold) {
    command = "B";   // diagonal atrás-izquierda
    text = "↙ Atrás + Izquierda";
  } else if (y > threshold && x > threshold) {
    command = "B";   // diagonal atrás-derecha
    text = "↘ Atrás + Derecha";
  }

  status.textContent = text;

  // Solo enviar si el comando cambió
  if (command !== lastCommand) {
    lastCommand = command;
    sendCommand(command);
  }
}

function resetJoystick() {
  active = false;
  stick.style.transform = "translate(-50%, -50%)";
  status.textContent = "Detenido";
  if (lastCommand !== "S") {
    lastCommand = "S";
    sendCommand("S");
  }
}

/* Eventos de ratón */
joystick.addEventListener("mousedown", (e) => {
  active = true;
  moveJoystick(e.clientX, e.clientY);
});

document.addEventListener("mousemove", (e) => {
  if (active) {
    moveJoystick(e.clientX, e.clientY);
  }
});

document.addEventListener("mouseup", () => {
  if (active) {
    resetJoystick();
  }
});

/* Eventos táctiles */
joystick.addEventListener("touchstart", (e) => {
  e.preventDefault();
  active = true;
  const touch = e.touches[0];
  moveJoystick(touch.clientX, touch.clientY);
}, { passive: false });

// Ahora touchmove se registra en document, no solo en el joystick
document.addEventListener("touchmove", (e) => {
  if (active) {
    e.preventDefault();
    const touch = e.touches[0];
    moveJoystick(touch.clientX, touch.clientY);
  }
}, { passive: false });

document.addEventListener("touchend", (e) => {
  if (active) {
    e.preventDefault();
    resetJoystick();
  }
}, { passive: false });

/* Botón de patada */
document.getElementById("kick").addEventListener("click", () => {
  sendCommand("K");
});
    </script>

  </body>

  </html>

  )rawliteral";

  server.send(200, "text/html", html);
}

// Movement control functions
void forward()
{
  Serial.println("Adelante");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  server.send(200, "text/plain", "Forward");
}

void backward()
{
  Serial.println("Atras");
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  server.send(200, "text/plain", "Backward");
}

void left()
{
  Serial.println("izq");
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  server.send(200, "text/plain", "Left");
}

void right()
{
  Serial.println("Der");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  server.send(200, "text/plain", "Right");
}

void stopCar()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  server.send(200, "text/plain", "Stop");
}

// Patada
void kick()
{
  // Responder primero para no bloquear el navegador
  server.send(200, "text/plain", "Kick");
  Serial.println("Pateando...");

  // Secuencia de patada
  patada.write(preDisparo);
  delay(500);

  patada.write(disparo);
  delay(200);

  patada.write(reposo);
  delay(300);
}

// ojos
void drawEyeLeft()
{
  display_left.clearDisplay();
  display_left.fillRoundRect(20, 12, 88, 40, 12, WHITE);
  display_left.fillCircle(64, 32, 8, BLACK);
  display_left.display();
}

void drawEyeRight()
{
  display_right.clearDisplay();
  display_right.fillRoundRect(20, 12, 88, 40, 12, WHITE);
  display_right.fillCircle(64, 32, 8, BLACK);
  display_right.display();
}

void blinkBoth()
{
  display_left.clearDisplay();
  display_left.drawLine(20, 32, 108, 32, WHITE);
  display_left.display();

  display_right.clearDisplay();
  display_right.drawLine(20, 32, 108, 32, WHITE);
  display_right.display();
  delay(120);

  drawEyeLeft();
  drawEyeRight();
}