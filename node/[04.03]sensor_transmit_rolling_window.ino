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
float h_rolling_window_array[maxSize];
int h_current_size = 0;
float t_rolling_window_array[maxSize];
int t_current_size = 0;
float f_rolling_window_array[maxSize];
int f_current_size = 0;
float hic_rolling_window_array[maxSize];
int hic_current_size = 0;
float moist_rolling_window_array[maxSize];
int moist_current_size = 0;

void addElement(int element, float array[], int &currentSize) {
  if (currentSize == maxSize) {
    for (int i = 0; i < maxSize - 1; i++) {
      array[i] = array[i + 1]; 
    }
    array[maxSize - 1] = element;
  }
  else {
    array[currentSize - 1] = element;
  }
}

float getAverage(float array[], int currentSize) {
  float sum = 0;
  for (int i = 0; i < currentSize; i++) {
    sum += array[i];
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

  if (currentMillis - prevsoilmoisturereadtime >= 2000)
  { // Soil moisture sensor reading every 3000 milliseconds
    prevsoilmoisturereadtime = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    float hic = dht.computeHeatIndex(t, h, false);

    addElement(h, h_rolling_window_array, h_current_size);
    addElement(t, t_rolling_window_array, t_current_size);
    addElement(f, f_rolling_window_array, f_current_size);
    addElement(hic, hic_rolling_window_array, hic_current_size);

    // Gravity soil sensor reading
    int moist = analogRead(A1);
    addElement(moist, moist_rolling_window_array, moist_current_size);

    h = getAverage(h_rolling_window_array, h_current_size);
    t = getAverage(t_rolling_window_array, t_current_size);
    f = getAverage(f_rolling_window_array, f_current_size);
    hic = getAverage(hic_rolling_window_array, hic_current_size);
    moist = getAverage(moist_rolling_window_array, moist_current_size);

    String message;

    if (moist > 50)
    {
      message = "Too wet!;Mulch mulch mulch!";
      setColor(0, 0, 1, 0, 0, 255); // Blue
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }
    if (moist < 10)
    {
      message = "Too dry!;Water water water!";
      setColor(1, 1, 0, 0, 0, 255); // Red green
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }
    if (t > 24.5)
    {
      message = "Too hot!;Irrigate!";
      setColor(1, 0, 0, 255, 0, 0); // Red
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }
    if (t < 23)
    {
      message = "Too cold!;gg";
      setColor(0, 1, 1, 0, 0, 255); // Cyan
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }
    if (h > 50)
    {
      message = "Too humid!;Mulch!!";
      setColor(1, 1, 1, 0, 0, 255); // Purple
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }
    if (h < 25)
    {
      message = "Too un-humid!;Add shade and water!";
      setColor(1, 0, 1, 0, 0, 255); // Purple
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }
    if (moist < 50 and moist > 10 and t < 24 and t > 23 and h < 50 and h > 25)
    {
      message = "OK";
      setColor(0, 0, 0, 0, 0, 255); // Off
      Serial.println(message);
      driver.send((uint8_t *)message.c_str(), message.length());
      driver.waitPacketSent();
      delay(1500);
    }

    message = "M: " + String(moist) + " H: " + String(h) + " T: " + String(t) + "; " + String(f) + " Hic: " + String(hic) + " GPS: " + GPSData;
    Serial.println(message);
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

// (255, 0, 0) -> (255, 255, 255,)
// (0, 255, 0,) - > (255, 255, 0)
// (0, 0, 255) -> (0, 0, 255)
// (255, 255, 0) -> (255, 255, 0)
// (0, 255, 255) -> (0, 0, 0)
// (255, 0, 255) -> (0, 0, 255)
