#include "ACS712.h"
#include <Wire.h>

ACS712 sensor1(ACS712_05B, A0);
ACS712 sensor2(ACS712_05B, A1);
ACS712 sensor3(ACS712_05B, A2);

float tong = 0;
float I;
float I_TB;
float ma;

float power1 = 0;
float power2 = 0;
float power3 = 0;

String stringTransferredToEsp = "";

void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(9600);           /* start serial for debug */

  delay(1200);
  sensor1.calibrate();
  delay(1200);
  sensor2.calibrate();
  delay(1200);
  sensor3.calibrate();
  delay(1200);
}

void loop() {
  power1 = calculateTheAmpere(sensor1);
  power2 = calculateTheAmpere(sensor2);
  power3 = calculateTheAmpere(sensor3);

  stringTransferredToEsp = String("1,") + String(power1)
  + String("k2,") + String(power2)
  + String("k3,") + String(power3) + String("k");

  Serial.println(stringTransferredToEsp);
}

void requestEvent() {
  uint8_t data[26];
  stringTransferredToEsp.getBytes(data, 26);

  for(int i = 0; i < 26; i++) {
    Wire.write(data[i]);
  } 
}

float calculateTheAmpere(ACS712 parameterSensor) {
  tong = 0;
  for (int i=0; i<=100; i++)
  {
    I = parameterSensor.getCurrentAC();
    tong = tong + I;
  } 
  I_TB = tong/100;
  ma = I_TB * 1000;
  return I_TB;
  delay(30);
}
