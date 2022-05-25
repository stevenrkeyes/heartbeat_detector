import scipy.signal as signal
import numpy as np

# Number of coefficients to use aka the number of taps
num_coefficients = 20

# Sampling frequency -- must be enforced by the microcontroller
sampling_frequency_hz = 400

# Limits of the band
#frequencies_hz = [0.05, 10]
frequencies_hz = [25]

# Parameters to auto-generate code
name_of_filtered_measurement_variable = "filtered_measurement"
name_of_raw_measurements_buffer = "past_raw_measurements"
name_of_size_of_raw_measurments_buffer = "PAST_RAW_MEASUREMENTS_SIZE"

############################

band_type = "lowpass"

coefficients = signal.firwin(num_coefficients, frequencies_hz, pass_zero=band_type, fs=sampling_frequency_hz)

from matplotlib import pyplot as plt
frequencies_rad_normalized, frequency_responses = signal.freqz(coefficients, [1], worN=8000)
plottable_frequencies_hz = frequencies_rad_normalized * sampling_frequency_hz / (2 * np.pi)
plt.plot(plottable_frequencies_hz, np.absolute(frequency_responses))
plt.title("Frequency Response of Generated Filter")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Gain")
plt.show()

def round_to_n_significant_figures(number_to_round, n):
    return round(number_to_round, n - int(np.floor(np.log10(abs(number_to_round))))-1)



def get_contribution_strings(coefficients, name_of_buffer, name_of_size_of_buffer):
    contribution_strings = []
    for coefficient_index, coefficient in enumerate(coefficients):
        if coefficient != 0:
            if coefficient_index == 0:
                buffer_index_string = "0"
            else:
                buffer_index_string = name_of_size_of_buffer + "-" + str(coefficient_index)
            # 32-bit floats only require at most 7 significant figures
            coefficient_rounded = round_to_n_significant_figures(coefficient, 7)
            contribution_string = str(coefficient_rounded) + " * "
            contribution_string += name_of_buffer + "[" + buffer_index_string + "]"
            contribution_strings.append(contribution_string)
    return contribution_strings

contribution_strings = get_contribution_strings(coefficients, name_of_raw_measurements_buffer, name_of_size_of_raw_measurments_buffer)

c_code = "// FIR filter: " + band_type + " at " + " and ".join([str(f) for f in frequencies_hz]) + " Hz, " + str(num_coefficients) + " taps\n"
c_code = c_code + name_of_filtered_measurement_variable + " = "
c_code += " + ".join(contribution_strings)
c_code += ";"

print(c_code)
