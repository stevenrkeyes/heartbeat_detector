import serial
import time
import numpy as np
from matplotlib import pyplot as plt

port = "COM5"

serial_device = serial.Serial(port=port, baudrate=115200)

# Clear the buffer if there is one
# This might also just be resetting the Arduino fwiw
serial_device.reset_input_buffer()
serial_device.readline()

num_messages_to_read = 500
stop_times = np.zeros(num_messages_to_read)
for message_index in range(num_messages_to_read):
    serial_device.readline()
    stop_times[message_index] = time.time()
serial_device.close()


def average_every_n(array, n):
    len_rounded_down_to_n = n * int(len(array) / n)
    truncated_array = array[:len_rounded_down_to_n]
    reshaped_array = truncated_array.reshape(int(len_rounded_down_to_n / n), n)
    return reshaped_array.mean(axis=1)

time_differences_seconds = np.diff(stop_times)

plt.plot(average_every_n(time_differences_seconds, 5))
plt.show()

effective_sample_rate_hertz = num_messages_to_read / (stop_times[-1] - stop_times[0])
print(round(effective_sample_rate_hertz, 1))
