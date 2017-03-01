/*
 * Author: Dennis Ruigrok
 * Date: 2017-02-18
 * 
 * This program is made for a dustbin 150cm max (IR). The arduino takes a number of measurements (numberOfSamples) from a number of sensors.
 * These sensors are:
 * 
 * - GP2Y0A02YK IR distance sensor with a range of 20cm min and 150cm max
 * The IR sensor take measurements in mm
 * - HCSR04 Ultrasonic echo sensor. 
 * Takes measurements from 2cm to 4m - But as we tried this in a dustbin it takes very inaccurate measurements. Still be left it in.
 *
 * The program puts the measurements in a buffer called the RunningMedian. This library then provides support to take the median or average.
 * We take the Average of the Echo sensor and the Median of the IR sensor.
 * 
 * The program then send the data over Lora (https://en.wikipedia.org/wiki/LPWAN) to The Things Network. The login keys are in keys.h which you can create by renaming keys.h.example
 * The measurements are first checked for out of reach, if so, the lowest value of the measurements is tried. I still out of reach the max value of 1 byte or 2 bytes (0xFF or 0xFFFF) is sent.
 * - Echo sensor is in cm so we can use a number between 0 and 255, which fits in 1 byte
 * - Ultrasonic sensor is in mm so we can use two bytes which gives us 0 to 255 * 255mm which is more than enough.
 * 
 * Connect the hardware:
 *  *
 * Connect the RN2xx3 to the arduino as follows:
 * RN2xx3 -- Arduino
 * Uart TX -- 10
 * Uart RX -- 11
 * Reset -- 12
 * Vcc -- 3.3V
 * Gnd -- Gnd
 *
 * Connect the IR to the arduino as follows:
 * Red wire: 5V
 * Black wire: GND
 * Yellow wire: A0
 *
 * Connect the Echo to the arduino as follows:
 * 5V to 5V
 * 0V or GND to GND
 * echoPin to 3
 * Tigger Pin to 4
 * Some chinese echo sensors have the echo and trigger pin switched!
 * 
 *
 */

// samples to take averages of
#define numberOfSamples 9
#include <RunningMedian.h>
// Lora library
#include <SoftwareSerial.h>
#include <rn2xx3.h>
#include "keys.h"

// IR sensor
#define SharpPin A0
int distanceIR;

// Echo sensor
long distanceEcho;
#define trigPin 4
#define echoPin 3

// Delays
#define betweenMeasurements 1000
#define waitBetweenLora ((10*1000) - (betweenMeasurements * numberOfSamples))
// number of seconds minus the seconds used for measurements, so the intervals are still the number of seconds

SoftwareSerial mySerial(10, 11); // RX, TX
//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);

// Buffers for measurements
RunningMedian samplesIR = RunningMedian(numberOfSamples);
RunningMedian samplesEcho = RunningMedian(numberOfSamples);

// the setup routine runs once when you press reset:
void setup()
{  
  pinMode(13, OUTPUT); // led pin
  // setup echo sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  led_on();
  // Open serial communications and wait for port to open:
  Serial.begin(57600); //serial port to computer
  // setup lora
  mySerial.begin(9600); //serial port to lora radio
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
    // take all the samples with [betweenMeasurements] seconds intervals
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
      Serial.print(distanceEcho*10); // because it's cm and we debug in mm so it's the same as the IR
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

/*
 * Check out of reach of sensors
 * Take averages or medians
 * Send to TTN via lora
 * 
 */
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

    // now we split in bytes
    // ir Takes two bytes
    uint16_t prepareDistanceIR = distanceIRFinal & 0xFFFF; // &0xFFFF makes sure the max of the variable is the max of two bytes
    
    uint8_t distance1IR = (prepareDistanceIR >> 8) & 0xFF; // first byte with a max of 255 (one byte max)
    uint8_t distance2IR = (prepareDistanceIR) & 0xFF; // second byte

    // make up the messenge for lora, we have 3 bytes to send
    uint8_t prepareMessenge[] = {preparedistanceEcho, distance1IR, distance2IR};

    // Send it
    Serial.println(F("Lora"));
    myLora.txBytes(prepareMessenge, sizeof(prepareMessenge)); 
    led_off();

    delay(waitBetweenLora);
    // Clear the buffers to start a new
    samplesIR.clear();
    samplesEcho.clear();
}


/** Sharp distance sensor read function - uses analog port
 *  return distance in mm
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

