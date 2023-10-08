#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

int round_f8_1(int frac, int precision, int inexact);

float8_1_t bf16_to_f8_1( float16_t a )
{
    union ui16_f16 uA;
    uint_fast16_t uiA;
    bool sign;
    int_fast16_t exp;
    uint_fast8_t frac;
    struct commonNaN commonNaN;
    uint_fast8_t uiZ, frac8_1;
    union ui8_f8_1 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = signBF16_2UI( uiA );
    exp  = expBF16_2UI( uiA );
    frac = fracBF16_2UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    int inexact   = 0;
    int precision = 0;
    int overflow  = 0;
    if (exp <= 0x78) { // 0x78 is the largest exponent that converts to a denorm in f8_1 
        if (exp == 0x00) {
            exp  = 0x0;
            frac = 0x0;
            if (frac != 0x00) {
                inexact = 1;
            }
            uiZ  = packToF8_1UI( sign, exp, frac);
        }
        else if (exp == 0x75) {
            precision = 1;
            if (frac != 0x00) {
                inexact   = 1;
            }
            exp  = 0x0;
            frac = 0x0;
            uiZ = packToF8_1UI( sign, exp, frac);
        }
        else if (exp == 0x76) {
            if (frac != 0x00) {
                inexact = 1;
                if (frac == 0x40) {
                    precision = 1;
                    inexact   = 0;
                }
                else if (frac > 0x40) {
                    precision = 1;
                }
            }
            exp  = 0x0;
            frac = 0x1;
            uiZ = packToF8_1UI( sign, exp, frac);
        }
        else if (exp == 0x77) {
            if (frac < 0x40) {
                if (frac != 0x00) {
                    inexact = 1;
                    if (frac == 0x20) {
                        precision = 1;
                        inexact = 0;
                    }
                    else if (frac > 0x20) {
                        precision = 1;
                    }
                }
                exp  = 0x0;
                frac = 0x2;
                uiZ = packToF8_1UI( sign, exp, frac);
            }
            else {
                if (frac != 0x40) {
                    inexact = 1;
                    if (frac == 0x60) {
                        precision = 1;
                        inexact = 0;
                    }
                    else if (frac > 0x60) {
                        precision = 1;
                    }
                }
                exp  = 0x0;
                frac = 0x3;
                uiZ = packToF8_1UI( sign, exp, frac);
            }
        }
        else if (exp == 0x78) {
            if (frac < 0x20) {
                if (frac != 0x00) {
                    inexact = 1;
                    if (frac == 0x10) {
                        precision = 1;
                        inexact = 0;
                    }
                    else if (frac > 0x10) {
                        precision = 1;
                    }
                }
                exp  = 0x0;
                frac = 0x4;
                uiZ = packToF8_1UI( sign, exp, frac);
            }
            else if (frac < 0x40) {
                if (frac != 0x20) {
                    inexact = 1;
                    if (frac == 0x30) {
                        precision = 1;
                        inexact = 0;
                    }
                    else if (frac > 0x30) {
                        precision = 1;
                    }
                }
                exp  = 0x0;
                frac = 0x5;
                uiZ = packToF8_1UI( sign, exp, frac);
            }
            else if (frac < 0x60) {
                if (frac != 0x40) {
                    inexact = 1;
                    if (frac == 0x50) {
                        precision = 1;
                        inexact   = 0;
                    }
                    else if (frac > 0x50) {
                        precision = 1;
                    }
                }
                exp  = 0x0;
                frac = 0x6;
                uiZ = packToF8_1UI( sign, exp, frac);
            }
            else {
                if (frac != 0x60) {
                    inexact = 1;
                    if (frac == 0x70) {
                        precision = 1;
                        inexact   = 0;
                    }
                    else if (frac > 0x70) {
                        precision = 1;
                    }
                }
                exp  = 0x0;
                frac = 0x7;
                uiZ = packToF8_1UI( sign, exp, frac);
            }
        }
    }
    else if (exp <= 0x86) {  	  // 0x86 is the biggest number in which mantissa is not overflowing
        int mask_frac = 0x70;     // creates a mask equal to 111_0000 which enables only the mantissa in the core
        int mask_precision = 0x8; // creates a mask equal to 000_1000 which enables only the mantissa in the core
        int mask_inexact = 0x07;  // creates a mask equal to 000_0111 which enables only the mantissa in the core
        precision = (frac & mask_precision) >> 3;
        inexact   = (frac & mask_inexact);
        exp       = exp - 127 + 7;
        frac      = (frac & mask_frac) >> 4;
        uiZ = packToF8_1UI( sign, exp, frac);
    }
    else if ( exp < 0xFF ) {
        exp = 0xF;
        frac = 0x0;
        uiZ = packToF8_1UI( sign, exp, frac);
    }
    else if ( exp == 0xFF ) {
        if ( frac ) {
            softfloat_bf16UIToCommonNaN( uiA, &commonNaN );
            uiZ = softfloat_commonNaNToF8_1UI( &commonNaN );
        } else {
            exp = 0xF;
            frac = 0x0;
            uiZ = packToF8_1UI( sign, exp, frac);
        }
        goto uiZ;
    }
    if (exp == 0xE && expF8_1UI(uiZ + round_f8_1(frac, precision, inexact)) == 1) {
        overflow == 1;
        softfloat_raiseFlags(softfloat_flag_overflow | softfloat_flag_inexact);
    }
    else if (inexact == 1 || precision == 1) {
        softfloat_raiseFlags(softfloat_flag_inexact );
        if (exp == 0) {
            softfloat_raiseFlags(softfloat_flag_underflow);
        }
    }
 uiZ:
    uZ.ui = uiZ + round_f8_1(frac, precision, inexact);
    //printf("uiZ = 0x%x, sign = %d, exp = 0x%lx, frac = 0x%x\n", uiZ, sign, exp, frac);
    return uZ.f;
}

int round_f8_1(int frac, int precision, int inexact) {
    //printf("frac = 0x%x, precision = %d, inexact = %x\n", frac, precision, inexact);
    int round = 0; // round down is the default unless we round up based on the following conditions
    int even_mask = 0x1;
    if (inexact == 0) {
        if (precision == 1) {
            if (frac & even_mask == 1) {
                round = 1;
            }
        }
    }
    else {
        if (precision == 1) { // round up
            round = 1;
        }
    }
    return round;
}
