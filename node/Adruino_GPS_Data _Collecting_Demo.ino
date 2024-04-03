/* Simple Receiver Code
* (TX out of Arduino is Digital Pin 1)
* (RX into Arduino is Digital Pin 0)
*/

void setup(){
  //2400 baud for the 434 model
  Serial.begin(115200);
  Serial1.begin(115200);
  //Serial1.setTimeout(1);
}
void loop(){
  // read in values, debug to computer
  //Serial.print(Serial1.available());
  // String message1 = Serial1.readString();
  // Serial.print("Received3: ");
  // Serial.print(message1);
  // Serial.println(Serial1.available());
  // Serial.print("\n");

  if (Serial1.available()) {
    String datamessage = Serial1.readStringUntil((char)'*');
    //Serial.print("Received: ");
    Serial.println(datamessage);
  }
  //Serial.println("Ran");
  // if (Serial.available()) {
  //   String message = Serial.readString();
  //   Serial.print("Received1: ");
  //   Serial.println(message);
  // }
  //delay(1000);
  //Serial.println(incomingByte, DEC);
}
