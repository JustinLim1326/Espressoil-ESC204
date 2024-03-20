#include "DHT.h"
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define DHTPIN 14     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
#define RxPin 1
#define TxPin 0

RH_ASK driver(2000, 4, 16, 5);

DHT dht(DHTPIN, DHTTYPE);
unsigned long prevDHTreadtime = 0;
unsigned long prevsoilmoisturereadtime = 0;
//mySerial = AltSoftSerial(RxPin, TxPin);

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  dht.begin();
  if (!driver.init()){
    Serial.println("init failed");
  }
}

void loop() {
  if (millis() - prevDHTreadtime >= 2000){
    prevDHTreadtime = millis();
  // Wait a 2 seconds between measurements.
  float h = dht.readHumidity();
  // Read temperature as Celsius (default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit 
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit 
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius 
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: ")); Serial.print(h);
  Serial.print(F("%  Temperature: ")); Serial.print(t);
  Serial.print(F("째C ")); Serial.print(f);
  Serial.print(F("째F  Heat index: ")); Serial.print(hic);
  Serial.print(F("째C ")); Serial.print(hif);
  }
  //Gravity soil humidity sensor
  if (millis() - prevsoilmoisturereadtime >= 1000){
    prevsoilmoisturereadtime = millis();
    Serial.print("Moisture Sensor Value:");
    Serial.println(analogRead(A1));
    delay(2000);
  }
//Transmitter
    const char *msg = "far";
    const char *msg1 = "working test";
    const char *msg2 = ":)";
    driver.send((uint8_t*)msg, strlen(msg));
    driver.send((uint8_t*)msg1, strlen(msg1));
    driver.send((uint8_t*)msg2, strlen(msg2));
    driver.waitPacketSent();
    delay(1000); // Wait for a second before sending the message again
  }
