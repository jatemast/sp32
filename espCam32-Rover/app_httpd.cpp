#include <esp32-hal-ledc.h>
#include "esp_http_server.h"
#include "Arduino.h"
#include "app_httpd.h"

httpd_handle_t camera_httpd = NULL;

// ----------------------
// VARIABLES EXTERNAS (solo declaradas)
// ----------------------
extern RobotMode currentMode;
extern int speed;
extern int noStop;
extern state actstate;

// Funciones externas
extern void moveForward();
extern void turnLeft();
extern void stopMotors();
extern void turnRight();
extern void moveBackward();

// ----------------------------------------------------------------
//   HANDLER DE CONTROL DE MOVIMIENTO
// ----------------------------------------------------------------
static esp_err_t cmd_handler(httpd_req_t *req)
{
    char* buf;
    size_t buf_len;
    char variable[32] = {0};
    char value[32] = {0};

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (!(httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
                  httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK)) 
            {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } 
    else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int val = atoi(value);
    int res = 0;

    // Ajustar velocidad
    if (!strcmp(variable, "speed"))
    {
        if      (val > 255) val = 255;
        else if (val < 0)   val = 0;
        speed = val;
    }
    else if (!strcmp(variable, "nostop"))
    {
        noStop = val;
    }
    else if (!strcmp(variable, "car"))
    {
        if (currentMode == MODE_REMOTE)
        {
            if (val == 1) moveForward();
            else if (val == 2) turnRight();
            else if (val == 3) stopMotors();
            else if (val == 4) turnLeft();
            else if (val == 5) moveBackward();
        }
        
        if (noStop != 1 && val == 3) { // Solo detener motores si noStop es 0 y el comando es STOP
            stopMotors();
        } else if (noStop != 1 && (val == 1 || val == 2 || val == 4 || val == 5)) {
            // Si noStop es 0 y se recibe un comando de movimiento, no detener los motores inmediatamente
            // Esto permite que el movimiento continúe hasta que se envíe un comando de parada explícito (val == 3)
            // o se levante el botón del control remoto.
            // La lógica de detener motores al soltar el botón se maneja en el HTML (onmouseup="move(3)").
        } else if (noStop == 1 && val == 3) {
            stopMotors(); // Si noStop es 1, permitir detener los motores explícitamente.
        }
        else {
            Serial.println("Robot en Modo Automático. Control remoto deshabilitado.");
        }
    }
    else 
    {
        Serial.println("variable desconocida");
        res = -1;
    }

    if(res){ return httpd_resp_send_500(req); }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}


// ----------------------------------------------------------------
//   HANDLER DE CAMBIO DE MODO (REMOTE / AUTO)
// ----------------------------------------------------------------
static esp_err_t mode_handler(httpd_req_t *req) 
{
    char* buf;
    size_t buf_len;
    char variable[32] = {0};
    char value[32] = {0};

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) 
    {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) 
        {
            if (!(httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
                  httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK))
            {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } 
        else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } 
    else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int val = atoi(value);

    if(!strcmp(variable, "mode")) 
    {
        if (val == 0) {
            currentMode = MODE_REMOTE;
            Serial.println("Modo: Control Remoto");
            stopMotors();
        } 
        else if (val == 1) {
            currentMode = MODE_AUTO;
            Serial.println("Modo: Automático");
            stopMotors();
        }
    }
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}


// ----------------------------------------------------------------
//   STATUS HANDLER (JSON PARA LA WEB)
// ----------------------------------------------------------------
static esp_err_t status_handler(httpd_req_t *req)
{
    static char json_response[1024];

    char * p = json_response;
    *p++ = '{';
    p+=sprintf(p, "\"speed\":%u,", speed);
    p+=sprintf(p, "\"noStop\":%u,", noStop);
    p+=sprintf(p, "\"currentMode\":%u", currentMode);
    *p++ = '}';
    *p = 0;

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}


// ----------------------------------------------------------------
//   PÁGINA HTML
// ----------------------------------------------------------------
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Control ESP32 Rover</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin: 20px; background-color: #f0f0f0; }
        .controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 300px; margin: 20px auto; }
        .control-button {
            padding: 20px; font-size: 24px; cursor: pointer; border: none; border-radius: 8px;
            background-color: #4CAF50; color: white; transition: background-color 0.3s;
        }
        .control-button.stop { background-color: #f44336; }
        .control-button.turn { background-color: #008CBA; }
        .control-button:active { background-color: #3e8e41; }
        .speed-control { margin-top: 20px; }
        input[type="range"] { width: 100%; margin-top: 10px; }
        .mode-buttons button { padding: 10px 20px; margin: 5px; font-size: 18px; border-radius: 5px; cursor: pointer; }
        .mode-buttons button.active { background-color: #555; color: white; }
    </style>
</head>
<body>
    <h1>Control ESP32 Rover</h1>

    <div class="mode-buttons">
        <button id="remoteModeBtn" class="active" onclick="setMode(0)">Control Remoto</button>
        <button id="autoModeBtn" onclick="setMode(1)">Modo Automático</button>
    </div>

    <div class="controls">
        <div class="spacer"></div>
        <button class="control-button" onmousedown="move(1)" onmouseup="move(3)">&#9650;</button> <!-- Arriba -->
        <div class="spacer"></div>

        <button class="control-button turn" onmousedown="move(4)" onmouseup="move(3)">&#9664;</button> <!-- Izquierda -->
        <button class="control-button stop" onmousedown="move(3)">STOP</button> <!-- Stop -->
        <button class="control-button turn" onmousedown="move(2)" onmouseup="move(3)">&#9654;</button> <!-- Derecha -->

        <div class="spacer"></div>
        <button class="control-button" onmousedown="move(5)" onmouseup="move(3)">&#9660;</button> <!-- Abajo -->
        <div class="spacer"></div>
    </div>

    <div class="speed-control">
        <label for="speedSlider">Velocidad: <span id="speedValue">255</span></label>
        <input type="range" id="speedSlider" min="0" max="255" value="255" onchange="setSpeed(this.value)">
    </div>

    <script>
        let currentMode = 0; // 0 for remote, 1 for auto

        function sendCommand(varName, val) {
            fetch(`/control?var=${varName}&val=${val}`)
                .then(response => {
                    if (!response.ok) {
                        console.error('Error al enviar comando:', response.statusText);
                    }
                })
                .catch(error => console.error('Error de red:', error));
        }

        function setMode(mode) {
            currentMode = mode;
            fetch(`/mode?var=mode&val=${mode}`)
                .then(response => {
                    if (!response.ok) {
                        console.error('Error al cambiar modo:', response.statusText);
                    }
                    updateModeButtons();
                })
                .catch(error => console.error('Error de red:', error));
        }

        function updateModeButtons() {
            const remoteBtn = document.getElementById('remoteModeBtn');
            const autoBtn = document.getElementById('autoModeBtn');
            if (currentMode === 0) {
                remoteBtn.classList.add('active');
                autoBtn.classList.remove('active');
            } else {
                remoteBtn.classList.remove('active');
                autoBtn.classList.add('active');
            }
        }

        function move(direction) {
            if (currentMode === 0) { // Solo permitir movimiento en modo remoto
                sendCommand('car', direction);
            } else {
                alert('El robot está en modo automático. Cambia a "Control Remoto" para moverlo.');
            }
        }

        function setSpeed(speed) {
            document.getElementById('speedValue').innerText = speed;
            sendCommand('speed', speed);
        }

        // Inicializar la interfaz al cargar la página
        window.onload = () => {
            updateModeButtons();
            setSpeed(document.getElementById('speedSlider').value); // Enviar velocidad inicial
        };
    </script>
</body>
</html>
)rawliteral";


// ----------------------------------------------------------------
//   INDEX HANDLER
// ----------------------------------------------------------------
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}


// ----------------------------------------------------------------
//   INICIO DEL SERVIDOR WEB
// ----------------------------------------------------------------
void startWebServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = { "/", HTTP_GET, index_handler, NULL };
    httpd_uri_t status_uri = { "/status", HTTP_GET, status_handler, NULL };
    httpd_uri_t cmd_uri    = { "/control", HTTP_GET, cmd_handler, NULL };
    httpd_uri_t mode_uri   = { "/mode", HTTP_GET, mode_handler, NULL };

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);

    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &mode_uri);
    }
}
