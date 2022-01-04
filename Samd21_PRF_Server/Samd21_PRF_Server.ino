/*
  Both the TX and RX ProRF boards will need a wire antenna. We recommend a 3" piece of wire.
  This example is a modified version of the example provided by the Radio Head
  Library which can be found here: 
  www.github.com/PaulStoffregen/RadioHeadd
*/

#include <SPI.h>

//Radio Head Library: 
#include <RH_RF95.h>

// We need to provide the RFM95 module's chip select and interrupt pins to the 
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

int LED = 13; //Status LED on pin 13

int packetCounter = 0; //Counts the number of packets sent
long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received
// The broadcast frequency is set to 921.2, but the SADM21 ProRf operates
// anywhere in the range of 902-928MHz in the Americas.
// Europe operates in the frequencies 863-870, center frequency at 
// 868MHz.This works but it is unknown how well the radio configures to this frequency:
//float frequency = 864.1;
float frequency = 915.0;

void setup()
{
  pinMode(LED, OUTPUT);

  SerialUSB.begin(9600);
  // It may be difficult to read serial messages on startup. The following
  // line will wait for serial to be ready before continuing. Comment out if not needed.
//  while(!SerialUSB);
  SerialUSB.println("RFM Server!");

  //Initialize the Radio. 
  if (rf95.init() == false){
    SerialUSB.println("Radio Init Failed - Freezing");
    while (1);
  }
  else{
  // An LED indicator to let us know radio initialization has completed.
    SerialUSB.println("Receiver up!");
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }

  rf95.setFrequency(frequency); 

   // The default transmitter power is 13dBm, using PA_BOOST.
   // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
   // you can set transmitter powers from 5 to 23 dBm:
   // Transmitter power can range from 14-20dbm.
   rf95.setTxPower(23, false);
}

void loop()
{
  if (rf95.available()){
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)){
      digitalWrite(LED, HIGH); //Turn on status LED
      timeSinceLastPacket = millis(); //Timestamp this packet

      SerialUSB.println("[Received client message]");
      SerialUSB.print("Received: ");
      //SerialUSB.println(buf, HEX);
      for (int x = 0; x < sizeof(long); x++)
      {
        SerialUSB.print(buf[x], HEX);
      }
      SerialUSB.println();

//      delay(500); // Pause for repeater

      // Send a reply
      unsigned long timeStamp = millis();
      uint8_t toSend[] = {  (timeStamp >> 24), (timeStamp >> 16) & 0xff, (timeStamp >> 8) & 0xff, (timeStamp) & 0xff  }; //"Hello Back!";
      rf95.send(toSend, sizeof(toSend));
      rf95.waitPacketSent(); // **** Blocks if LoraWAN jumpers set if no time out passed!
      SerialUSB.print("Sent: ");
      SerialUSB.println(timeStamp, HEX);

       digitalWrite(LED, LOW); //Turn off status LED
      
      SerialUSB.print("RxGood: ");
      SerialUSB.print(rf95.rxGood());
      SerialUSB.print(" RxBad: ");
      SerialUSB.println(rf95.rxBad());

      SerialUSB.print("RSSI: ");
      SerialUSB.print(-137 + rf95.lastRssi(), DEC);
      SerialUSB.println(" dBm");

      SerialUSB.print("HeaderFrom: ");
      SerialUSB.println(rf95.headerFrom());
      SerialUSB.print("HeaderTo: ");
      SerialUSB.println(rf95.headerTo());
      SerialUSB.print("HeaderID: ");
      SerialUSB.println(rf95.headerId());

      packetCounter++;

      SerialUSB.println();
    }
    else
      SerialUSB.println("Recieve failed");
  }
  
  //Turn off status LED if we haven't received a packet after 1s
  if(millis() - timeSinceLastPacket > 1000){
    digitalWrite(LED, LOW); //Turn off status LED
    timeSinceLastPacket = millis(); //Don't write LED but every 1s    
  }     
}
