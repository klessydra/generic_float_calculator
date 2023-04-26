#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "types.h"
#include "vars.h"
#include "defs.h"
#include "funcs.h"

int is_NaN_inf (number f) {                                                                 //returns 0 for normal numbers, 1 for NaN, 2 positive infinity, 3 negative infinity

    bool sign = f.int_i >> ( float_size - 1 );
    if ( sign == 1) {
        f.int_i = f.int_i & ~((uint64_t) pow(2,float_size - 1));                                   //tolgo segno
    }
    int64_t exp = f.int_i >> mantissa_size;
    uint64_t frac = f.int_i & (uint64_t) (pow(2,mantissa_size) - 1);
    if ( exp == pow(2, exponent_size) - 1 ) {
        if ( frac != 0) {
            return 1;
        } else {
            switch (sign) {
                case 0: 
                    return 2;
                    break;
                default:
                    return 3;
                    break;
            }
        }
    }
    return 0;
}

int rand_gen(int lower, int upper) {
  static int seed_set = 0; // Flag to check if the seed has been set
  int range = upper - lower + 1; // Calculate the range of values
  
  if (!seed_set) { // If the seed has not been set yet
    srand(time(NULL)); // Seed the random number generator with the current time
    seed_set = 1; // Set the flag to indicate that the seed has been set
  }
  
  int random_num = rand() % range; // Generate a random integer within the range
  return lower + random_num; // Scale and shift the random integer to fit within the specified range
}