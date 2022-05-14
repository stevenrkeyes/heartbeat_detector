/******************************************************************************
Heart_Rate_Display.ino
Demo Program for AD8232 Heart Rate sensor.
Casey Kuhns @ SparkFun Electronics
6/27/2014
https://github.com/sparkfun/AD8232_Heart_Rate_Monitor

The AD8232 Heart Rate sensor is a low cost EKG/ECG sensor.  This example shows
how to create an ECG with real time display.  The display is using Processing.
This sketch is based heavily on the Graphing Tutorial provided in the Arduino
IDE. http://www.arduino.cc/en/Tutorial/Graph

Resources:
This program requires a Processing sketch to view the data in real time.

Development environment specifics:
	IDE: Arduino 1.0.5
	Hardware Platform: Arduino Pro 3.3V/8MHz
	AD8232 Heart Monitor Version: 1.0

This code is beerware. If you see me (or any other SparkFun employee) at the
local pub, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include <CircularBuffer.h>
#include <SoftPWM.h>


#define LED_PIN LED_BUILTIN

#define COOLDOWN_MS 500

long beat_detected_time_ms;

void led_heartbeat(uint8_t led_pin, long start_time_ms) {
  int fade_value;
  long current_time_ms = millis();
  long elapsed_time_ms = current_time_ms - start_time_ms;

  if (elapsed_time_ms < 100) {
    fade_value = (int)elapsed_time_ms;
  } else if (elapsed_time_ms < 200) {
    fade_value = 100 - (elapsed_time_ms - 100);
  } else if (elapsed_time_ms < 250) {
    fade_value = 0;
  } else if (elapsed_time_ms < 290) {
    fade_value = elapsed_time_ms - 250;
  } else if (elapsed_time_ms < 330) {
    fade_value = 40 - (elapsed_time_ms - 290);
  } else {
    fade_value = 0;
  }
  
  SoftPWMSet(led_pin, fade_value);
}

void setup() {
  // initialize the serial communication:
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  SoftPWMBegin();
  SoftPWMSetFadeTime(LED_PIN, 0, 0);
  SoftPWMSet(LED_PIN, 0);

  beat_detected_time_ms = 0;
}

float filtered_measurement = 0;
float filtered_measurement_high = 0;
float filtered_measurement_low = 0;

CircularBuffer<float, 200> past_samples;

void loop() {

  int raw_measurement = analogRead(A0);
  
  // Do a little bit of digital high- and low- pass filtering
  filtered_measurement_high = 0.8 * filtered_measurement_high + 0.2 * raw_measurement;
  filtered_measurement_low = 0.99 * filtered_measurement_low + 0.01 * raw_measurement;
  filtered_measurement = filtered_measurement_high - filtered_measurement_low;

  past_samples.push(filtered_measurement);
  float max_sample = 0;
  for (int index = 0; index < past_samples.size(); index++){
    if (past_samples[index] > max_sample) {
      max_sample = past_samples[index];
    }
  }

  if (filtered_measurement > 0.9 * max_sample) {
     long candidate_beat_detected_time_ms = millis();
     if (candidate_beat_detected_time_ms - beat_detected_time_ms > COOLDOWN_MS) {
       beat_detected_time_ms = candidate_beat_detected_time_ms;
     }
  }
  
  led_heartbeat(LED_PIN, beat_detected_time_ms);

  Serial.println(filtered_measurement);
  
  //Wait for a bit to keep serial data from saturating
  delay(1);
}
