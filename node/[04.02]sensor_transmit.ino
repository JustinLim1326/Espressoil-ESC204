#include "DHT.h"
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define DHTPIN 14 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
// Initialize DHT sensor.
#define RxPin 1
#define TxPin 0

RH_ASK driver(2000, 4, 2, 5);
String GPSData = "No GPS";

DHT dht(DHTPIN, DHTTYPE);
unsigned long prevDHTreadtime = 0;
unsigned long prevsoilmoisturereadtime = 0;
// mySerial = AltSoftSerial(RxPin, TxPin);

int redpin = 17;
int greenpin = 18;
int bluepin = 19;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    dht.begin();
    if (!driver.init())
    {
        Serial.println("RF driver init failed"); //Sensor check
    }
  
    pinMode(bluepin, OUTPUT);
    pinMode(redpin, OUTPUT);
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - prevDHTreadtime >= 2000)
    { // DHT sensor reading every 2000 milliseconds
        prevDHTreadtime = currentMillis;
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float f = dht.readTemperature(true);
        if (isnan(h) || isnan(t) || isnan(f))
        {
            Serial.println("Failed to read from DHT sensor!");
        }else{
            // Prepare the message to be sent to receiver
            String message;
            driver.send((uint8_t *)message.c_str(), message.length());
            driver.waitPacketSent();
        }
    }

    if (Serial1.available()) {
        GPSData = Serial1.readStringUntil((char)'*');
        //Serial.println(GPSData);
    }

    if (currentMillis - prevsoilmoisturereadtime >= 3000)
    { // Soil moisture sensor reading every 3000 milliseconds
        prevsoilmoisturereadtime = currentMillis;
        
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float f = dht.readTemperature(true);
        float hic = dht.computeHeatIndex(t, h, false);

        Serial.print("Humidity: "); Serial.print(h);
        Serial.print("%  Temperature: "); Serial.print(t);
        Serial.print("C "); Serial.print(f);
        Serial.print("F  Heat index: "); Serial.print(hic); Serial.print("C ");

    //Gravity soil sensor reading
        int moist = analogRead(A1);
        Serial.print("Moisture Sensor Value: ");
        Serial.println(moist);

      //LED 
        if(moist > 500){
          //analogWrite(bluepin, HIGH);
          setColor(0, 0, 255);
        }else if(moist < 5){
          //analogWrite(bluepin, HIGH);
          setColor(0, 0, 255);
          delay(1000);
          //analogWrite(bluepin, LOW);
          setColor(0, 0, 0);
        }else{
          //analogWrite(bluepin, LOW);
          setColor(0, 0, 0);
        }
        if( t > float(30.0)){
          //analogWrite(redpin, HIGH);
          setColor(255, 0, 0);
        }else if(t < float(26.0)){
          //analogWrite(redpin, HIGH);
          setColor(255, 0, 0);
          delay(1000);
          //analogWrite(redpin, LOW);
          setColor(0, 0, 0);
        }else{
          //analogWrite(redpin, LOW);
          setColor(0, 0, 0);
        }
        // Append moisture value to the message and send to receiver
        String message = "M: " + String(moist) + " H: " + String(h) + " T: " + String(t) + "; " + String(f) + " Hic: " + String(hic) + " GPS: " + GPSData;
        Serial.println(message);
        driver.send((uint8_t *)message.c_str(), message.length());
        driver.waitPacketSent();
    }
}


void setColor(int redValue, int greenValue,  int blueValue) {
  analogWrite(redpin, 255-redValue);
  analogWrite(greenpin,  255-greenValue);
  analogWrite(bluepin, 255-blueValue);
  }