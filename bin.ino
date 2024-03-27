#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>

#define GSM_PIN "<YOUR_GSM_PIN>"
#define APN "<YOUR_APN>"
#define MQTT_BROKER "mqtt.server.com"
#define MQTT_PORT 1883
#define MQTT_CLIENTID "dustbin"
#define MQTT_TOPIC "waste_collection"

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial gsmSerial(4, 3); // Using pin numbers instead of D4 and D3
TinyGsm modem(gsmSerial);
TinyGsmClient gsmClient(modem);
PubSubClient mqttClient(gsmClient);

const int trigPin = 6; // Using pin numbers instead of D6
const int echoPin = 5; // Using pin numbers instead of D5
const int threshold = 75;

String phoneNumber = "+9188305848xx";
String mapLink = "https://maps.google.com/maps?q=<latitude>,<longitude>"; // Replace <latitude> and <longitude> with actual coordinates

bool wasteCollectionRequested = false;

TinyGPSPlus gps;

void setup() {
  Serial.begin(9600); // Start serial communication
  gsmSerial.begin(9600); // Start serial communication with GSM module
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Connect to GSM network
  modem.restart();
  modem.simUnlock(GSM_PIN);
  modem.gprsConnect(APN);

  // Connect to MQTT broker
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  lcd.clear();
  lcd.print("Smart Dustbin");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
}

void loop() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;

  int fillLevel = map(distance, 0, 100, 100, 0);

  lcd.clear();
  lcd.print("Fill Level: ");
  lcd.print(fillLevel);
  lcd.print("%");

  if (fillLevel >= threshold && !wasteCollectionRequested) {
    sendSignalToRobot();
  }

  // Check GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
        mapLink = "https://maps.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
      }
    }
  }

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
}

void sendSignalToRobot() {
  wasteCollectionRequested = true; // Set flag to indicate waste collection requested
  if (mqttClient.publish(MQTT_TOPIC, mapLink.c_str())) {
    Serial.println("Signal sent to robot");
    lcd.clear();
    lcd.print("Signal Sent!");
    delay(2000);
  } else {
    Serial.println("Failed to send signal");
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENTID)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}
