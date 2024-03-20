#include "DHT.h"
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define DHTPIN 14     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
#define RxPin 1
#define TxPin 0

RH_ASK driver(2000, 4, 2, 5);

DHT dht(DHTPIN, DHTTYPE);
unsigned long prevDHTreadtime = 0;
unsigned long prevsoilmoisturereadtime = 0;
//mySerial = AltSoftSerial(RxPin, TxPin);

void setup() {
    Serial.begin(115200);
    dht.begin();
    if (!driver.init()) {
        Serial.println("RF driver init failed");
    }
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - prevDHTreadtime >= 2000) { // DHT sensor reading every 2000 milliseconds
        prevDHTreadtime = currentMillis;

        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float f = dht.readTemperature(true);

        if (isnan(h) || isnan(t) || isnan(f)) {
            Serial.println("Failed to read from DHT sensor!");
        } else {
            float hif = dht.computeHeatIndex(f, h);
            float hic = dht.computeHeatIndex(t, h, false);

            Serial.print("Humidity: "); Serial.print(h);
            Serial.print("%  Temperature: "); Serial.print(t); Serial.print("째C ");
            Serial.print(f); Serial.print("째F  Heat index: "); Serial.print(hic); Serial.print("째C ");
            Serial.print(hif); Serial.println("째F");

            // Prepare the message to be sent
            String message = String(h) + "," + String(t) + "," + String(f) + "," + String(hic) + ",";
            driver.send((uint8_t *)message.c_str(), message.length());
            driver.waitPacketSent();
        }
    }

    if (currentMillis - prevsoilmoisturereadtime >= 1000) { // Soil moisture sensor reading every 1000 milliseconds
        prevsoilmoisturereadtime = currentMillis;
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float f = dht.readTemperature(true);
        float hic = dht.computeHeatIndex(t, h, false);

        int moist = analogRead(A1);
        Serial.print("Moisture Sensor Value: ");
        Serial.println(moist);

        // Append moisture value to the message and send
        String message = "Moisture: " + String(moist) + " Humidity: " + String(h) + " Temperature: " + String(t) + ", " + String(f) + " Heat Index: " + String(hic);
        driver.send((uint8_t *)message.c_str(), message.length());
        driver.waitPacketSent();
    }
}