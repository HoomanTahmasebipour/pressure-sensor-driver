# Driver to Convert Precision Pressure Sensor Readings To KPa for an 8-bit Microcontroller 

Author: Hooman Tahmasebipour

Date: September, 2023

This program is written for an 8-bit microcontroller to provide the functionality to convert ADC readings from a 
precision sensor into pressure values based on a given mapping. The primary consideration of this module was the
resource-constrained nature of its application. 

To run this application, clone the repository into a directory of your choosing and run the bash script
to spin up the docker container (labelled "**run_dockerized_solution.sh**"). This automated script will attach 
a terminal to the run and enable an interactive session. Alternatively, the solution can be compiled using 
an IDE of your choosing, although be wary of compiler mismatches resulting in phantom errors. Running the 
docker container will not result in this.

In order to optimize total RAM usage and time-efficiency of the algorithm, two key techniques were used:

1. For optimizing RAM storage, the function that would be ported into the 8-bit microcontroller
   environment takes in the address of the Pressure-ADC mapping look-up table, which would be
   stored in Flash memory prior to running the ConvertADCReadingToPressure function.
2. For optimizing run-time, the binary search algorithm was implemented to reduce the time  
   complexity of finding the Pressure-ADC entry in the look-up table from O(n) to O(log(n)).
   For the provided data set with 90 entries, the worst-case number of iterations is 8.

This module performs interpolation when the input reading is not located directly in the Pressure-ADC mapping, but 
it is within its boundaries, and extrapolation when the input value is outside the bounds of the look-up table. It 
should be noted that this extrapolation can only work if integer types with 64 bits are available. The module is 
designed to work in a resource-constrained environment and uses fixed-point arithmetic to achieve precision without 
relying on floating-point operations.

int ConvertADCReadingToPressure(int adcReading, PressureTableEntry* pressureTablePtr): 
- Function to be run in the microcontroller environment

int main():
- Test code to exercise ConvertADCReadingToPressure(...) in a desktop environment

Notes:
- This module assumes the Pressure-ADC mapping is stored in a sorted array.
- For optimal performance, the ADC readings should fall within the range of the predefined table.
- Interpolation is used when an exact match for the ADC reading isn't found in the table.
- If the ADC reading is out of bounds of the table, extrapolation is applied.
- It is recommended to store the Pressure-ADC mapping in Flash memory to conserve RAM.
- The implementation avoids unnecessary use of RAM by minimizing intermediate variable usage.
- Floating point arithmetic is achieved to achieve 0.01 KPa percision when there is no floating 
	point support in the microcontroller. This is done by applying a multiplication factor of 100 
	in all computations, and ensuring that the output is treated as the scaled value of the real
	pressure measurement.
