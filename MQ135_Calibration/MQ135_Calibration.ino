/*
Source: https://benjaminstelzner.de/mq135-gas-sensor/
*/
#include "MQ135.h"
#define RLOAD 22000
//#define RZERO 76.63
#define RZERO 30600

MQ135 gasSensor = MQ135(A0); 
int val; 
int sensorPin = A0; 
int sensorValue = 0;

int runningSum = 0;
int counter = 1;

void setup() { 
  Serial.begin(115200);
  pinMode(sensorPin, INPUT); 
} 
  
void loop() { 
  float zero = gasSensor.getRZero(); 
  Serial.print("RZERO: "); 
  Serial.println(zero);

  float avgRzero =(runningSum += zero) / counter++; 
  Serial.print("Average RZERO: ");
  Serial.println(avgRzero);
  Serial.print("Running Sum:  ");
  Serial.println(runningSum);
  Serial.print("Counter: ");
  Serial.println(counter);
  
  float ppm = gasSensor.getPPM(); 
  Serial.print ("ppm: "); 
  Serial.println (ppm); 
  delay(2000); 
} 
