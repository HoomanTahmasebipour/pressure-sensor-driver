/***************************************************************************************************
* Module Name: Convert Precision Sensor Readings To KPa
*
* Author: Hooman Tahmasebipour
* Date: September, 2023
*
* Module Description:
*   This module provides the functionality to convert ADC readings from a precision sensor into pressure 
*   values based on a given  mapping. The primary consideration of this module was the resource constrained 
*   nature of its application.
*
***************************************************************************************************/

#include <stdio.h>
#include <stdint.h>

// Variable definitions that, in the microcontroller environment, would be stored in 
// Flash memory to limit RAM usage.
typedef struct {
    // Pressure is multiplied by FIXED_POINT_ARITH to avoid floating point. Using a 32 bit int for this to 
    // prevent bit overflow with the nominal 16 bit int in the microcontroller. Directly specifying this rather
    // than long for easier readibility.
    int32_t pressure;
    uint16_t adc;
} PressureTableEntry;

static PressureTableEntry pressureTable[] = {
    {  10000, 1696  },
    {  11000, 1909  },
    {  12000, 2118  },
    {  13000, 2272  },
    {  14000, 2366  },
    {  15000, 2448  },
    {  16000, 2570  },
    {  17000, 2745  },
    {  18000, 2931  },
    {  19000, 3073  },
    {  20000, 3151  },
    {  21000, 3200  },
    {  22000, 3278  },
    {  23000, 3411  },
    {  24000, 3573  },
    {  25000, 3706  },
    {  26000, 3777  },
    {  27000, 3808  },
    {  28000, 3853  },
    {  29000, 3955  },
    {  30000, 4100  },
    {  31000, 4236  },
    {  32000, 4316  },
    {  33000, 4348  },
    {  34000, 4382  },
    {  35000, 4468  },
    {  36000, 4610  },
    {  37000, 4762  },
    {  38000, 4871  },
    {  39000, 4927  },
    {  40000, 4971  },
    {  41000, 5058  },
    {  42000, 5210  },
    {  43000, 5390  },
    {  44000, 5541  },
    {  45000, 5639  },
    {  46000, 5710  },
    {  47000, 5812  },
    {  48000, 5979  },
    {  49000, 6190  },
    {  50000, 6389  },
    {  51000, 6534  },
    {  52000, 6641  },
    {  53000, 6762  },
    {  54000, 6943  },
    {  55000, 7177  },
    {  56000, 7414  },
    {  57000, 7604  },
    {  58000, 7743  },
    {  59000, 7877  },
    {  60000, 8060  },
    {  61000, 8302  },
    {  62000, 8560  },
    {  63000, 8778  },
    {  64000, 8938  },
    {  65000, 9074  },
    {  66000, 9243  },
    {  67000, 9470  },
    {  68000, 9726  },
    {  69000, 9954  },
    {  70000, 10119 },
    {  71000, 10244 },
    {  72000, 10383 },
    {  73000, 10577 },
    {  74000, 10810 },
    {  75000, 11028 },
    {  76000, 11187 },
    {  77000, 11292 },
    {  78000, 11394 },
    {  79000, 11542 },
    {  80000, 11739 },
    {  81000, 11937 },
    {  82000, 12085 },
    {  83000, 12170 },
    {  84000, 12237 },
    {  85000, 12340 },
    {  86000, 12498 },
    {  87000, 12675 },
    {  88000, 12815 },
    {  89000, 12893 },
    {  90000, 12938 },
    {  91000, 13007 },
    {  92000, 13135 },
    {  93000, 13299 },
    {  94000, 13444 },
    {  95000, 13531 },
    {  96000, 13575 },
    {  97000, 13631 },
    {  98000, 13744 },
    {  99000, 13908 },
    { 100000, 14073 },
};

// Macro definitions for meeting the 
#define FIXED_POINT_ARITH 100
#define PRESSURE_TABLE_PTR &pressureTable
/* Note : When running the function in the microcontroller environment, change the below macro 
          definition to equal the memory address reserved in Flash memory for pressure table, as 
          discussed in points 2 and 3. */
#define TABLE_SIZE (sizeof(pressureTable) / sizeof(pressureTable[0])) - 1

/* This function will follow the below algorithm steps:
*   1. Determine if the ADC sensor reading to convert to a pressure value is within the bounds 
*      of the saved Presure to ADC Sensor Reading table. 
*   2. If it is, then either the entry will be found directly or interpolation will be used to 
*      determine the appropriate pressure reading associated with the ADC reading.
*        2a. Prior to performing the interpolation, a binary search is performed to either
*            find the entry directly our find the (P1, ADC1) and (P2, ADC2) that are closest to it.
*            This occurs when (int) ((searchWindowEnd - searchWindowStart) / 2) == 0. In performing
*            this, we are taking advantage of the fact that integer division always rounds down in C
*            programming.
*        2b. The next step would be to perform the interpolation using the following formula.
*              Interpolation formula: m = (P2 - P1)/(ADC2 - ADC1) => P = P1 + m * (ADC - ADC1)
*                Where, (P1, ADC1) and (P2, ADC2) are the two Pressure-ADCSensor
*                mapping pairs closest to the provided reading, ADC is the provided sensor reading
*                value, and P is the output pressure reading.
*   3. If it is not, then first a least-squares approximation method is used to arrive at a linear fit 
*      for the saved data. The resulting slope and y-intercept values are then used to extrapolate
*      for the appropriate pressure reading given the input ADC reading.
*           Least Squares minimization to arrive at equations for the slope (m) and y-intercept (b),
*           result in the following formulae:
*               m = (N * sum(ADC_i * P_i) - sum(ADC_i) * sum(P_i)) / N * sum(ADC_i^2) - (sum(ADC_i))^2
*               b = (sum(ADC_i^2) * sum(P_i) - sum(ADC_i) * sum(ADC_i * P_i)) / N * sum(ADC_i^2) - (sum(ADC_i))^2
*           Where N is the total number of data points in the available data set, ADC_i is ADC reading
*           element i in the data set, P_i is Pressure reading i in the data set, and sum(x) denotes 
*           the summation of expression x across all values of ADC_i and/or P_i for i = 0, ..., N.
*    
*   IMPORTANT: The below function provides a method for extrapolation which requires 64-bit integers.
*              If this variable type does not exist in the microcontroller environment, then extrapolation
*              is not possible.
*/
int ConvertADCReadingToPressure(int adcReading, PressureTableEntry* pressureTablePtr) {
    // Total amount of RAM used by run-time variables: 24 Bytes
    // NOTE: The pressureTable size is not included since running this function in the microcontroller 
    //       use-case allows for this table to be be allocated in Flash memory. Further explanations
    //       are in points 2 and 3 under the heading "Developer Notes".
    
    int32_t pressure = 0;

    if ((adcReading >= pressureTablePtr[0].adc) && (adcReading <= pressureTablePtr[TABLE_SIZE].adc)) {
        // The Pressure reading can either be found in the table or can be interpolated.
        // To find the pressure reading in the table, we perform a binary search by taking
        // advantage of the sorted nature of the table and optimize the program for speed.
        
        int16_t searchWindowStart = 0;
        int16_t searchWindowEnd = TABLE_SIZE;
        int16_t midPoint = searchWindowStart + (searchWindowEnd - searchWindowStart) / 2;
        int16_t readingFound = 0;
        int32_t P1, P2;
        uint16_t ADC1, ADC2;
        
        // First check the first and last element of the table, if the ADC Reading is found, 
        // return the associated Pressure reading.
        if (adcReading == pressureTablePtr[0].adc) {
            readingFound = 1;
            pressure = pressureTablePtr[0].pressure;
        }
        else if (adcReading == pressureTablePtr[TABLE_SIZE].adc) {
            readingFound = 1;
            pressure = pressureTablePtr[TABLE_SIZE].pressure;
        }
        else {
            // The requested ADC Reading was not there, so perform a binary search
            while (readingFound == 0) {
                if (adcReading == pressureTablePtr[midPoint].adc) {
                    readingFound = 1;
                    pressure = pressureTablePtr[midPoint].pressure;
                }
                else if (midPoint == searchWindowStart) {
                    // The entries surrounding the input ADC Reading have been found.
                    // Use the interpolation formula to output the Pressur Reading
                    readingFound = 1;
                    P1 = pressureTablePtr[searchWindowStart].pressure;
                    P2 = pressureTablePtr[searchWindowEnd].pressure;
                    ADC1 = pressureTablePtr[searchWindowStart].adc;
                    ADC2 = pressureTablePtr[searchWindowEnd].adc;
                    pressure = P1 + ((P2 - P1) / (ADC2 - ADC1)) * (adcReading - ADC1);
                }
                else if (adcReading < pressureTablePtr[midPoint].adc) {
                    // The ADC Reading is in the first half of the search window
                    searchWindowEnd = midPoint;
                    midPoint = searchWindowStart + (searchWindowEnd - searchWindowStart) / 2;
                }
                else if (adcReading > pressureTablePtr[midPoint].adc) {
                    // The ADC Reading is in the second half of the search window
                    searchWindowStart = midPoint;
                    midPoint = searchWindowStart + (searchWindowEnd - searchWindowStart) / 2;
                }
            }
        }
    }
    else {
        // The Pressure reading must be extrapolated. 
        // NOTE: To prevent bit overflow with the provided data set, either 64-bit integer types are needed 
        //       for Fixed-Point arithmetic operations or SW/HW level floating point support is needed for 
        //       the 32-bit float data type. 
        int64_t sumOfADC_iTimesP_i = 0;
        int64_t sumOfADC_i = 0;
        int64_t sumOfP_i = 0;
        int64_t sumOfADC_iSquared = 0;
        
        for (int16_t i = 0; i <= TABLE_SIZE; i++) {
            sumOfADC_iTimesP_i = sumOfADC_iTimesP_i + pressureTablePtr[i].adc * (pressureTablePtr[i].pressure / FIXED_POINT_ARITH);
            sumOfADC_i = sumOfADC_i + pressureTablePtr[i].adc;
            sumOfP_i = sumOfP_i + pressureTablePtr[i].pressure / FIXED_POINT_ARITH;
            sumOfADC_iSquared = sumOfADC_iSquared + pressureTablePtr[i].adc * pressureTablePtr[i].adc;
        }
        sumOfADC_iTimesP_i = sumOfADC_iTimesP_i / TABLE_SIZE;
        sumOfADC_i = sumOfADC_i / TABLE_SIZE;
        sumOfP_i = sumOfP_i / TABLE_SIZE;
        sumOfADC_iSquared = sumOfADC_iSquared / TABLE_SIZE;
        int64_t slope = (sumOfADC_iTimesP_i - sumOfADC_i * sumOfP_i) * FIXED_POINT_ARITH / (sumOfADC_iSquared - sumOfADC_i * sumOfADC_i);
        int64_t intercept = (sumOfADC_iSquared * sumOfP_i - sumOfADC_i * sumOfADC_iTimesP_i) * FIXED_POINT_ARITH / (sumOfADC_iSquared - sumOfADC_i * sumOfADC_i);
        pressure = slope * adcReading + intercept;
    }
    return pressure;
}

/* 
* The main function contains the test code to exercise the ConvertADCReadingToPressure function. 
* It will continue running until -1 is entered by the operator. Since this code is not intended 
* for execution in the microcontroller environment, but rather on a Windows desktop, the same 
* level of precautions for limiting resource usage is not maintained. However, to follow best 
* practices the software was written to be as efficient in time/complexity as possible.
*/
int main() {
    int adcReading = 0;
    printf("Enter the ADC Sensor Reading to convert to a pressure reading with a precision of 0.01 KPa: \n");
    int res = scanf("%d", &adcReading);
    if (res != 1) {
        printf("Error occured with scanf operation.");
        return -1;
    }
    // Note, we know that the ADC Reading is an unsigned integer. Thus, a value of -1 is not possible 
    // and can be used as a stopping condition.
    while (adcReading >= 0) {
        int pressureReading = ConvertADCReadingToPressure(adcReading, PRESSURE_TABLE_PTR);
        printf("ADC Reading: %d, Pressure Reading: %d \n", adcReading, pressureReading);
        printf("Divide above pressure reading by %d to get the decimal result with 0.01 KPa precision.\n\n", FIXED_POINT_ARITH);
        printf("To exit the program, enter a negative number. Otherwise, enter another number to convert to a pressure reading: \n");
        res = scanf("%d", &adcReading);
        if (res != 1) {
            printf("Error occured with scanf operation.");
            return -1;
        }
    }
    return 0;
}