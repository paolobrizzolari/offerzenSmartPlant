/**
Rouan van der Ende (rouan@openboard.co.za)
2021 
https://github.com/rvdende/offerzenSmartPlant

OpenSource.. use are you please.
**/

#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// SMART PLANT
#define LED D4
#define PUMP D6
#define MOISTURESENSORANALOG A0
#define MOISTURESENSORDIGITAL D5
#define CONFIG_ID "smartplant"
#define CONFIG_TYPE "smartplant"
volatile bool  state_led = true;
volatile int state_moisture = -1;
volatile bool state_moisturedigital = false;
volatile bool  state_pump = false;
volatile bool  control_pump = false;
volatile bool  control_led = false;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(CONFIG_WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DynamicJsonDocument incomingjson(128);
  DeserializationError error = deserializeJson(incomingjson, payload, length);
  
  if (error)
  {
      Serial.println(error.c_str());
      return;
  }
  
  serializeJsonPretty(incomingjson, Serial);

  if (incomingjson.containsKey("data"))
  {
      JsonObject data = incomingjson["data"];
      // TOGGLE PUMP OR LED
      if (data.containsKey("control_pump")) control_pump = !control_pump;
      if (data.containsKey("control_led")) control_led = !control_led;      
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    const char *id = CONFIG_ID;
    
    // add random?
    // clientId += String(random(0xffff), HEX);
    const char *apikey = CONFIG_APIKEY;
    char mqttPassword[64];
    const char *first = "key-";
    strcpy(mqttPassword, first);
    strcat(mqttPassword, apikey); 

    // Attempt to connect
    if (client.connect(id,"api", mqttPassword)) {
      Serial.println("connected.. subscribing.");

      DynamicJsonDocument doc(96);
      JsonObject root = doc.to<JsonObject>();
      root["apikey"] = CONFIG_APIKEY;
      root["id"] = CONFIG_ID; // remove to subscribe to all devices
      char output[96];
      serializeJson(doc, output);
      client.subscribe(output);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(PUMP, OUTPUT);
  pinMode(MOISTURESENSORDIGITAL,INPUT);
  digitalWrite(LED, state_led);
  digitalWrite(PUMP, state_pump);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(CONFIG_SERVER, CONFIG_MQTTPORT);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  // UPDATE DEVICE STATES
  if (control_pump != state_pump) {
    state_pump = control_pump;
    digitalWrite(PUMP, state_pump);
    lastMsg = -10000; // to force an update immediately
  }

  if (control_led != state_led) {
    state_led = control_led;
    digitalWrite(LED, state_led);
    lastMsg = -10000; // to force an update immediately
  }  

  // UPDATE SERVER ON STATE
  if (now - lastMsg > 2000) {
    state_moisture = analogRead(MOISTURESENSORANALOG);
    state_moisturedigital = digitalRead(MOISTURESENSORDIGITAL);
    lastMsg = now;
    publishState();
  }
}


/**
example packet:
{
  "id": "smartplant",
  "type": "smartplant",
  "data": {
    "state_pump": false,
    "state_moisture": 283
  }
}
**/
void publishState()
{    
    Serial.println("Publish state:");
    DynamicJsonDocument doc(96);
    JsonObject root = doc.to<JsonObject>();
    root["id"] = CONFIG_ID;
    root["type"] = CONFIG_TYPE;
    JsonObject data = root.createNestedObject("data");
    data["state_pump"] = state_pump;
    data["state_moisture"] = state_moisture;
    char textbuffer[96];
    size_t length = serializeJson(doc, textbuffer);
    Serial.println(textbuffer);
    client.publish(CONFIG_APIKEY, textbuffer, length);
}