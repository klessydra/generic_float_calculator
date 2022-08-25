#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

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