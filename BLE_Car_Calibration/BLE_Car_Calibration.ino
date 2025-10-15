#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string>

using namespace std;

const int motor1Pin1 = 27, motor1Pin2 = 26, enable1Pin = 14;
const int motor2Pin1 = 32, motor2Pin2 = 33, enable2Pin = 25;

const int freq = 30000;
const int pwmChannel1 = 0,pwmChannel2 = 1;
const int resolution = 8;
int dutyCycle = 255;

const int mqttPort = 1883;
const char *ssid = "Br", *password = "dk-dutisa", *mqttHost = "148.230.101.206", *mqttU = "dk", *mqttP = "dkdkdk";
const string topicMotor = "things/motor/";

String dir;
String en;
int unit;

WiFiClient espClient;
PubSubClient client(espClient);

void motorLogic() {

  // ledcWrite(enable1Pin, dutyCycle);   
  // ledcWrite(enable2Pin, dutyCycle); 
  // digitalWrite(motor1Pin1, LOW);
  // digitalWrite(motor1Pin2, HIGH); 
  // digitalWrite(motor2Pin1, LOW);
  // digitalWrite(motor2Pin2, HIGH); 
  // return;

  
  printf("En : %s Dir %s\n", en,dir);
    
  if (en == "true") {
    ledcWrite(enable1Pin, dutyCycle);   
    ledcWrite(enable2Pin, dutyCycle); 
    
    if (dir == "w") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH); 
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH); 

    } else if (dir == "s") {
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW); 
      digitalWrite(motor2Pin1, HIGH);
      digitalWrite(motor2Pin2, LOW); 

    } else if (dir == "d") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);

    } else if (dir == "a") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
    }
    delay(unit);
    ledcWrite(enable1Pin, 0);   
    ledcWrite(enable2Pin, 0);
    en = "false";
  } else {
    ledcWrite(enable1Pin, 0);   
    ledcWrite(enable2Pin, 0);
  }
}

void callback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  // Copy payload into a temporary buffer (make sure it’s null-terminated)
  char buffer[length + 1];
  memcpy(buffer, payload, length);
  buffer[length] = '\0';

  StaticJsonDocument<128> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, buffer);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  if (topic == topicMotor) {
    dir = jsonDoc["dir"].as<String>();
    en = jsonDoc["en"].as<String>();
    unit = jsonDoc["unit"].as<int>();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  ledcAttachChannel(enable1Pin, freq, resolution, pwmChannel1);
  ledcAttachChannel(enable2Pin, freq, resolution, pwmChannel2);


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected");
  client.setServer(mqttHost, mqttPort);
  client.setCallback(callback);

}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqttU, mqttP)) {
      Serial.println("connected");

      client.subscribe("things/motor/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }


  }
}

void loop() {
  
  motorLogic();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}