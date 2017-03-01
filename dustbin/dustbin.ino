/*
 * Author: Dennis Ruigrok
 * Date: 2017-02-18
 * 
 *
 * This program is meant to be used with an Arduino UNO or NANO, conencted to an RNxx3 radio module.
 * It will most likely also work on other compatible Arduino or Arduino compatible boards, like The Things Uno, but might need some slight modifications.
 *
 * Connect the RN2xx3 as follows:
 * RN2xx3 -- Arduino
 * Uart TX -- 10
 * Uart RX -- 11
 * Reset -- 12
 * Vcc -- 3.3V
 * Gnd -- Gnd
 *
 * It uses the GP2Y0A02YK IR distance sensor and transmits the distance to TTN
 *
 */


#define numberOfSamples 9
#include <RunningMedian.h>
#include <rn2xx3.h>
#include <SoftwareSerial.h>
#define SharpPin A0
int distanceIR;
long distanceEcho;
#define trigPin 4
#define echoPin 3
#include "keys.h"
#define betweenMeasurements 1000
#define waitBetweenLora ((10*1000) - (betweenMeasurements * numberOfSamples))

SoftwareSerial mySerial(10, 11); // RX, TX

//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);


RunningMedian samplesIR = RunningMedian(numberOfSamples);
RunningMedian samplesEcho = RunningMedian(numberOfSamples);

// the setup routine runs once when you press reset:
void setup()
{
  //output LED pin
  pinMode(13, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  led_on();

  // Open serial communications and wait for port to open:
  Serial.begin(57600); //serial port to computer
  mySerial.begin(9600); //serial port to radio
  Serial.println(F("Startup"));

  initialize_radio();

  led_off();
  delay(2000);
}

void initialize_radio()
{
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println(F("Communication with RN2xx3 unsuccessful. Power cycle the board."));
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println(F("When using OTAA, register this DevEUI: "));
  Serial.println(myLora.hweui());
  Serial.println(F("RN2xx3 firmware version:"));
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println(F("Trying to join TTN"));
  bool join_result = false;

  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP(addr, appskey, mwkskey);

  //OTAA: initOTAA(String AppEUI, String AppKey);
  //join_result = myLora.initOTAA("70B3D57ED00001A6", "A23C96EE13804963F8C2BD6285448198");

  while(!join_result)
  {
    Serial.println(F("Unable to join. Are your keys correct, and do you have TTN coverage?"));
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println(F("Successfully joined TTN"));

}

// the loop routine runs over and over again forever:
void loop()
{
    led_on();

    for(int i = 0; i < numberOfSamples; i++) {
      distanceIR = get_Sharp_GP2Y0A02YK_Distance(SharpPin);
      samplesIR.add(distanceIR); // add to buffer
      
      Serial.println(F("echo trigger"));
      long duration;      
      digitalWrite(trigPin, LOW);  
      delayMicroseconds(2); 
      digitalWrite(trigPin, HIGH);      
      delayMicroseconds(10); 
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distanceEcho = (duration/2) / 29.1;   
      samplesEcho.add(distanceEcho); // add to buffer

      // display
      Serial.print(F("Echo: "));     
      Serial.print(distanceEcho*10);     
      Serial.println(F(" mm"));
      Serial.print(F("IR: "));     
      Serial.print(distanceIR);     
      Serial.println(F(" mm"));

      // delay
      delay(betweenMeasurements);
    }
    
    led_off();

    sendLora();    
}

void sendLora() {
    led_on();     

    

         
    // pick one Echo
    // taking the average
    int distanceEchoFinal = samplesEcho.getAverage();

    uint8_t preparedistanceEcho = 0xFF; // out of range by default      
    if (distanceEchoFinal >= 200 || distanceEchoFinal <= 0){
      //Serial.println("Out of range");
      // if the average is out of range then almost all values where out of range!

      // take lowest
      distanceEchoFinal = samplesEcho.getLowest();
      // again out of range??
    }
    if (!(distanceEchoFinal >= 200 || distanceEchoFinal <= 0)) {
      preparedistanceEcho = distanceEchoFinal & 0xFF;
    }

    // pick one IR
    // Taking the median
    long distanceIRFinal = samplesIR.getMedian();
    
    
    if(distanceIRFinal > 1490) {
      // maybe the lowest value?
      distanceIRFinal = samplesIR.getLowest();      
      // again out of range??
    }
    if(distanceIRFinal > 1490) {
      distanceIRFinal = 0xFFFF; // we reserve this value for out of reach
    }
    Serial.print(F("Choose for echo: "));
    Serial.println(distanceEchoFinal * 10);
    Serial.print(F("Choose for IR: "));
    Serial.println(distanceIRFinal);
  
    uint16_t prepareDistanceIR = distanceIRFinal & 0xFFFF; // 2 byte max filter
    
    uint8_t distance1IR = (prepareDistanceIR >> 8) & 0xFF; // first byte
    uint8_t distance2IR = (prepareDistanceIR) & 0xFF; // second byte
    
    uint8_t prepareMessenge[] = {preparedistanceEcho, distance1IR, distance2IR};
    Serial.println(F("Lora"));
    myLora.txBytes(prepareMessenge, sizeof(prepareMessenge)); 
    led_off();

    delay(waitBetweenLora);
    samplesIR.clear();
    samplesEcho.clear();
}


/** Sharp distance sensor read function - uses analog port
return distance in mm
**/
float get_Sharp_GP2Y0A02YK_Distance(int PinID)
{
  // Read analog to digital converter value
  float ADCValue = (float)analogRead(PinID);

  // Convert in millimeters and return distance
  return  2583.711122992086
          - 20.197897855471 * ADCValue
          + 0.071746539329 * ADCValue * ADCValue
          - 0.000115854182 * ADCValue * ADCValue * ADCValue
          + 0.000000068590 * ADCValue * ADCValue * ADCValue * ADCValue;
}

void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}

