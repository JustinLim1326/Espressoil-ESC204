#include "DHT.h"
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define DHTPIN 14     // Digital pin connected to the DHT sensor
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

int redpin = 18;
int greenpin = 16;
int bluepin = 17;

const int maxSize = 5;
float h_1 = 0.0;
float h_2 = 0.0;
float h_3 = 0.0;
float h_4 = 0.0;
float h_5 = 0.0;
int h_current_size = 0;
float t_1 = 0.0;
float t_2 = 0.0;
float t_3 = 0.0;
float t_4 = 0.0;
float t_5 = 0.0;
int t_current_size = 0;
float hic_1 = 0.0;
float hic_2 = 0.0;
float hic_3 = 0.0;
float hic_4 = 0.0;
float hic_5 = 0.0;
int hic_current_size = 0;
float moist_1 = 0.0;
float moist_2 = 0.0;
float moist_3 = 0.0;
float moist_4 = 0.0;
float moist_5 = 0.0;
int moist_current_size = 0;

// Max temperatures in ethiopia can range from 15 to 32 degrees celsius with some tolerance 
float max_temp_ = 40.0;
float min_temp_ = 5.0;

// Max and min humidity in Ethiopia is 20-80% with some tolerance
float max_humidity = 85.0;
float min_humidity = 15.0;

// Max and min moisture, since measrued in %, is 0 and 100
float min_moisture = 0.0;
float max_moisture = 100.0;

void addElement(int element, float &val_1, float &val_2, float &val_3, float &val_4, float &val_5, int &currentSize) {
  if (currentSize == maxSize) {
    val_1 = val_2;
    val_2 = val_3;
    val_3 = val_4;
    val_4 = val_5;
    val_5 = element;
  }
  else if (currentSize < maxSize) {
    currentSize++;
    switch (currentSize) {
    case 1:
      val_1 = element;
      break;
    case 2:
      val_2 = element;
      break;
    case 3:
      val_3 = element;
      break;
    case 4:
      val_4 = element;
      break;
    case 5:
      val_5 = element;
      break;
    }
  }
}
float getAverage(float val_1, float val_2, float val_3, float val_4, float val_5, int currentSize) {
  float sum = 0;
  switch (currentSize) {
  case 1:
    sum = val_1;
    break;
  case 2:
    sum = val_1 + val_2;
    break;
  case 3:
    sum = val_1 + val_2 + val_3;
    break;
  case 4:
    sum = val_1 + val_2 + val_3 + val_4;
    break;
  case 5:
    sum = val_1 + val_2 + val_3 + val_4 + val_5;
    break;
  }
  return sum / currentSize;
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  dht.begin();
  if (!driver.init())
  {
    Serial.println("RF driver init failed"); // Sensor check
  }
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
}

float clamp(float value, float min, float max)
{
  if (value < min)
  {
    return min;
  }
  if (value > max)
  {
    return max;
  }
  return value;
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - prevDHTreadtime >= 2000)
  { // DHT sensor reading every 2000 milliseconds
    prevDHTreadtime = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    t = clamp(t, min_temp_, max_temp_);
    h = clamp(h, min_humidity, max_humidity);

    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      // Prepare the message to be sent to receiver
      String message;
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
    }
  }

  if (Serial1.available())
  {
    GPSData = Serial1.readStringUntil((char)'*');
    // Serial.println(GPSData);
  }

  if (currentMillis - prevsoilmoisturereadtime >= 5000)
  { // Soil moisture sensor reading every 3000 milliseconds
    prevsoilmoisturereadtime = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float hic = dht.computeHeatIndex(t, h, false);

    t = clamp(t, min_temp_, max_temp_);
    h = clamp(h, min_humidity, max_humidity);

    addElement(h, h_1, h_2, h_3, h_4, h_5, h_current_size);
    addElement(t, t_1, t_2, t_3, t_4, t_5, t_current_size);
    addElement(hic, hic_1, hic_2, hic_3, hic_4, hic_5, hic_current_size);

    // Gravity soil sensor reading
    int moist = analogRead(A1);
    moist = clamp(moist, min_moisture, max_moisture);

    addElement(moist, moist_1, moist_2, moist_3, moist_4, moist_5, moist_current_size);

    h = getAverage(h_1, h_2, h_3, h_4, h_5, h_current_size);
    t = getAverage(t_1, t_2, t_3, t_4, t_5, t_current_size);
    hic = getAverage(hic_1, hic_2, hic_3, hic_4, hic_5, hic_current_size);
    moist = getAverage(moist_1, moist_2, moist_3, moist_4, moist_5, moist_current_size);

    String message = "";
    //error messages
    Serial.print(moist);
    Serial.print(t);
    if( moist >= 900){
    message += "Moisture sensor error";
    Serial.println(message);
    driver.send((uint8_t *)message.c_str(), message.length());
    delay(500);
    }
    if( t > 80){
    message += "Temperature sensor error";
    Serial.println(message);
    driver.send((uint8_t *)message.c_str(), message.length());
    delay(500);
  }
   if( h > 98){
    message += "Humidity sensor error";
    Serial.println(message);
    driver.send((uint8_t *)message.c_str(), message.length());
    delay(500);
  }

  //Recommendations
    if (moist >= 500)
    {
      message += "Too wet!;Mulch!/";
      setColor(0, 0, 1, 0, 0, 255); // Blue
      delay(500);
    }
    if (moist <= 100)
    {
      message += "Too dry!;Water!/";
      setColor(1, 1, 0, 0, 0, 255); // Red green
      delay(500);
    }
    if (t >= 27 && t <= 40)
    {
      message += "Too hot!;Irrigate!/";
      setColor(1, 0, 0, 255, 0, 0); // Red
      delay(500);
    }
    if (t <= 14)
    {
      message += "Too cold!;!/";
      setColor(0, 1, 1, 0, 0, 255); // Cyan
      delay(500);
    }
    if (h< 99 && h >= 50)
    {
      message += "Too humid!;Mulch!/";
      setColor(1, 1, 1, 0, 0, 255); // Purple
      Serial.println(message);
      delay(500);
    }
    if (h <= 30)
    {
      message += "Too arid!;Add shade and water!/";
      setColor(1, 0, 1, 0, 0, 255); // Purple
      delay(500);
    }
    if (moist < 500 and moist > 100 and t < 27 and t > 14 and h < 50 and h > 30)
    {
      message = "OK";
      setColor(0, 0, 0, 0, 0, 255); // Off
      delay(500);
    }
    Serial.println(message);
    driver.send((uint8_t *)message.c_str(), message.length());
    driver.waitPacketSent();
  }
    

}

void setColor(int redBool, int greenBol, int blueBool, int redValue, int greenValue, int blueValue)
{
  pinMode(redpin, redBool ? OUTPUT : INPUT);
  pinMode(greenpin, greenBol ? OUTPUT : INPUT);
  pinMode(bluepin, blueBool ? OUTPUT : INPUT);
  analogWrite(redpin, redValue);
  analogWrite(greenpin, greenValue);
  analogWrite(bluepin, blueValue);
}