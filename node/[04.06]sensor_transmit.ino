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

// initialize LED pins
int redpin = 18;
int greenpin = 16;
int bluepin = 17;

// Initialize variables for rolling window
const int maxSize = 5;  // max size that we want to keep in rolling window
float h_1 = 0.0;        // humidity[1]
float h_2 = 0.0;        // humidity[2]
float h_3 = 0.0;        // humidity[3]
float h_4 = 0.0;        // humidity[4]
float h_5 = 0.0;        // humidity[5]
int h_current_size = 0; // current size of h
float t_1 = 0.0;        // similar but for temperature
float t_2 = 0.0;
float t_3 = 0.0;
float t_4 = 0.0;
float t_5 = 0.0;
int t_current_size = 0;
float hic_1 = 0.0; // similar but for heat index
float hic_2 = 0.0;
float hic_3 = 0.0;
float hic_4 = 0.0;
float hic_5 = 0.0;
int hic_current_size = 0;
float moist_1 = 0.0; // similar but for moisture levels
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

// Max and min moisture, range shown below is from testing soil mositure sensor
float min_moisture = 0.0;
float max_moisture = 1023.0;
float whcs = 0.108488; // water holding capacity of soil scaling factor determined via sensor calibration

float max_sensor_temp = 80.0;      // max temp possible by temp sensor
float min_sensor_temp = -40.0;     // min temp possible by temp sensor
float max_sensor_humidity = 100.0; // max humidity possible by humidity sensor
float min_sensor_humidity = 0.0;   // min humidity possible by humidity sensor
float min_sensor_moist = 0.0;      // min moisture possible by soil sensor
float max_sensor_moist = 1023.0;   // max moisture possible by soil sensor

String message = ""; // message to be sent to the homepod

void addElement(float element, float &val_1, float &val_2, float &val_3, float &val_4, float &val_5, int &currentSize)
''' Add element to rolling window '''
{
  if (currentSize == maxSize) // If we are at max size, shift all elements to the left and append to last element
  {
    val_1 = val_2;
    val_2 = val_3;
    val_3 = val_4;
    val_4 = val_5;
    val_5 = element;
  }
  else if (currentSize < maxSize) // If we are not at max size, just append to the next element
  {
    currentSize++;
    switch (currentSize)
    {
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

float getAverage(float val_1, float val_2, float val_3, float val_4, float val_5, int currentSize) // Get average of rolling window
{
  float sum = 0;
  switch (currentSize)
  {
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
  Serial.begin(115200);  // Initialize serial communication
  Serial1.begin(115200); // Initialize serial communication
  dht.begin();           // Initialize DHT sensor
  if (!driver.init())    // if drivers are not initialized
  {
    Serial.println("RF driver init failed"); // Sensor check
  }

  // Set pin modes for LED
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
}

bool checkBounds(float value, float min, float max) // Check if value is out of bounds
{
  if (value < min)
  {
    return true;
  }
  if (value > max)
  {
    return true;
  }
  return false;
}

float clamp(float value, float min, float max) // Clamp value to min and max
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

void loop() // Main loop
{
  unsigned long currentMillis = millis(); // Get current time

  if (currentMillis - prevDHTreadtime >= 2000) // if we delete this everything breaks no idea why
  {                                            // DHT sensor reading every 2000 milliseconds
    prevDHTreadtime = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    t = clamp(t, min_temp_, max_temp_);
    h = clamp(h, min_humidity, max_humidity);

    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
  }

  if (Serial1.available()) // if GPS data is available
  {
    GPSData = Serial1.readStringUntil((char)'*');
    // Serial.println(GPSData);
  }

  if (currentMillis - prevsoilmoisturereadtime >= 6000)
  {                                           // Soil moisture sensor reading every 3000 milliseconds
    prevsoilmoisturereadtime = currentMillis; // Get current time

    float h = dht.readHumidity();                         // Read humidity
    float t = dht.readTemperature();                      // Read temperature
    float hic = dht.computeHeatIndex(t, h, false);        // Compute heat index
    if (checkBounds(t, min_sensor_temp, max_sensor_temp)) // Check if temp is out of bounds
    {
      message += "Temperature sensor error";                     // Add error message
      Serial.println(message);                                   // Print error message
      driver.send((uint8_t *)message.c_str(), message.length()); // Send error message
      return;
    }
    if (checkBounds(h, min_sensor_humidity, max_sensor_humidity)) // Check if humidity is out of bounds
    {
      message += "Humidity sensor error";                        // Add error message
      Serial.println(message);                                   // Print error message
      driver.send((uint8_t *)message.c_str(), message.length()); // Send error message
      return;
    }

    t = clamp(t, min_temp_, max_temp_);       // Clamp temp to min and max
    h = clamp(h, min_humidity, max_humidity); // Clamp humidity to min and max

    addElement(h, h_1, h_2, h_3, h_4, h_5, h_current_size);               // Add humidity to rolling window
    addElement(t, t_1, t_2, t_3, t_4, t_5, t_current_size);               // Add temperature to rolling window
    addElement(hic, hic_1, hic_2, hic_3, hic_4, hic_5, hic_current_size); // Add heat index to rolling window

    // Gravity soil sensor reading
    int moist = analogRead(A1); // Read soil moisture sensor

    if (checkBounds(moist, min_sensor_moist, max_sensor_moist)) // Check if moisture is out of bounds
    {
      message += "Moisture sensor error";                        // Add error message
      Serial.println(message);                                   // Print error message
      driver.send((uint8_t *)message.c_str(), message.length()); // Send error message
      return;
    }
    moist = clamp(moist, min_moisture, max_moisture); // Clamp moisture to min and max

    moist = moist * whcs;                                                               // conversion to whcs
    addElement(moist, moist_1, moist_2, moist_3, moist_4, moist_5, moist_current_size); // Add moisture to rolling window

    h = getAverage(h_1, h_2, h_3, h_4, h_5, h_current_size);                             // Get average humidity
    t = getAverage(t_1, t_2, t_3, t_4, t_5, t_current_size);                             // Get average temperature
    hic = getAverage(hic_1, hic_2, hic_3, hic_4, hic_5, hic_current_size);               // Get average heat index
    moist = getAverage(moist_1, moist_2, moist_3, moist_4, moist_5, moist_current_size); // Get average moisture

    // Recommendations
    message = "";   // reset message
    if (moist > 70) // if moisture is greater than 70
    {
      message += "Too wet!;Mulch!/"; // Add message to message string
      setColor(0, 0, 1, 0, 0, 255);  // Blue
      delay(1500);                   // Delay for 1500 milliseconds
    }
    if (moist < 30) // if moisture is less than 30
    {
      message += "Too dry!;Water!/"; // Add message to message string
      setColor(1, 1, 0, 0, 0, 255);  // Red green
      delay(1500);                   // Delay for 1500 milliseconds
    }
    if (t >= 27 && t <= 40) // if temperature is between 27 and 40
    {
      message += "Too hot!;Irrigate!/"; // Add message to message string
      setColor(1, 0, 0, 255, 0, 0);     // Red
      delay(1500);                      // Delay for 1500 milliseconds
    }
    if (t <= 23) // if temperature is less than 23
    {
      message += "Too cold!;Mulch!/"; // Add message to message string
      setColor(0, 1, 1, 0, 0, 255);   // Cyan
      delay(1500);                    // Delay for 1500 milliseconds
    }
    if (h < 99 && h >= 50) // if humidity is between 50 and 99
    {
      message += "Too humid!;Mulch!/"; // Add message to message string
      setColor(1, 1, 1, 0, 0, 255);    // White
      delay(1500);                     // Delay for 1500 milliseconds
    }
    if (h <= 15) // if humidity is less than 15
    {
      message += "Too arid!;Add shade!/"; // Add message to message string
      setColor(1, 0, 1, 0, 0, 255);       // Purple
      delay(1500);                        // Delay for 1500 milliseconds
    }
    if (moist <= 70 and moist >= 30 and t < 27 and t > 23 and h < 50 and h > 20) // if all conditions are met
    {
      message = "OK/";              // Add message to message string
      setColor(0, 0, 0, 0, 0, 255); // Off
      delay(1500);                  // Delay for 1500 milliseconds
    }
    message.remove(message.length() - 1);                                                                                        // Remove last character
    Serial.println(message);                                                                                                     // Print message
    String data = "M: " + String(moist) + " H: " + String(h) + " T: " + String(t) + " Hic: " + String(hic) + " GPS: " + GPSData; // Add data to message
    Serial.println(data);                                                                                                        // Print data
  }
  driver.send((uint8_t *)message.c_str(), message.length()); // Send message
  driver.waitPacketSent();                                   // Wait for packet to be sent
}

void setColor(int redBool, int greenBol, int blueBool, int redValue, int greenValue, int blueValue) // Set color of LED
{
  pinMode(redpin, redBool ? OUTPUT : INPUT);
  pinMode(greenpin, greenBol ? OUTPUT : INPUT);
  pinMode(bluepin, blueBool ? OUTPUT : INPUT);
  analogWrite(redpin, redValue);
  analogWrite(greenpin, greenValue);
  analogWrite(bluepin, blueValue);
}
