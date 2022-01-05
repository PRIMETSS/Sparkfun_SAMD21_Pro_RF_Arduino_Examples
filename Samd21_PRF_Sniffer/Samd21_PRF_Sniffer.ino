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
RH_RF95 rf95(12, 6); // D12 = RFM_CS, D6 = RFM_INT

int LED = 13; //Status LED is on pin 13

long packetCounter = 0; //Counts the number of packets sent
long loopCounter = 0; //Counts the number of complete Freq loops performed
long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

// The broadcast frequency is set to 921.2, but the SADM21 ProRf operates
// anywhere in the range of 902-928MHz in the Americas.
// Europe operates in the frequencies 863-870, center frequency at 868MHz.
// This works but it is unknown how well the radio configures to this frequency:

// Packet Decoder https://lorawan-packet-decoder-0ta6puiniaut.runkit.sh/?data=40c1190126800100024926272bf18bbb6341584e27e23245&nwkskey=00000000000000000000000000000111&appskey=00000000000000000000000000000111

// https://lora-alliance.org/wp-content/uploads/2020/11/rp_2-1.0.1.pdf RFreq and Channels page 44 (AUS)
// Upstream – 64 channels numbered 0 to 63 utilizing LoRa 125 kHz BW varying from
// DR0 to DR5, using coding rate 4/5, starting at 915.2 MHz and incrementing linearly
// by 200 kHz to 927.8 MHz
// • Upstream – 8 channels numbered 64 to 71 utilizing LoRa 500 kHz BW at DR6
// starting at 915.9 MHz and incrementing linearly by 1.6 MHz to 927.1 MHz
// • Downstream – 8 channels numbered 0 to 7 utilizing LoRa 500 kHz BW at DR8 to
// DR13) starting at 923.3 MHz and incrementing linearly by 600 kHz to 927.5 MHz


double minFrequency = 915.2; // Start frequency Mhz   // 64 Chn 902.3 / 8 Chn 903.0
double maxFrequency = 927.8; // End frequency Mhz     // 64 Chn 914.9 / 8 Chn 914.2
double currentFrequency = minFrequency;  // Frequency to step
double stepFrequency = 0.2;  // Frequency to step     // 64 Chn 0.2   / 8 Chn 1.6
RH_RF95::ModemConfigChoice modemCfg =  RH_RF95::Bw125Cr45Sf128; // RH_RF95::Bw125Cr45Sf128; Upstream 64 Chn // or Bw500Cr45Sf128 Upstream 8 Chan

void setup()
{
  pinMode(LED, OUTPUT);

  SerialUSB.begin(9600);
  // It may be difficult to read serial messages on startup. The following line
  // will wait for serial to be ready before continuing. Comment out if not needed.
  while(!SerialUSB); // Wait for terminal to connect
  SerialUSB.println("RFM Sniffer!"); 

  //Initialize the Radio.
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);
  if (rf95.init() == false){
    SerialUSB.println("Radio Init Failed - Freezing");    
    while (1);
  }
  else{
    //An LED inidicator to let us know radio initialization has completed. 
    SerialUSB.println("Listner up!"); 
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }

  // Set frequency
  rf95.setFrequency(minFrequency);

   // The default transmitter power is 13dBm, using PA_BOOST.
   // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
   // you can set transmitter powers from 5 to 23 dBm:
   // Transmitter power can range from 14-20dbm.
   rf95.setTxPower(14, false);   
}


void loop()
{
  // Set frequency
  rf95.setFrequency(currentFrequency);

  SerialUSB.print("[Listening to [ ");
  SerialUSB.print(currentFrequency, DEC);
  SerialUSB.print(" ] Total Packets Rec [ ");
  SerialUSB.print(packetCounter);
  SerialUSB.print(" ] Total Loops [ ");
  SerialUSB.print(loopCounter);  
  SerialUSB.print(" ]\r");
   
  // Now wait for any broadcast @ this Freq
  byte buf[RH_RF95_MAX_MESSAGE_LEN];
  byte len = sizeof(buf);
  
  if (rf95.waitAvailableTimeout(500)) {    
    if (rf95.recv(buf, &len)) {
      SerialUSB.print("Got reply: ");
      //SerialUSB.println((char*)buf);
      for (int x = 0; x < sizeof(long); x++)
      {
        SerialUSB.print(buf[x], HEX);
      }
      SerialUSB.println();
      SerialUSB.print("RxGood: ");
      SerialUSB.print(rf95.rxGood());
      SerialUSB.print(" RxBad: ");
      SerialUSB.println(rf95.rxBad());

      SerialUSB.print("RSSI: ");
      SerialUSB.print(-137 + rf95.lastRssi(), HEX);
      SerialUSB.println(" dBm");
      
       packetCounter++;      
    }
    else {
  //    SerialUSB.println("Receive failed");
    }
  }
  else {
    // Shift freq to new 
    if (currentFrequency <= maxFrequency) {
      currentFrequency += stepFrequency;
    } else {
      currentFrequency = minFrequency;
      loopCounter++;
    }    
  }  
}
