#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

int round_f8_2(int frac, int precision, int inexact);

float8_2_t bf16_to_f8_2( float16_t a )
{
    union ui16_f16 uA;
    uint_fast16_t uiA;
    bool sign;
    int_fast16_t exp;
    uint_fast8_t frac;
    struct commonNaN commonNaN;
    uint_fast8_t uiZ, frac8_2;
    union ui8_f8_2 uZ;

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
    if (exp <= 0x70) { // 0x70 is the largest exponent that converts to a denorm in f8_2
        if (exp == 0x00) {
            exp  = 0x0;
            frac = 0x0;
            if (frac != 0x00) {
                inexact = 1;
            }
            uiZ  = packToF8_2UI( sign, exp, frac);
        }
        else if (exp == 0x6E) {
            precision = 1;
            if (frac != 0x00) {
                inexact   = 1;
            }
            exp  = 0x0;
            frac = 0x0;
            uiZ = packToF8_2UI( sign, exp, frac);
        }
        else if (exp == 0x6F) {
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
            uiZ = packToF8_2UI( sign, exp, frac);
        }
        else if (exp == 0x70) {
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
                uiZ = packToF8_2UI( sign, exp, frac);
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
                uiZ = packToF8_2UI( sign, exp, frac);
            }
        }
    }
    else if (exp <= 0x8E) {  	   // 0x8E is the biggest number in which mantissa is not overflowing
        int mask_frac = 0x60;      // creates a mask equal to 110_0000 which enables only the mantissa in the core
        int mask_precision = 0x10; // creates a mask equal to 001_0000 which enables only the mantissa in the core
        int mask_inexact = 0x0F;   // creates a mask equal to 000_1111 which enables only the mantissa in the core
        precision = (frac & mask_precision) >> 4;
        inexact   = (frac & mask_inexact);
        exp       = exp - 127 + 15;
        frac      = (frac & mask_frac) >> 5;
        uiZ = packToF8_2UI( sign, exp, frac);
    }
    else if ( exp < 0xFF ) {
        exp = 0x1F;
        frac = 0x0;
        uiZ = packToF8_2UI( sign, exp, frac);
    }
    else if ( exp == 0xFF ) {
        if ( frac ) {
            softfloat_bf16UIToCommonNaN( uiA, &commonNaN );
            uiZ = softfloat_commonNaNToF8_2UI( &commonNaN );
        } else {
            exp = 0x1F;
            frac = 0x0;
            uiZ = packToF8_2UI( sign, exp, frac);
        }
        goto uiZ;
    }
    if (exp == 0x1E && expF8_2UI(uiZ + round_f8_2(frac, precision, inexact)) == 1) {
        overflow == 1;
        softfloat_raiseFlags(softfloat_flag_overflow | softfloat_flag_inexact);
    }
    else if (inexact == 1 || precision == 1) {
        softfloat_raiseFlags(softfloat_flag_inexact);
        if (exp == 0) {
            softfloat_raiseFlags(softfloat_flag_underflow);
        }
    }
 uiZ:
    uZ.ui = uiZ + round_f8_2(frac, precision, inexact);
    //printf("uiZ = 0x%x, sign = %d, exp = 0x%lx, frac = 0x%x\n", uiZ, sign, exp, frac);
    return uZ.f;
}

int round_f8_2(int frac, int precision, int inexact) {
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
