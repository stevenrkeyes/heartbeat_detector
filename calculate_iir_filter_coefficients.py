import scipy.signal as signal
import numpy as np

# Number of coefficients to use -- must be odd
num_coefficients = 5

# Sampling frequency -- must be enforced by the microcontroller
sampling_frequency_hz = 1000

# Limits of the band
frequencies_hz = [0.05, 10]

# Parameters to auto-generate code
name_of_raw_measurement_variable = "raw_measurement"
name_of_raw_measurements_buffer = "past_raw_measurements"
name_of_size_of_raw_measurments_buffer = "PAST_RAW_MEASUREMENTS_SIZE"
name_of_filtered_measurement_variable = "filtered_measurement"
name_of_filtered_measurements_buffer = "past_filtered_measurements"
name_of_size_of_filtered_measurments_buffer = "PAST_FILTERED_MEASUREMENTS_SIZE"

############################

band_type = "bandpass"

# The number of coefficents is one plus the filter order for each filter frequency;
# Since this is a bandpass filter, that will be 1 + 2 * filter order.
filter_order = (num_coefficients - 1) / 2

b_coefficients, a_coefficients = signal.iirfilter(filter_order, frequencies_hz, btype=band_type, fs=sampling_frequency_hz)

from matplotlib import pyplot as plt

frequencies_rad_normalized, frequency_responses = signal.freqz(b_coefficients, a_coefficients, worN=8000)
plottable_frequencies_hz = frequencies_rad_normalized * sampling_frequency_hz / (2 * np.pi)
plt.plot(plottable_frequencies_hz, np.absolute(frequency_responses))
plt.title("Frequency Response of Generated Filter")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Gain")
plt.show()

z, p, gain = signal.iirfilter(filter_order, frequencies_hz, output="zpk", btype=band_type, fs=sampling_frequency_hz)
plt.plot(np.real(z), np.imag(z), 'ob', markerfacecolor='none')
plt.plot(np.real(p), np.imag(p), 'xr')
plt.title('Pole / Zero Plot')
plt.xlabel('Real')
plt.ylabel('Imaginary')
plt.grid()
plt.gca().add_patch(plt.Circle((0, 0), 1, fill=False))
plt.gca().set_aspect(aspect=1)
plt.show()


def round_to_n_significant_figures(number_to_round, n):
    return round(number_to_round, n - int(np.floor(np.log10(abs(number_to_round))))-1)



def get_contribution_strings(coefficients, name_of_buffer, name_of_size_of_buffer):
    contribution_strings = []
    for coefficient_index, coefficient in enumerate(coefficients):
        if coefficient != 0:
            # buffer[len-1] is the most recent sample, buffer[len-2] is the 2nd most recent, etc.
            buffer_index_string = name_of_size_of_buffer + " - " + str(coefficient_index + 1)
            # 32-bit floats only require at most 7 significant figures
            coefficient_rounded = round_to_n_significant_figures(coefficient, 7)
            contribution_string = str(coefficient_rounded) + " * "
            contribution_string += name_of_buffer + "[" + buffer_index_string + "]"
            contribution_strings.append(contribution_string)
    return contribution_strings

b_contribution_strings = get_contribution_strings(b_coefficients, name_of_raw_measurements_buffer, name_of_size_of_raw_measurments_buffer)
# Skip the first a coefficient since it is the coefficient of the value to be calculated. For this reason, all other values have been normalized to it, so it will always be 1.
# Also, negate them since they will need to be negated per their form
a_contribution_strings = get_contribution_strings(-a_coefficients[1:], name_of_filtered_measurements_buffer, name_of_size_of_filtered_measurments_buffer)

c_code = "// IIR filter: " + band_type + " at " + " and ".join([str(f) for f in frequencies_hz]) + "Hz, " + str(num_coefficients) + " taps\n"
c_code = c_code + name_of_filtered_measurement_variable + " = "
c_code += " + ".join(b_contribution_strings)
c_code += " + "
c_code += " + ".join(a_contribution_strings)
c_code += ";"

print(c_code)
print()
print(b_coefficients)
print(a_coefficients)
