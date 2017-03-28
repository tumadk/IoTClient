#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define JSON_BUFF_DIMENSION 2500
#define JSON_BUFFER_SIZE 4000

const char* ssid          = "xxxxx";
const char* password      = "xxxxx";
const char* IoTServerHost = "xxxxx";
const char* IoTServerPath  = "";
const int IoTServerPort   = 3000;

WiFiClient client;

bool deviceRegistered = false;
bool deviceConfigFetched = false;

String wifi_ip;

struct sensor {
  int id;
  byte pin;
  String type;
  unsigned int timer;
  unsigned int timer_count;
  bool send_stats;
  bool active;
  long int value;
};

const int sensor_count = 10;
struct sensor sensors[sensor_count];

struct peripheral {
  int id;
  byte pin;
  String type;
  bool state_on;
  bool state;
  bool active;
  bool toggle;
  int toggle_timer_count;
  int toggle_timer;
};
const int peripheral_count = 10;
struct peripheral peripherals[peripheral_count];

void setup() {

  Serial.begin(115200);

  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  WiFi.onEvent(WiFiEvent);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.println("Wait for WiFi... ");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("ChipID: ");
  Serial.println(ESP.getChipId());

  /*
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D4, HIGH);
  */

}


void turnOffEverything() {

}

void getURL(String url) {

  if (!client.connect(IoTServerHost, IoTServerPort)) {
    Serial.println("connection failed");
    return;
  }
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + IoTServerHost + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  while (client.available()) {
    String str = client.readStringUntil('\r');
    str.replace("\n", "");
    //text = new char[str.length() + 1];
    //strcpy(text, str.c_str());
    //text = temp.c_str();
    if (str != "") {
      if (str.substring(0, 1) == "/") {
        //Serial.print("STR: ");
        //Serial.println(str);

        int j = getNextSubstringPlacement(str, 1);

        //Serial.print("j: ");
        //Serial.println(j);
        if (j != -1) {
          int x = 0;
          int j2 = getNextSubstringPlacement(str, j + 1);
          while (j2 != -1) {
            //Serial.print("j2: ");
            //Serial.println(j2);
            x++;
            j2 = getNextSubstringPlacement(str, j2 + 1);
          }
          //Serial.print("X: ");
          //Serial.println(x);
          String res[x + 1];
          res[0] = str.substring(1, j);
          j2 = getNextSubstringPlacement(str, j + 1);
          int x2 = 1;
          int i = j + 1;
          while (j2 != -1) {
            res[x2] = str.substring(i, j2);
            x2++;
            i = j2 + 1;
            j2 = getNextSubstringPlacement(str, j2 + 1);
          }
          if (x > 0) {
            if (res[0] == "device") {
              if (res[1] == "registered") {
                if (res[2] == "true") {
                  deviceRegistered = true;
                } else {
                  deviceRegistered = false;
                }
              }
            } else if (res[0] == "peripheral") {
              bool found = false;
              int next = -1;
              for (int z = 0; z < peripheral_count; z++) {
                if (peripherals[z].id == res[1].toInt()) {

                  String pinStr = res[2];
                  if (pinStr == "D0") {
                    peripherals[z].pin = D0;
                  } else if (pinStr == "D1") {
                    peripherals[z].pin = D1;
                  } else if (pinStr == "D2") {
                    peripherals[z].pin = D2;
                  } else if (pinStr == "D3") {
                    peripherals[z].pin = D3;
                  } else if (pinStr == "D4") {
                    peripherals[z].pin = D4;
                  } else if (pinStr == "D5") {
                    peripherals[z].pin = D5;
                  } else if (pinStr == "D6") {
                    peripherals[z].pin = D6;
                  } else if (pinStr == "D7") {
                    peripherals[z].pin = D7;
                  } else if (pinStr == "D8") {
                    peripherals[z].pin = D8;
                  }

                  if (peripherals[z].toggle_timer_count != res[4].toInt()) {
                    peripherals[z].toggle_timer = 0;
                  }
                  peripherals[z].type = res[3];
                  peripherals[z].toggle_timer_count = res[6].toInt();
                  peripherals[z].state_on = (res[4].toInt() == 1) ? true : false;
                  peripherals[z].toggle = (res[5].toInt() == 1) ? true : false;
                  peripherals[z].active = true;

                  pinMode(peripherals[z].pin, OUTPUT);
                  digitalWrite(peripherals[z].pin, (peripherals[z].state_on)?HIGH:LOW);
                  peripherals[z].state = peripherals[z].state_on;

                  found = true;
                } else {
                  if (next == -1 && (peripherals[z].id == 0 || !peripherals[z].active)) {
                    next = z;
                  }
                }
              }
              if (!found && next != -1) {

                Serial.println("peripherals");

                peripherals[next].id = res[1].toInt();
                String pinStr = res[2];
                if (pinStr == "D0") {
                  peripherals[next].pin = D0;
                } else if (pinStr == "D1") {
                  peripherals[next].pin = D1;
                } else if (pinStr == "D2") {
                  peripherals[next].pin = D2;
                } else if (pinStr == "D3") {
                  peripherals[next].pin = D3;
                } else if (pinStr == "D4") {
                  peripherals[next].pin = D4;
                } else if (pinStr == "D5") {
                  peripherals[next].pin = D5;
                } else if (pinStr == "D6") {
                  peripherals[next].pin = D6;
                } else if (pinStr == "D7") {
                  peripherals[next].pin = D7;
                } else if (pinStr == "D8") {
                  peripherals[next].pin = D8;
                }
                peripherals[next].type = res[3];
                peripherals[next].toggle_timer_count = res[6].toInt();
                peripherals[next].state_on = (res[4].toInt() == 1) ? true : false;
                peripherals[next].toggle = (res[5].toInt() == 1) ? true : false;
                peripherals[next].toggle_timer = millis();
                peripherals[next].active = true;
                peripherals[next].state = peripherals[next].state_on;

                pinMode(peripherals[next].pin, OUTPUT);
                digitalWrite(peripherals[next].pin, (peripherals[next].state_on)?HIGH:LOW);

                found = true;

              } 
              if (found) {
                deviceConfigFetched = true;
              }
              
            } else if (res[0] == "sensor") {
              bool found = false;
              int next = -1;
              for (int z = 0; z < sensor_count; z++) {
                if (sensors[z].id == res[1].toInt()) {

                  String pinStr = res[2];
                  if (pinStr == "A0") {
                    sensors[z].pin = A0;
                  }
                  sensors[z].type = res[3];
                  if (sensors[z].timer_count != res[4].toInt()) {
                    sensors[z].timer = 0;
                  }
                  sensors[z].timer_count = res[4].toInt();
                  sensors[z].send_stats = (res[5].toInt() == 1) ? true : false;
                  sensors[z].active = true;
                  found = true;
                } else {
                  if (next == -1 && (sensors[z].id == 0 || !sensors[z].active)) {
                    next = z;
                  }
                  //Serial.println(sensors[z].id);
                }
              }
              if (!found && next != -1) {

                sensors[next].id = res[1].toInt();
                String pinStr = res[2];
                if (pinStr == "A0") {
                  sensors[next].pin = A0;
                }
                sensors[next].type = res[3];
                sensors[next].timer_count = res[4].toInt();
                sensors[next].send_stats = (res[5].toInt() == 1) ? true : false;
                sensors[next].active = true;
                
                found = true;

              } 
              if (found) {
                deviceConfigFetched = true;
              }
            }
          }
        }
      }
    }
  }
}

void loop() {

  if (deviceRegistered && deviceConfigFetched) {

    for (int z = 0; z < sensor_count; z++) {
      if (sensors[z].active) {
        if (millis() - sensors[z].timer >= sensors[z].timer_count || sensors[z].timer == 0) {
          if (sensors[z].type == "analogread") {
            /*
            sensors[z].value = analogRead(sensors[z].pin);
            Serial.print("Sensor value: ");
            Serial.println(sensors[z].value);
            if (sensors[z].send_stats) {
              sendStats(sensors[z].id, sensors[z].value);
            }
            */
          }
          sensors[z].timer = millis();
        }
      }
    }
    for (int z = 0; z < peripheral_count; z++) {
      if (peripherals[z].active) {
        if (millis() - peripherals[z].toggle_timer >= peripherals[z].toggle_timer_count || peripherals[z].toggle_timer == 0) {
          if (peripherals[z].type == "LED") {
            if (peripherals[z].toggle) {
              digitalWrite(peripherals[z].pin, (peripherals[z].state)?LOW:HIGH);
              peripherals[z].state = !peripherals[z].state;
            }
          }
          peripherals[z].toggle_timer = millis();
        }
      }
    }    
    delay(100);

  } else if (deviceRegistered && !deviceConfigFetched) {

    delay(2000);
    turnOffEverything();
    Serial.println("Trying to fetch configuration");
    fetchDeviceConfig();

  } else if (!deviceRegistered) {
    delay(2000);
    turnOffEverything();
    Serial.println("Trying to register device");
    registerDevice();
  }
}


/* functions */

int getNextSubstringPlacement(String str, int start) {

  int j = -1;
  for (int i = start; i <= str.length(); i++) {
    if (j == -1) {
      if (str.substring(i, i + 1) == "/") {
        j = i;
      }
    }
  }
  return j;
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      wifi_ip = WiFi.localIP().toString();
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      Serial.println("WiFi lost connection");
      deviceRegistered = false;
      break;
  }
}

void sendStats(int id, int value) {

  String url = IoTServerPath;
  url += "/device/sensor-reading/";
  url += "?deviceSerial=";
  url += ESP.getChipId();
  url += "&deviceIP=";
  url += wifi_ip;
  url += "&id=";
  url += id;
  url += "&value=";
  url += value;
  getURL(url);

}

void registerDevice() {

  String url = IoTServerPath;
  url += "/device/connect/";
  url += "?deviceSerial=";
  url += ESP.getChipId();
  url += "&deviceIP=";
  url += wifi_ip;
  getURL(url);

}

void fetchDeviceConfig() {

  String url = IoTServerPath;
  url += "/device/fetch-config/";
  url += "?deviceSerial=";
  url += ESP.getChipId();
  url += "&deviceIP=";
  url += wifi_ip;
  getURL(url);

}

