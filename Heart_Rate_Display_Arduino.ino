// CircularBuffer library from AgileWare, accessible from the Arduino library manager
#include <CircularBuffer.h>


#define LED_PIN LED_BUILTIN

#define COOLDOWN_MS 500

long beat_detected_time_ms;

void setup() {
  // initialize the serial communication:
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  
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
       digitalWrite(LED_PIN, HIGH);
     } else {
       digitalWrite(LED_PIN, LOW);
     }
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  Serial.println(filtered_measurement);
  
  //Wait for a bit to keep serial data from saturating
  delay(1);
}
