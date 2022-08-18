#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "vars.h"
#include "defs.h"

void round_fp(uint64_t mantissa_extract, uint64_t sign_extract, double mantissa, double mantissa_middle, double mantissa_lower, double mantissa_upper, uint64_t* mantissa_rounded, uint64_t* exponent_rounded) {
	const uint64_t lsb_en = 1;
	uint64_t nearest_even_shift;
	if ((mantissa_extract + 1) & lsb_en == 1) {  
		nearest_even_shift = -1;
	}
	else {
		nearest_even_shift = 1;
	}
	if (round_mode == 1) { // RNE
		if (mantissa == mantissa_middle) {
			if (mantissa_extract == (int)(mnt_range-1.0) && nearest_even_shift == 1) {
				*exponent_rounded += 1;
			}
			if (mantissa_extract & lsb_en) {
				*mantissa_rounded = mantissa_extract + nearest_even_shift;
			}
			else {
				*mantissa_rounded = mantissa_extract;
			}
		}
		else if (mantissa > mantissa_middle) {
			if (mantissa_extract == (int)(mnt_range-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;
		}
		else { // mantissa < mantissa_middle
			*mantissa_rounded = mantissa_extract;
		}
	}
	else if (round_mode == 2) { // RTZ
		if (mantissa == mantissa_upper) {  // take the upper bound if the mantissa is equal to it, else take always the lower bound
			if (mantissa_extract == (int)(mnt_range-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;	
		}
		else {
			*mantissa_rounded = mantissa_extract;
		}
	}
	else if (round_mode == 3) { // RDN
		if (sign_extract == 0) {
			if (mantissa == mantissa_upper) {  // take the upper bound if the mantissa is equal to it, else take always the lower bound
				if (mantissa_extract == (int)(mnt_range-1.0)) {
					*exponent_rounded += 1;
				}
				*mantissa_rounded = mantissa_extract + 1;	
			}
			else {
				*mantissa_rounded = mantissa_extract;
			}
		}
		else {
			if (mantissa == mantissa_lower) {
				*mantissa_rounded = mantissa_extract;	
			}
			else {
				if (mantissa_extract == (int)(mnt_range-1.0)) {
					*exponent_rounded += 1;
				}
				*mantissa_rounded = mantissa_extract + 1;
			}
		}		
	}
	else if (round_mode == 4) { // RUP
		if (sign_extract == 0) {
			if (mantissa == mantissa_lower) {
				*mantissa_rounded = mantissa_extract;	
			}
			else {
				if (mantissa_extract == (int)(mnt_range-1.0)) {
					*exponent_rounded += 1;
				}
				*mantissa_rounded = mantissa_extract + 1;
			}
		}
		else {
			if (mantissa == mantissa_upper) {  // take the upper bound if the mantissa is equal to it, else take always the lower bound
				if (mantissa_extract == (int)(mnt_range-1.0)) {
					*exponent_rounded += 1;
				}
				*mantissa_rounded = mantissa_extract + 1;	
			}
			else {
				*mantissa_rounded = mantissa_extract;
			}
		}		
	}
	else if (round_mode == 5) { // RMM
		if (mantissa >= mantissa_middle) {
			if (mantissa_extract == (int)(mnt_range-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;
		}
		else {
			*mantissa_rounded = mantissa_extract;
		}
	}
}