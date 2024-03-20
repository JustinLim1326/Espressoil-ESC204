#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver(2000, 2, 4, 5);

void setup(){
    Serial.begin(115200);  // Initialize Serial Monitor
    if (!driver.init()){
         Serial.println("init failed");
    }
}

void loop(){
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      // int i;
      
      // Message with a good checksum received, dump it.
      String rcv;
      for (int i = 0; i < buflen; i++){
        rcv += (char)buf[i];
      }
      Serial.print("Message: ");
      Serial.println((char)buf);
      Serial.println(rcv);
    }
}
