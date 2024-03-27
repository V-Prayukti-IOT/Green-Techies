#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define TRIGGER_PIN 5  // Pin connected to the trigger pin of the ultrasonic sensor
#define ECHO_PIN 4     // Pin connected to the echo pin of the ultrasonic sensor

// Wi-Fi credentials
const char* ssid = "Keerthi";
const char* password = "04072004";

// MQTT broker details
const char* mqttServer = "broker.hivemq.com"; // Change to your MQTT server address
const int mqttPort = 1883;
const char* mqttUser = ""; // Change to your MQTT username
const char* mqttPassword = ""; // Change to your MQTT password

// MQTT topic to publish to
const char* mqttTopic = "bin/bot";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
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
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Publish a message every 5 seconds
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  // Speed of sound is approximately 343 meters per second (depends on temperature and humidity)
  // Divide by 2 because sound travels to the object and back
  float distance = (duration * 0.0343) / 2; // Convert duration to distance in centimeters
  Serial.println(distance);
  delay(3000);
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    client.publish(mqttTopic,"Dustbin Fill!!!");
  }
   
}
