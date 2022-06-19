#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

#include "RingBuffer.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup_filesystem(void) {
  LittleFS.begin();
  if ( LittleFS.exists("/index.html") ) {
    Serial.println("I: LittleFS found /index.html");
  }
  else {
    Serial.println("E: LittleFS found no /index.html");
  }  
}

/*
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
*/
void setup_WiFi(void) {
  // Connect to Wi-Fi network with SSID and password
  Serial.print("starting Access Point...");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP("radar", "");
  

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void setup_webserver(void) {
  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(200, "text/html", "Servus vom MC");
  }); */
  server.addHandler(&ws);
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

typedef struct {
	unsigned long avail0;     
	unsigned long data_ok;    
	unsigned long data_error;
	unsigned long less4;
	unsigned long ne;
	unsigned long notFF;
  unsigned long overwrite;
  unsigned long loops;
} STATS;

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

bool checksum_valid_2(unsigned char* data) // == char[4]
{
	const int checksum = ( data[0] + data[1] + data[2] ) & 0xFF;
	return checksum == data[3];
}

bool read_millimeter_from_serial(Stream* serial, STATS* stats, unsigned int* millimeter) {
  unsigned char data[4];
  data[0] = 0xFF;

  for (;;) {
    int b = serial->read();
    if ( b == -1) {
      break;
    }    
    else if ( b != 0xFF ) {
      stats->notFF += 1;
    }
    else if ( serial->readBytes(&(data[1]),3) != 3 ) {
      stats->ne += 1;
    }
    else if ( ! checksum_valid_2(data) ) {
      stats->data_error += 1;
    }
    else {
      stats->data_ok += 1;
		  *millimeter = ( data[1] << 8 ) + data[2];
      return true;
      //sprintf(jsonReply, "{\"l\":%u,\"r\":0}", millimeter);
      //ws.textAll(jsonReply);
    }
  }
  return false;
}

bool          pair_has_value[2] = {false, false};

ValuePair   pair;
ValuePair   sum_pair;
RingBuffer  ring_buf;

void serialOnReceive(Stream* serial, const int sensor, STATS* stats) {
  static char jsonReply[32];
  unsigned int millimeter;

  if (!read_millimeter_from_serial(serial, stats, &millimeter)) {
    // 
  }
  else {
    if ( pair_has_value[sensor] ) {
      stats->overwrite += 1;
    }

    pair.val[sensor]       = millimeter;
    pair_has_value[sensor] = true;

    if ( pair_has_value[1-sensor] ) {

      ring_buf.push(pair);

      const int avg_range = 10;
      int summed = ring_buf.sum_last_values(avg_range, &sum_pair);
      if ( summed < (avg_range/2) ) {
        sum_pair.reset();
      }
      
      sprintf(jsonReply, "{\"l\":%u,\"r\":%u}"
        , sum_pair.val[0] / avg_range
        , sum_pair.val[1] / avg_range);

      ws.textAll(jsonReply);
      
      pair_has_value[0] = false;
      pair_has_value[1] = false;
    }
  }
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

void ser1OnReceive(void) {
  serialOnReceive(&Serial1, 0, &(g_stats[0]));
}
void ser2OnReceive(void) {
  serialOnReceive(&Serial2, 1, &(g_stats[1]));
}

void ser1OnError(hardwareSerial_error_t err) {
  Serial.printf("E: #0 - %s\n", getSerialErrMsg(err));
}
void ser2OnError(hardwareSerial_error_t err) {
  Serial.printf("E: #1 - %s\n", getSerialErrMsg(err));
}

void setup_sensors(void) {
  //Serial1.onReceive     (ser1OnReceive);
  //Serial1.onReceiveError(ser1OnError);
  //Serial2.onReceive     (ser2OnReceive);
  //Serial2.onReceiveError(ser2OnError);

  Serial1.onReceive( []() { serialOnReceive(&Serial1, 0, &(g_stats[0])); } );
  Serial2.onReceive( []() { serialOnReceive(&Serial2, 1, &(g_stats[1])); } );

  Serial1.onReceiveError( [](hardwareSerial_error_t err) { Serial.printf("E: #0 - %s\n", getSerialErrMsg(err)); } );
  Serial1.onReceiveError( [](hardwareSerial_error_t err) { Serial.printf("E: #1 - %s\n", getSerialErrMsg(err)); } );


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
}



void loop() {
  ws.cleanupClients();

  every(
    1000,
    []() {
      print_stats(g_stats, g_last_stats);	
		  memcpy(g_last_stats, g_stats, 2 * sizeof(STATS));
    } 
  );

  g_stats[0].loops++;

}