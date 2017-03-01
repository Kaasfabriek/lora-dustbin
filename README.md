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
 * The public_html folder contains a website to display the data
 *
 * 
 */

