#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver(2000, 2, 4, 5);

void setup()
{
    Serial.begin(115200);  // Initialize Serial Monitor
    Serial1.begin(115200);
    if (!driver.init())
         Serial.println("init failed");
}
void loop()
{
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      // Message with a good checksum received, dump it.
      String rcv;
      for (int i = 0; i < buflen; i++){
        rcv += (char)buf[i];
      }
      //error handling empty string
      if(rcv != ""){
        Serial.print("Message: ");
        Serial1.println(rcv);
        Serial.println(rcv);
      }
    }
}