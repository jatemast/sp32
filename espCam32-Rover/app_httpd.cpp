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

            if (noStop != 1) stopMotors();
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
    << TU HTML SE MANTIENE IGUAL >>
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
