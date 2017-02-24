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



#include <rn2xx3.h>
#include <SoftwareSerial.h>
#define SharpPin A0
int distanceIR;
#define trigPin 4
#define echoPin 3
#include "keys.h"

SoftwareSerial mySerial(10, 11); // RX, TX

//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);

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
  Serial.println("Startup");

  initialize_radio();

  //transmit a startup message
  myLora.tx("TTN Mapper on TTN Enschede node");

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
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;

  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP(addr, appskey, mwkskey);

  //OTAA: initOTAA(String AppEUI, String AppKey);
  //join_result = myLora.initOTAA("70B3D57ED00001A6", "A23C96EE13804963F8C2BD6285448198");

  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");

}

// the loop routine runs over and over again forever:
void loop()
{
    led_on();


    distanceIR = get_Sharp_GP2Y0A02YK_Distance(SharpPin);
    
    Serial.println("echo trigger");
    long duration, distanceEcho;
    digitalWrite(trigPin, LOW);  // Added this line
    delayMicroseconds(2); // Added this line
    digitalWrite(trigPin, HIGH);
    //  delayMicroseconds(1000); - Removed this line
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distanceEcho = (duration/2) / 29.1;   
    uint8_t preparedistanceEcho = 0xFF; // out of range
    if (distanceEcho >= 200 || distanceEcho <= 0){
      Serial.println("Out of range");
      Serial.println(distanceEcho);     
    } else {
      Serial.print(distanceEcho);
      Serial.println(" cm");      
      preparedistanceEcho = distanceEcho & 0xFF;
    }

    
    if(distance > 1490) distanceIR = 0xFFFF; // we reserve this value for out of reach

    uint16_t prepareDistanceIR = distanceIR & 0xFFFF; // 2 byte max filter
    
    uint8_t distance1IR = (prepareDistanceIR >> 8) & 0xFF; // first byte
    uint8_t distance2IR = (prepareDistanceIR) & 0xFF; // second byte
    
    uint8_t prepareMessenge[] = {preparedistanceEcho, distance1IR, distance2IR};
    
    myLora.txBytes(prepareMessenge, sizeof(prepareMessenge)); 
    led_off();

    delay(10* 1000);
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

