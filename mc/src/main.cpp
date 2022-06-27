#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "RingBuffer.h"
#include "stats.h"
#include "sensor.h"
#include "settings.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocket ws_stats("/ws/stats");

SensorSettings settings;

void setup_filesystem(void) {
  LittleFS.begin();
  if ( LittleFS.exists("/index.html") ) {
    Serial.println("I: found /index.html");
  }
  else if ( LittleFS.exists("/index.html.gz") ){
    Serial.println("I: found /index.html.gz");
  }  
  else {
    Serial.println("E: found no index.html.*");
  }
}


void setup_WiFi(void) {
  WiFi.begin("BabyStube", "PaulaNadja1977");
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.printf("IPv6: %s\n", WiFi.enableIpV6() ? "enabled" : "error enabling");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIPv6());
}

/*
void setup_WiFi(void) {
  // Connect to Wi-Fi network with SSID and password
  Serial.print("starting Access Point...");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP("radar", "");
  

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}
*/

void setup_webserver(void) {
  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(200, "text/html", "Servus vom MC");
  }); */
  server.addHandler(&ws);
  server.addHandler(&ws_stats);
  server.begin();

  server.serveStatic("/",         LittleFS, "/").setDefaultFile("index.html");
  server.serveStatic("/log",      LittleFS, "/").setDefaultFile("index.html");
  server.serveStatic("/settings", LittleFS, "/").setDefaultFile("index.html");
  //server.serveStatic("/css/",     LittleFS, "/css/");
  //server.serveStatic("/fonts/",   LittleFS, "/fonts/");
  //server.serveStatic("/scripts/", LittleFS, "/scripts/");
}

void setup_mDNS(void) {
  if (!MDNS.begin("radar")) {
    Serial.println("Error setting up MDNS responder!");
  }
  else {
    Serial.println("mDNS responder started");
  }
}

STATS g_stats[2];
STATS g_last_stats[2];

void print_stats(const STATS stats[], const STATS last_stats[]) {

	Serial.printf("[avail0: %5lu|%5lu] [ok: %2lu|%2lu] [err: %2lu|%2lu] [less4: %5lu|%5lu] [ne: %5lu|%5lu] [notFF: %5lu|%5lu] [overwrite: %2lu|%2lu] [loops: %5lu]\n",
		stats[0].avail0      - last_stats[0].avail0,        stats[1].avail0      - last_stats[1].avail0,
		stats[0].data_ok     - last_stats[0].data_ok,       stats[1].data_ok     - last_stats[1].data_ok, 
		stats[0].data_error  - last_stats[0].data_error,    stats[1].data_error  - last_stats[1].data_error,
		stats[0].less4       - last_stats[0].less4,         stats[1].less4       - last_stats[1].less4,
		stats[0].ne          - last_stats[0].ne,            stats[1].ne          - last_stats[1].ne,
		stats[0].notFF	     - last_stats[0].notFF,         stats[1].notFF	     - last_stats[1].notFF,
    stats[0].overwrite,                                 stats[1].overwrite,
    stats[0].loops       - last_stats[0].loops
  );
}


const char* getSerialErrMsg(const hardwareSerial_error_t err) {
  const char* errStr;
  switch (err)
  {
    case hardwareSerial_error_t::UART_BREAK_ERROR:       errStr = "UART_BREAK_ERROR"; break;
    case hardwareSerial_error_t::UART_BUFFER_FULL_ERROR: errStr = "UART_BUFFER_FULL_ERROR"; break;
    case hardwareSerial_error_t::UART_FIFO_OVF_ERROR:    errStr = "UART_FIFO_OVF_ERROR"; break;
    case hardwareSerial_error_t::UART_FRAME_ERROR:       errStr = "UART_FRAME_ERROR"; break;
    case hardwareSerial_error_t::UART_PARITY_ERROR:      errStr = "UART_PARITY_ERROR"; break;
    default:                                             errStr = "n/a"; break;
  }
  return errStr;
}

void onSensorPairReady(ValuePair* pair) {

  const int diff = pair->val[0] - pair->val[1];
  if ( diff > 0 ) {
    digitalWrite(25,LOW);
    digitalWrite(26,HIGH);
  }
  else {
    digitalWrite(26,LOW);
    digitalWrite(25,HIGH);
  }

  {
    static char jsonReply[32];

    sprintf(jsonReply, "{\"l\":%u,\"r\":%u}"
          , pair->val[0]
          , pair->val[1] );

    ws.textAll(jsonReply);
  }
}

void setup_sensors(void) {
  Serial1.onReceive( []() { serialOnReceive(&Serial1, 0, &(g_stats[0]), onSensorPairReady); } );
  Serial2.onReceive( []() { serialOnReceive(&Serial2, 1, &(g_stats[1]), onSensorPairReady); } );

  Serial1.onReceiveError( [](hardwareSerial_error_t err) { Serial.printf("E: #0 - %s\n", getSerialErrMsg(err)); } );
  Serial1.onReceiveError( [](hardwareSerial_error_t err) { Serial.printf("E: #1 - %s\n", getSerialErrMsg(err)); } );
}

void setup_settings() {
  String err;
  if ( !settings.load_from_file(&err) ) {
    Serial.printf("E: load settings. %s\n", err.c_str());
  }
  else {
    Serial.printf("I: load settings ok.\n");
  }
}

typedef void (pf_every_ms)(void);

void every(const unsigned int every_millis, pf_every_ms action) {
  static unsigned long last_millis = 0;
  
  const unsigned long currMillis = millis();
  if ( currMillis - last_millis > every_millis) {
    action();
    last_millis = currMillis;
  }
}

void setup() {
  Serial.begin(9600);
  setup_filesystem();
  setup_WiFi();
  setup_webserver();
  setup_mDNS();
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  
  Serial1.begin(9600, SERIAL_8N1, 18, 19);
  setup_sensors();
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  setup_settings();
}

StaticJsonDocument<256> json_stats;
String json_stats_str;

void loop() {
  ws.cleanupClients();

  every(
    2000,
    []() {
      //print_stats(g_stats, g_last_stats);	
      
      if ( ws_stats.count() > 0 ) {
        json_stats.clear();
        json_stats["data_ok"]    = g_stats[0].data_ok    - g_last_stats[0].data_ok;
        json_stats["data_error"] = g_stats[0].data_error - g_last_stats[0].data_error;
        json_stats["loops"]      = g_stats[0].loops      - g_last_stats[0].loops;
        json_stats_str.clear();
        serializeJson(json_stats, json_stats_str);
        Serial.print(json_stats_str);
        ws_stats.textAll(json_stats_str);
      }

      memcpy(g_last_stats, g_stats, 2 * sizeof(STATS));
    } 
  );

  g_stats[0].loops++;

}