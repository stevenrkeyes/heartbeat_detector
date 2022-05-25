#include <CircularBuffer.h>
#include <SoftPWM.h>


#define LED_PIN LED_BUILTIN

// Period after detection of an R-peak to wait before further detections are allowed
#define COOLDOWN_MS 500

// System time of the last detected R-peak
long beat_detected_time_ms = 0;

// Buffers for digital filtering
#define PAST_RAW_MEASUREMENTS_SIZE 50
#define PAST_FILTERED_MEASUREMENTS_SIZE 200
CircularBuffer<int, PAST_RAW_MEASUREMENTS_SIZE> past_raw_measurements;
CircularBuffer<float, PAST_FILTERED_MEASUREMENTS_SIZE> past_filtered_measurements;
float filtered_measurement = 0;

void setup() {
  // initialize the serial communication:
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  // Initialize all buffers to 0s
  for (int index = 0; index < PAST_RAW_MEASUREMENTS_SIZE; index++) {
    past_raw_measurements.push(0);
  }
  for (int index = 0; index < PAST_FILTERED_MEASUREMENTS_SIZE; index++) {
    past_filtered_measurements.push(0);
  }
}

// Detects an r-peak in the filtered data based on whether it is >90% of the previous data (mean removed), as long as it is long enough since the previous peak
// Todo: fix this, it is too slow. Probably remove the mean calculation and make the FIR filter be a IIR bandpass. Or maybe use integer math for the mean calculation.
//   Also, replace the max sample calculation with something faster, such as a circular buffer that tracks which index is the max.
bool is_r_peak() {
  long candidate_beat_detected_time_ms = millis();
  if (candidate_beat_detected_time_ms - beat_detected_time_ms < COOLDOWN_MS) {
    return false;
  }
     
  // Calculate mean
  float filtered_measurement_total = 0;
  for (int index = 0; index < PAST_FILTERED_MEASUREMENTS_SIZE; index++){
    filtered_measurement_total += past_filtered_measurements[index];
  }
  float filtered_measurement_mean = filtered_measurement_total / PAST_FILTERED_MEASUREMENTS_SIZE;

  // Calculate max sample
  float max_sample = 0;
  for (int index = 0; index < PAST_FILTERED_MEASUREMENTS_SIZE; index++){
    if (past_filtered_measurements[index] > max_sample) {
      max_sample = past_filtered_measurements[index];
    }
  }

  float max_sample_minus_mean = max_sample - filtered_measurement_mean;
  float filtered_measurement_minus_mean = filtered_measurement - filtered_measurement_mean;

  if (filtered_measurement_minus_mean > 0.90 * max_sample_minus_mean) {
    beat_detected_time_ms = candidate_beat_detected_time_ms;
    return true;
  }
  return false;
}

void loop() {

  int raw_measurement = analogRead(A0);
  
  past_raw_measurements.push(raw_measurement);
  
  // FIR filter: lowpass at 25 Hz, 20 taps
  filtered_measurement = -0.00162832 * past_raw_measurements[0] + -0.0008381586 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-1] + 0.001602399 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-2] + 0.00857948 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-3] + 0.02247107 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-4] + 0.04384433 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-5] + 0.0706881 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-6] + 0.09857132 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-7] + 0.1217617 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-8] + 0.1349481 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-9] + 0.1349481 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-10] + 0.1217617 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-11] + 0.09857132 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-12] + 0.0706881 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-13] + 0.04384433 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-14] + 0.02247107 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-15] + 0.00857948 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-16] + 0.001602399 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-17] + -0.0008381586 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-18] + -0.00162832 * past_raw_measurements[PAST_RAW_MEASUREMENTS_SIZE-19];  past_filtered_measurements.push(filtered_measurement);
  
  Serial.println(filtered_measurement);

  past_filtered_measurements.push(filtered_measurement);

  // Check if this is a heartbeat
  if (is_r_peak()) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }


  //Wait for a bit to keep serial data from saturating
  delay(1);
}
