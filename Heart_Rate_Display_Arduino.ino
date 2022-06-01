// CircularBuffer library from AgileWare, accessible from the Arduino library manager
#include <CircularBuffer.h>


#define LED_PIN LED_BUILTIN
#define ECG_INPUT_PIN A0
#define LEADS_OFF_P_PIN A3

// Only identify r-peaks in the ECG signal if they are spaced farther apart than the following cooldown, to avoid false detections
#define R_PEAK_COOLDOWN_MS 500
long beat_detected_system_time_ms = 0;

// The leads off signal is a little noisy, so assume the electrodes are disconnected if a leads-off has been detected within the following delay
#define LEADS_OFF_COOLDOWN_MS 200
long last_leads_off_system_time_ms = 0;

void setup() {
  // Initialize the serial communication:
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(ECG_INPUT_PIN, INPUT);
  pinMode(LEADS_OFF_P_PIN, INPUT);
}

float filtered_measurement = 0;
float filtered_measurement_high = 0;
float filtered_measurement_low = 0;

CircularBuffer<float, 200> past_samples;

void loop() {
  int raw_measurement = analogRead(ECG_INPUT_PIN);
  
  // Do a little bit of digital high- and low- pass filtering
  filtered_measurement_high = 0.8 * filtered_measurement_high + 0.2 * raw_measurement;
  filtered_measurement_low = 0.99 * filtered_measurement_low + 0.01 * raw_measurement;
  filtered_measurement = filtered_measurement_high - filtered_measurement_low;

  // The electrodes (at least the positive one) are disconnected from a person (leads off) if the pin is high
  bool leads_off = digitalRead(LEADS_OFF_P_PIN);
  if (leads_off) {
    last_leads_off_system_time_ms = millis();
  }

  past_samples.push(filtered_measurement);
  float max_sample = 0;
  for (int index = 0; index < past_samples.size(); index++){
    if (past_samples[index] > max_sample) {
      max_sample = past_samples[index];
    }
  }

  long candidate_beat_detected_system_time_ms = millis();
  // Check that we are leads on (including the cooldown of any recent leads off events)
  if (candidate_beat_detected_system_time_ms - last_leads_off_system_time_ms > LEADS_OFF_COOLDOWN_MS) {
    // Check that the sample is large enough
    if (filtered_measurement > 0.9 * max_sample) {
       if (candidate_beat_detected_system_time_ms - beat_detected_system_time_ms > R_PEAK_COOLDOWN_MS) {
         beat_detected_system_time_ms = candidate_beat_detected_system_time_ms;
         digitalWrite(LED_PIN, HIGH);
       } else {
         digitalWrite(LED_PIN, LOW);
       }
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
