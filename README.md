# Robot Soccer - Control por WiFi

Un proyecto de carrito robótico controlado por WiFi usando un **ESP32**, con capacidad de movimiento en 4 direcciones, mecanismo de patada servomotor y pantallas OLED con animaciones de ojos. Controlado mediante una interfaz web.

---

## 📋 Tabla de Contenidos

1. [Características](#características)
2. [Hardware Requerido](#hardware-requerido)
3. [Estructura del Código](#estructura-del-código)
4. [Explicación Detallada del Código](#explicación-detallada-del-código)
5. [Métodos y Funciones](#métodos-y-funciones)
6. [Configuración](#configuración)
7. [Instalación y Carga en ESP32](#instalación-y-carga-en-esp32)
8. [Uso de la Aplicación](#uso-de-la-aplicación)
9. [Librerías Utilizadas](#librerías-utilizadas)

---

## ✨ Características

- ✅ **Control WiFi en tiempo real** - Conecta vía Access Point (AP)
- ✅ **Movimiento 4 direcciones** - Adelante, Atrás, Izquierda, Derecha
- ✅ **Mecanismo de patada** - Servo motor 
- ✅ **Pantallas OLED animadas** - Dos ojos con parpadeo automático (I2C)
- ✅ **Interface web** - Diseño adaptativo a cualquier tamaño de pantalla
- ✅ **Bajo consumo de recursos** - Optimizado para ESP32

---

## 🔧 Hardware Requerido

### Microcontrolador
- **ESP32** (NodeMCU-32S o similar)

### Motores
- **Motor DC** (para las 4 ruedas o movimiento)
- **Servo Motor** (para el mecanismo de patada)
- **Driver Motor** (L298N o similar, con 4 salidas PWM)

### Sensores y Pantallas
- **Pantalla OLED SSD1306** (128x64 píxeles, I2C)
  - Dirección 1: 0x3C
  - Dirección 2: 0x3D

### Conectividad
- Conexión I2C para las pantallas

### Fuente de Poder
- Batería o fuente de alimentación para motores (5V)
- Conexión USB para programar el ESP32

---

## Estructura del Código

```
ESP_Carrito.ino
├── Includes (Librerías)
├── Configuración de Hardware
│   ├── Pantallas OLED
│   ├── Servo Motor
│   ├── Pines de Motores
│   └── WiFi
├── Setup() - Inicialización
├── Loop() - Bucle Principal
├── Interfaz Web HTML/CSS/JS
├── Controladores de Movimiento
│   ├── forward()
│   ├── backward()
│   ├── left()
│   ├── right()
│   └── stopCar()
├── Control de Servo
│   └── kick()
└── Funciones de Ojos (OLED)
    ├── drawEyeLeft()
    ├── drawEyeRight()
    └── blinkBoth()
```

---

## Código

### 1️⃣ **Includes - Librerías (Líneas 1-6)**

```cpp
#include <WiFi.h>              // Para conectividad WiFi
#include <WebServer.h>         // Para crear servidor web
#include <ESP32Servo.h>        // Para controlar servo motor
#include <Wire.h>              // Protocolo I2C (comunicación pantallas)
#include <Adafruit_SSD1306.h>  // Driver para pantallas OLED
#include <Adafruit_GFX.h>      // Librería gráfica base
```

**Propósito**: Importan todas las funciones necesarias para WiFi, servidor web, servo y pantallas OLED.

---

### 2️⃣ **Configuración de Pantallas OLED (Líneas 8-16)**

```cpp
#define SCREEN_WIDTH 128        // Ancho en píxeles
#define SCREEN_HEIGHT 64        // Alto en píxeles

#define OLED_LEFT_ADD 0x3C      // Dirección I2C pantalla izquierda
#define OLED_RIGHT_ADD 0x3D     // Dirección I2C pantalla derecha

// Crear objetos para cada pantalla
Adafruit_SSD1306 display_left(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SSD1306 display_right(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
```

**Propósito**: Define las características de las pantallas OLED (128x64 píxeles cada una) y sus direcciones I2C únicas.

---

### 3️⃣ **Configuración del Servo Motor (Líneas 18-27)**

```cpp
const int inServo = 14;         // Pin GPIO14 del ESP32 (salida PWM para servo)

// Ángulos de posición del servo
const int reposo = 85;          // Ángulo de reposo (sin patear)
const int preDisparo = 90;      // Ángulo antes de la patada
const int disparo = 20;         // Ángulo máximo de patada

Servo patada;                   // Objeto servo
```

**Propósito**: Configura el servo motor con sus ángulos de operación.
- **Reposo (85°)**: Posición neutral
- **Pre-Disparo (90°)**: Preparación para patada
- **Disparo (20°)**: Extensión máxima de la patada

---

### 4️⃣ **Configuración de Pines de Motores (Líneas 29-33)**

```cpp
const int in1 = 26;  // Motor izquierdo: Pin 1
const int in2 = 25;  // Motor izquierdo: Pin 2
const int in3 = 33;  // Motor derecho: Pin 1
const int in4 = 32;  // Motor derecho: Pin 2
```

**Propósito**: Define los pines de control para 2 motores DC (4 pines totales).
- **in1, in2**: Controlan motor izquierdo (dirección)
- **in3, in4**: Controlan motor derecho (dirección)

**Control de dirección**:
- Motor adelante: in1=LOW, in2=HIGH
- Motor atrás: in1=HIGH, in2=LOW

---

### 5️⃣ **Configuración WiFi (Líneas 35-40)**

```cpp
const char *ssid = "CARRITO:PP";        // Nombre de la red WiFi
const char *password = "12345678";      // Contraseña

WebServer server(80);                   // Servidor web en puerto 80 (HTTP)
```

**Propósito**: Define credenciales WiFi y el servidor web.

---

### 6️⃣ **Función Setup() - Inicialización (Líneas 43-107)**

#### a) Iniciar Serial (Línea 45)
```cpp
Serial.begin(115200);  // Comunicación con monitor serial a 115200 baud
```

#### b) Inicializar Pines de Motores (Líneas 48-51)
```cpp
pinMode(in1, OUTPUT);
pinMode(in2, OUTPUT);
pinMode(in3, OUTPUT);
pinMode(in4, OUTPUT);
```
**Propósito**: Configura los 4 pines como salidas digitales para controlar motores.

#### c) Inicializar Servo (Líneas 53-56)
```cpp
patada.attach(inServo);  // Conecta el servo al pin 14
patada.write(reposo);    // Lo posiciona en ángulo de reposo
delay(500);              // Espera medio segundo
```

#### d) Iniciar WiFi en Modo Access Point (Líneas 60-63)
```cpp
WiFi.softAP(ssid, password);  // Crea red WiFi propia
Serial.println("WiFi AP Started");
Serial.print("IP Address: ");
Serial.println(WiFi.softAPIP());
```
**Resultado**: El ESP32 actúa como router WiFi. Los dispositivos se conectan directamente a él.

#### e) Configurar Rutas Web (Líneas 66-72)
```cpp
server.on("/", handleRoot);    // "/" -> Página HTML principal
server.on("/F", forward);      // "/F" -> Adelante
server.on("/B", backward);     // "/B" -> Atrás
server.on("/L", left);         // "/L" -> Izquierda
server.on("/R", right);        // "/R" -> Derecha
server.on("/S", stopCar);      // "/S" -> Parar
server.on("/K", kick);         // "/K" -> Patada
```

**Propósito**: Mapea URLs a funciones. Cuando el navegador accede a `/F`, ejecuta `forward()`.

#### f) Iniciar I2C para Pantallas (Línea 78)
```cpp
Wire.begin(13, 12);  // SDA=GPIO13, SCL=GPIO12
```

#### g) Inicializar Pantallas OLED (Líneas 81-106)
```cpp
if (!display_left.begin(SSD1306_SWITCHCAPVCC, OLED_LEFT_ADD)) {
    Serial.println("No se encontró OLED izquierda (0x3C)");
    while(1) ;  // Se queda en bucle si no encuentra la pantalla
}
// Lo mismo para la derecha (dirección 0x3D)

display_left.clearDisplay();   // Limpia ambas pantallas
display_right.clearDisplay();

drawEyeLeft();                 // Dibuja los ojos
drawEyeRight();

display_left.display();        // Actualiza pantalla
display_right.display();
```

---

### 7️⃣ **Función Loop() - Bucle Principal (Líneas 110-121)**

```cpp
void loop() {
    server.handleClient();  // Procesa peticiones HTTP

    // Parpadeo automático de ojos cada 3 segundos
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 3000) {
        lastBlink = millis();
        blinkBoth();
    }
}
```

**Propósito**: 
- **`server.handleClient()`**: Espera y procesa comandos desde la web
- **Parpadeo**: Cada 3 segundos, los ojos parpadean

**`static unsigned long lastBlink`**: Variable que mantiene su valor entre ejecuciones del loop.

---

## 🎮 Métodos y Funciones

### **Control de Movimiento (4 Direcciones)**

#### **forward() - Adelante (Líneas 641-649)**
```cpp
void forward() {
    Serial.println("Adelante");
    digitalWrite(in1, LOW);   // Motor izq: sentido 1
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);   // Motor der: sentido 1
    digitalWrite(in4, HIGH);
    server.send(200, "text/plain", "Forward");
}
```
**Resultado**: Ambos motores giran en la misma dirección → carrito avanza.

---

#### **backward() - Atrás (Líneas 651-659)**
```cpp
void backward() {
    Serial.println("Atras");
    digitalWrite(in1, HIGH);  // Motor izq: sentido inverso
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);  // Motor der: sentido inverso
    digitalWrite(in4, LOW);
    server.send(200, "text/plain", "Backward");
}
```
**Resultado**: Ambos motores giran en sentido opuesto → carrito retrocede.

---

#### **left() - Izquierda (Líneas 661-669)**
```cpp
void left() {
    Serial.println("izq");
    digitalWrite(in1, HIGH);  // Motor izq: atrás
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);   // Motor der: adelante
    digitalWrite(in4, HIGH);
    server.send(200, "text/plain", "Left");
}
```
**Resultado**: Motor izquierdo atrás + motor derecho adelante → carrito gira a la izquierda.

---

#### **right() - Derecha (Líneas 671-679)**
```cpp
void right() {
    Serial.println("Der");
    digitalWrite(in1, LOW);   // Motor izq: adelante
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);  // Motor der: atrás
    digitalWrite(in4, LOW);
    server.send(200, "text/plain", "Right");
}
```
**Resultado**: Motor izquierdo adelante + motor derecho atrás → carrito gira a la derecha.

---

#### **stopCar() - Parar (Líneas 681-688)**
```cpp
void stopCar() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    server.send(200, "text/plain", "Stop");
}
```
**Resultado**: Todos los pines en LOW → motores se detienen.

---

### **Control del Servo - Patada (Líneas 691-706)**

```cpp
void kick() {
    // Responder primero para no bloquear el navegador
    server.send(200, "text/plain", "Kick");
    Serial.println("Pateando...");

    // Secuencia de patada
    patada.write(preDisparo);  // 90° - Preparación (500ms)
    delay(500);

    patada.write(disparo);     // 20° - Extensión (200ms)
    delay(200);

    patada.write(reposo);      // 85° - Reposo (300ms)
    delay(300);
}
```

**Secuencia de Patada**:
1. **0-500ms**: Posición de pre-disparo (90°) - prepara fuerza
2. **500-700ms**: Máxima extensión (20°) - ejecuta la patada
3. **700-1000ms**: Regresa a reposo (85°)

**Propósito**: Simula un movimiento natural de patada con aceleración y desaceleración.

---

### **Funciones de Ojos OLED**

#### **drawEyeLeft() - Ojo Izquierdo (Líneas 709-715)**
```cpp
void drawEyeLeft() {
    display_left.clearDisplay();              // Borra todo
    display_left.fillRoundRect(20, 12, 88, 40, 12, WHITE);  // Rectángulo redondeado blanco
    display_left.fillCircle(64, 32, 8, BLACK);              // Círculo negro (pupila)
    display_left.display();                   // Actualiza pantalla
}
```

**Parámetros de `fillRoundRect`**:
- `20, 12`: Posición X, Y (superior-izquierda)
- `88, 40`: Ancho, Alto
- `12`: Radio de esquinas redondeadas
- `WHITE`: Color de relleno

**Resultado**: Dibuja un ojo blanco con pupila negra.

---

#### **drawEyeRight() - Ojo Derecho (Líneas 717-723)**
```cpp
void drawEyeRight() {
    display_right.clearDisplay();
    display_right.fillRoundRect(20, 12, 88, 40, 12, WHITE);
    display_right.fillCircle(64, 32, 8, BLACK);
    display_right.display();
}
```
**Idéntica a la izquierda pero en la pantalla derecha.**

---

#### **blinkBoth() - Parpadeo (Líneas 725-738)**
```cpp
void blinkBoth() {
    // Cerrar los ojos (línea horizontal)
    display_left.clearDisplay();
    display_left.drawLine(20, 32, 108, 32, WHITE);  // Línea de párpado
    display_left.display();

    display_right.clearDisplay();
    display_right.drawLine(20, 32, 108, 32, WHITE);
    display_right.display();
    delay(120);  // Mantener ojos cerrados 120ms

    // Abrir los ojos (restaurar)
    drawEyeLeft();
    drawEyeRight();
}
```

**Secuencia**:
1. Dibuja una línea horizontal (cierre de párpado)
2. Espera 120ms
3. Redibuja los ojos abiertos

**Se ejecuta**: Cada 3 segundos en el `loop()`.

---

## 🌐 Interfaz Web (HTML/CSS/JavaScript)

La interfaz está embebida en el código como un string (`R"rawliteral(...)"`) de líneas 126-635.

### **Estructura HTML (Líneas 459-484)**

```html
<h2>ESP32 Control 🚗</h2>

<!-- JOYSTICK VIRTUAL -->
<div id="joystick">
    <div id="stick"></div>
</div>

<!-- BOTÓN DE PATADA -->
<button id="kick">⚽ PATEAR</button>

<!-- ESTADO -->
<div id="status">Detenido</div>
```

### **Lógica de Joystick (JavaScript, Líneas 488-630)**

#### **Función de Envío de Comandos (Líneas 489-491)**
```javascript
function sendCommand(cmd) {
    fetch("/" + cmd).catch(() => {});
}
```
Envía petición HTTP GET a rutas como `/F`, `/B`, etc.

#### **Control del Joystick (Líneas 500-571)**

```javascript
function moveJoystick(clientX, clientY) {
    // Calcula posición relativa al centro del joystick
    const rect = joystick.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    
    let x = clientX - centerX;  // Distancia X desde centro
    let y = clientY - centerY;  // Distancia Y desde centro
    
    // Limita movimiento al radio máximo
    const distance = Math.sqrt(x * x + y * y);
    if (distance > maxDistance) {
        x = (x / distance) * maxDistance;
        y = (y / distance) * maxDistance;
    }
    
    // Determina dirección basada en posición
    if (y < -threshold) command = "F";      // Arriba = Adelante
    else if (y > threshold) command = "B";  // Abajo = Atrás
    else if (x < -threshold) command = "L"; // Izquierda = Izquierda
    else if (x > threshold) command = "R";  // Derecha = Derecha
    else command = "S";                     // Centro = Parar
    
    // Solo envía si cambió el comando
    if (command !== lastCommand) {
        sendCommand(command);
    }
}
```

#### **Eventos Táctiles (Líneas 602-623)**
```javascript
joystick.addEventListener("touchstart", (e) => {
    active = true;
    moveJoystick(e.touches[0].clientX, e.touches[0].clientY);
});

document.addEventListener("touchmove", (e) => {
    if (active) {
        e.preventDefault();
        moveJoystick(e.touches[0].clientX, e.touches[0].clientY);
    }
});
```
Permite control mediante toque en dispositivos móviles.

---

## ⚙️ Configuración

### **Cambiar Credenciales WiFi**

Edita en el código:
```cpp
const char *ssid = "TU_SSID";
const char *password = "TU_PASSWORD";
```

### **Cambiar Ángulos del Servo**

```cpp
const int reposo = 85;      // Ajusta según tu servo
const int preDisparo = 90;
const int disparo = 20;
```

Prueba qué ángulos funcionan mejor para tu mecanismo de patada.

### **Cambiar Direcciones I2C de Pantallas**

Si tus pantallas usan otras direcciones:
```cpp
#define OLED_LEFT_ADD 0x3C   // Tu dirección 1
#define OLED_RIGHT_ADD 0x3D  // Tu dirección 2
```

---

## 📥 Instalación y Carga en ESP32

### **Paso 1: Instalar Arduino IDE**

1. Descarga desde: https://www.arduino.cc/en/software

### **Paso 2: Agregar Soporte para ESP32**

1. Abre Arduino IDE
2. Archivo → Preferencias
3. En "URLs adicionales de Gestor de tarjetas", agrega:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. OK
5. Herramientas → Placa → Gestor de tarjetas
6. Busca "esp32" e instala **"ESP32 por Espressif Systems"**

### **Paso 3: Instalar Librerías Requeridas**

En Arduino IDE: Herramientas → Gestor de librerías

Instala:
- ✅ **Adafruit SSD1306** - para pantallas OLED
- ✅ **Adafruit GFX Library** - gráficos base
- ✅ **Adafruit BusIO** - comunicación I2C
- ✅ **ESP32Servo** - control de servo

### **Paso 4: Conectar ESP32**

1. Conecta ESP32 a la PC vía USB
2. En Arduino IDE: 
   - Herramientas → Placa → Selecciona "ESP32 Dev Module"
   - Herramientas → Puerto → Selecciona el puerto COM (ej: COM3)
   - Herramientas → Velocidad de carga → 115200 baud

### **Paso 5: Descargar y Abrir el Código**

1. Descarga el archivo `ESP_Carrito.ino`
2. Abre en Arduino IDE

### **Paso 6: Compilar (Verificar)**

Presiona: **Ctrl + R** (o botón ✓)

Espera a que compile. Si ves "✓ Done compiling", está bien.

### **Paso 7: Subir a ESP32**

Presiona: **Ctrl + U** (o botón ➜)

Verás:
```
Connecting........____....
```

Espera a que se complete la carga.

### **Paso 8: Verificar Carga Exitosa**

1. Una vez cargado, en el Monitor Serial (Herramientas → Monitor Serial):
   - Velocidad: 115200 baud
   - Deberías ver:
     ```
     Todo bien :)
     WiFi AP Started
     IP Address: 192.168.4.1
     Todo bien :)))
     ```

2. Si los OLED están conectados:
   - Deberías ver:
     ```
     SSD1306 OLED found at 0x3C
     SSD1306 OLED found at 0x3D
     ```

---

## 🕹️ Uso de la Aplicación

### **Conectarse al WiFi**

1. En tu móvil/PC, busca redes WiFi disponibles
2. Conecta a: **"CARRITO:PP"** (contraseña: **#######**)
3. Abre navegador: **http://192.168.4.1** (Direccion IP del dispositivo)
---

## 📚 Librerías Utilizadas

### **WiFi.h**
- Control de conectividad WiFi
- Métodos: `WiFi.softAP()`, `WiFi.softAPIP()`

### **WebServer.h**
- Servidor web HTTP
- Métodos: `server.on()`, `server.handleClient()`, `server.send()`

### **ESP32Servo.h**
- Control PWM de servo motores
- Métodos: `servo.attach()`, `servo.write()`

### **Wire.h**
- Protocolo I2C
- Métodos: `Wire.begin()`, `Wire.write()`, `Wire.read()`

### **Adafruit_SSD1306.h**
- Driver para pantallas OLED SSD1306
- Métodos: `display.begin()`, `display.display()`, `display.clearDisplay()`

### **Adafruit_GFX.h**
- Funciones gráficas base
- Métodos: `fillRoundRect()`, `fillCircle()`, `drawLine()`
