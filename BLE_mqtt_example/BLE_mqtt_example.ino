#include <WiFi.h>
#include <PubSubClient.h>

#define LED_PIN 2
#define BUTTON_PIN 4

const char* ssid = "aaaaaaaaa";
const char* password = "aaaaaaaaaaaaa";

const char* mqttHost = "";
const uint16_t mqttPort = 1883;
const char* mqttU = "aaaaaaa";
const char* mqttP = "aaaaaaaaaaaaaa";

int lastButtonState = HIGH;

bool val = true;

void pinSetup(){
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message: ");
  Serial.println(message);

  if (message == "True") {
    val = true;
  } else {
    val = false;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqttU, mqttP)) {
      Serial.println("connected");

      client.subscribe("things/signal");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }


  }
}

void setup() {
  pinSetup();

  Serial.begin(115200);
  delay(1000);
  printf("Hello World!\n");

  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttHost, 1883);
  client.setCallback(callback);


}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int buttonState = digitalRead(BUTTON_PIN);

  if(lastButtonState == HIGH && buttonState == LOW){
    bool newVal = !val;
    const char* msg = newVal ? "true" : "false";
    client.publish("things/toggle", msg);
    delay(500);
  }

  if(val){
    digitalWrite(LED_PIN,HIGH);
  }else{
    digitalWrite(LED_PIN,LOW);
  }

}