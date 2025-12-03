#include <esp32-hal-ledc.h>
#include "esp_http_server.h"
#include "Arduino.h"

int speed = 255;
int noStop = 0;

enum state {fwd,rev,stp};
state actstate = stp;

static esp_err_t cmd_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
    char variable[32] = {0,};
    char value[32] = {0,};

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
                httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
            } else {
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
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int val = atoi(value);
    int res = 0;
    
    //Remote Control Car
    //Don't use channel 1 and channel 2
    else if(!strcmp(variable, "speed"))
    {
      if      (val > 255) val = 255;
      else if (val <   0) val = 0;
      speed = val;
    }
    else if(!strcmp(variable, "nostop"))
    {
      noStop = val;
    }
    else if(!strcmp(variable, "car")) {  
      if (val==1) {
        Serial.println("Forward");
        actstate = fwd;     
        ledcWrite(4,speed);  // pin 12
        ledcWrite(3,0);      // pin 13
        ledcWrite(5,speed);  // pin 14  
        ledcWrite(6,0);      // pin 15   
        delay(200);
      }
      else if (val==2) {   
        Serial.println("TurnLeft");
        ledcWrite(3,0);
        ledcWrite(5,0); 
        //if      (actstate == fwd) { ledcWrite(4,speed); ledcWrite(6,    0); }
        //else if (actstate == rev) { ledcWrite(4,    0); ledcWrite(6,speed); }
        //else                      { ledcWrite(4,speed); ledcWrite(6,speed); }
        ledcWrite(4,speed); 
        ledcWrite(6,speed);
              
        delay(100);  
        ledcWrite(4,0);
        ledcWrite(6,0);
      }
      else if (val==3) {
        Serial.println("Stop"); 
        actstate = stp;       
        ledcWrite(4,0);
        ledcWrite(3,0);
        ledcWrite(5,0);     
        ledcWrite(6,0);  
      }
      else if (val==4) {
        Serial.println("TurnRight");
        ledcWrite(4,0);
        ledcWrite(6,0); 
        //if      (actstate == fwd) { ledcWrite(3,    0); ledcWrite(5,speed); }
        //else if (actstate == rev) { ledcWrite(3,speed); ledcWrite(5,    0); }
        //else                      { ledcWrite(3,speed); ledcWrite(5,speed); }
        ledcWrite(3,speed); 
        ledcWrite(5,speed);
              
        delay(100);

        ledcWrite(3, 0);
        ledcWrite(5, 0);
        
      }
      else if (val==5) {
        Serial.println("Backward");  
        actstate = rev;      
        ledcWrite(4,0);
        ledcWrite(3,speed);
        ledcWrite(5,0);  
        ledcWrite(6,speed); 
        delay(200);              
      }
      if (noStop!=1) 
      {
        ledcWrite(3, 0);
        ledcWrite(4, 0);  
        ledcWrite(5, 0);  
        ledcWrite(6, 0);
      }         
    }        
    else 
    { 
      Serial.println("variable");
      res = -1; 
    }

    if(res){ return httpd_resp_send_500(req); }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req){
    static char json_response[1024];

    char * p = json_response;
    *p++ = '{';
    p+=sprintf(p, "\"speed\":%u,", speed);
    p+=sprintf(p, "\"noStop\":%u", noStop);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <title>ROBOT LIMPIADOR ESP32CAM ROBOT</title>
        <style>
            *{
                padding: 0; margin: 0;
                font-family:monospace;
            }
        canvas {
        margin: auto;
        display: block;

        }
        .tITULO{
            text-align: center;
            color: rgb(97, 97, 97);
            
        }
        .LINK{
            color: red;
            width: 60px;
            margin: auto;
            display: block;
            font-size: 14px;
        }
        .cont_stream{
            width: 90%;
            max-width: 700px;

            border: 1px solid red;
            margin: auto;
            display:block;
        }
        .cont_flex{
            margin: 20px auto 20px;
            width: 70%;
            max-width: 400px;
            display: flex;
            flex-wrap: wrap;
            justify-content: space-around;
        }
        .cont_flex button{
            width: 70px;
            height: 30px;
            border: none;
            background-color: red;
            border-radius: 10px;
            color: white;

        }
        .cont_flex button:active{
            background-color: green;
        }
        input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#cecece;cursor:pointer;margin:0}
        input[type=range]:focus{outline:0}
        input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}
        input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}

        </style>
    </head>
    <body>
         
        <h1 class="tITULO">ROBOT LIMPIADOR</h1>
        <a href="https://www.youtube.com/channel/UCKD9PvMAW0nYi681AZbSZSQ/videos" class="LINK">YOUTUBE</a>


        <div class="cont_flex"><div><input type="checkbox" style="margin-right: 5px;" id="nostop" onclick="var noStop=0;if (this.checked) noStop=1;fetch(document.location.origin+'/control?var=nostop&val='+noStop);">No Stop</div></div>

        <div class="cont_flex">     
            <button type="button" id="forward" onclick="fetch(document.location.origin+'/control?var=car&val=1');">Forward</button>
        </div>

        <div class="cont_flex">     
            <button type="button" id="turnleft" onclick="fetch(document.location.origin+'/control?var=car&val=4');">TurnLeft</button>
            <button type="button" id="stop" onclick="fetch(document.location.origin+'/control?var=car&val=3');">Stop</button>
            <button type="button" id="turnright" onclick="fetch(document.location.origin+'/control?var=car&val=2');">TurnRight</button>  
        </div>

        <div class="cont_flex">     
            <button type="button" id="backward" onclick="fetch(document.location.origin+'/control?var=car&val=5');">Backward</button>
        </div>

        <div class="cont_flex">  
            <div style="display: flex;align-items: center;">Speed <input type="range" id="speed" min="0" max="255" value="255" onchange="try{fetch(document.location.origin+'/control?var=speed&val='+this.value);}catch(e){}"></div>
        </div>

        <script>
            document.addEventListener(
            'DOMContentLoaded',function(){
                function b(B){let C;switch(B.type){case'checkbox':C=B.checked?1:0;break;case'range':case'select-one':C=B.value;break;case'button':case'submit':C='1';break;default:return;}const D=`${c}/control?var=${B.id}&val=${C}`;fetch(D).then(E=>{console.log(`request to ${D} finished, status: ${E.status}`)})}var c=document.location.origin;fetch(`${c}/status`).then(function(B){return B.json()}).then(function(B){document.querySelectorAll('.default-action').forEach(C=>{i(C,B[C.id],!1)})});document.querySelectorAll('.default-action').forEach(B=>{B.onchange=()=>b(B)});});
        
        </script>
    </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

void startWebServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t status_uri = {
        .uri       = "/status",
        .method    = HTTP_GET,
        .handler   = status_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t cmd_uri = {
        .uri       = "/control",
        .method    = HTTP_GET,
        .handler   = cmd_handler,
        .user_ctx  = NULL
    };
    
    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
    }
}
