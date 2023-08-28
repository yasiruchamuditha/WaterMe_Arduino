#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
//@author yasiru
//contact me : https://linktr.ee/yasiruchamuditha for more information.
#define FIREBASE_HOST "waterme-31c24-default-rtdb.firebaseio.com" // Firebase host
#define FIREBASE_AUTH "DP6jqVDt34o2bwXM00J3hb0TB5KOtUIvAI0mZFsG" // Firebase Auth code

#define WIFI_SSID "A03-s"     // Enter your WiFi Name
#define WIFI_PASSWORD "5142774672" // Enter your password

#define DHTPIN D5         // Pin where the DHT11 is connected
#define DHTTYPE DHT11     // DHT 11 sensor type
DHT dht(DHTPIN, DHTTYPE);

int soilMoisturePin = A0;
int soilMoistureValue = 0;
int userStatus = 0;
int percentage = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize DHT sensor
  dht.begin();
  
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(D5, INPUT);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  // Set Firebase credentials
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("/Alert/outputMessage", "System initialized");
}

void loop() {
  // Read DHT11 sensor data
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  } else {
    // Update Firebase with sensor data
    Firebase.setFloat("/DHT11/Temperature", t);
    Firebase.setFloat("/DHT11/Humidity", h);
    
    // Read soil moisture sensor
    soilMoistureValue = analogRead(soilMoisturePin);
    percentage = map(soilMoistureValue, 0, 1023, 0, 100);
    
    // Update Firebase with soil moisture data
    Firebase.setInt("/soilMoistureSensor/soilStatus", soilMoistureValue);
    Firebase.setInt("/soilMoistureSensor/soilPercentage", percentage);
    
    // Get user status from Firebase
    userStatus = Firebase.getInt("/SoilMoisterUserStatus/userStatus");
    bool shutdownStatus = Firebase.getBool("/ShutdownStatus/Switch01");
    
    // Check if the system is not in shutdown mode
    if (shutdownStatus) {
      // Control LEDs based on soil moisture
      if (soilMoistureValue < userStatus) {
        Serial.println("LOW SOIL MOISTURE - RED Led Turned ON");
        Firebase.setString("/Alert/outputMessage", "LOW SOIL MOISTURE - RED Bulb turned on");
        digitalWrite(D2, HIGH);
   
      } else if (soilMoistureValue > userStatus) {
        Serial.println("HIGH SOIL MOISTURE - GREEN Led Turned ON");
        Firebase.setString("/Alert/outputMessage", "HIGH SOIL MOISTURE - GREEN Led Turned ON");
        digitalWrite(D2, LOW);
      } else {
        Serial.println("Command Error! Please send 0/1");
        Firebase.setString("/Alert/outputMessage", "Command Error! Please send 0/1");
      }
    } else {
      Serial.println("System in shutdown mode");
      Firebase.setString("/Alert/PowerStatus/outputMessage", "System in shutdown mode");
      digitalWrite(D2, LOW);
    
    }
  }
  
  delay(500);
}
