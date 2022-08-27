#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "vars.h"
#include "defs.h"
#include "funcs.h"

void hex_to_float(number f, double* f_out) {
	const uint64_t lsb_en = 1;
	int64_t sign_extract     = 0;
	int64_t exponent_extract = 0;
	int64_t mantissa_extract = 0;
	double   mantissa_extract_f = 0;
	int64_t mantissa_en = mnt_range-1; 
	int64_t exponent_en = exp_range-1; 
	int64_t denormalized = 0;
	int64_t bias_compeensate = 0;
	if (f.int_i >= (uint64_t) pow(2,float_size)) {
 		printf(RED "\nERROR: " WHITE "%lx Out of range for the defined float type\n", f.int_i);
 		exit(-1);
	}
	else {
		sign_extract     = (f.int_i >> float_size-1) & lsb_en;
		exponent_extract = (f.int_i >> mantissa_size) & exponent_en;
		mantissa_extract = (f.int_i & mantissa_en);
		if (exponent_extract > 0) {
			mantissa_extract_f = 1;
			denormalized      = 0;
			bias_compeensate   = 0;
		}
		else {
			mantissa_extract_f = 0;
			denormalized = 1;
			bias_compeensate = 1;
		}
		for (int i=1; i<=mantissa_size; i++) {
			mantissa_extract_f += (double)((mantissa_extract >> (mantissa_size-i)) & lsb_en) * (1 / pow(2, i));
		}
		//printf(CYAN "exponent_extract = %ld\n", exponent_extract);
		//printf(CYAN "mantissa_extract = %ld\n", mantissa_extract);
		//printf(CYAN "mantissa_extract_F= %lf\n", mantissa_extract_f);
		if (sign_extract == 0) {
			*f_out = pow(2, exponent_extract-(bias-bias_compeensate)) * mantissa_extract_f;
			if (exponent_extract == (int) exp_range-1) {
				if (mantissa_extract > 0) {
					*f_out = 0.0 / 0.0;
				}
				else {
					*f_out = INFINITY;
				}
			}
		}
		else {
			*f_out = -pow(2, exponent_extract-(bias-bias_compeensate)) * mantissa_extract_f;
			if (exponent_extract == (int) exp_range-1) {
				if (mantissa_extract > 0) {
					*f_out = -0.0 / 0.0;
				}
				else {
					*f_out = -INFINITY;
				}
			}
		}
	}
}

int float_to_hex(number f, uint64_t* myfloat_h, double* f_out) {
	const uint64_t lsb_en = 1;
	uint64_t mantissa_en = mnt_range-1; 
	uint64_t exponent_en = exp_range-1; 
	uint64_t exponent_extract = 0;
	uint64_t mantissa_extract = 0;
	uint64_t mantissa_rounded = 0;
	uint64_t exponent_rounded = 0;
	double   denormalized = 0;
	double   normalized   = 0;
	double mantissa_rounded_f;
	uint64_t sign_extract = (f.int_i >> 63) & lsb_en;
	double exponent_lower  = pow(2,-(bias+mantissa_size-1));
	double exponent_upper  = pow(2,-(bias+mantissa_size));
	double mantissa_lookup = 0.0;
	double mantissa_lower;
	double mantissa_upper;
	double mantissa_middle;
	double mantissa;
	int exact = 0;
	int underflow = 0; 
	double absolute_float = fabs(f.float_i);
	if (isnan(f.float_i)) {
		// Generate a qnan (Quiet NaN)
		mantissa_rounded = pow(2,mantissa_size-1);
		exponent_rounded = exp_range-1; 
		*myfloat_h = (sign_extract << float_size-1) + (exponent_rounded << mantissa_size) + mantissa_rounded;
	}
	else if (absolute_float >= upper_bound) {
		mantissa_rounded = 0;
		exponent_rounded = exp_range-1; 
		*myfloat_h = (sign_extract << float_size-1) + (exponent_rounded << mantissa_size) + mantissa_rounded;
		//printf(CYAN "HERE\n");
	}
	else if (absolute_float == 0) { // if input float is 0 
		*myfloat_h = sign_extract << float_size-1;
		//printf(CYAN "HERE2\n");
	}
	else {
		if (absolute_float >= 0 && absolute_float < exponent_lower) {
			underflow = 1;
		}
		for (int i=-(bias+mantissa_size-1); i<=bias; i++) {
			exponent_lower = pow(2,i);
			exponent_upper = pow(2,i+1);
			if ((absolute_float >= exponent_lower && absolute_float < exponent_upper) || underflow == 1) {
				//printf(CYAN "EXPONENT: %d\n", i);
				if (i < -(bias-1)) {  // denormalized exponent
					exponent_extract = 0;
					mantissa = absolute_float * pow(2,bias-1);
					denormalized = 1;
					normalized   = 0;
				}
				else {
					exponent_extract = i + bias;
					mantissa = absolute_float / pow(2,i);
					denormalized = 0;
					normalized   = 1;
				}
				exponent_rounded = exponent_extract;
				//printf(CYAN "MANTISSA: %f\n", mantissa);
				mantissa_lower = mantissa_lookup;
				mantissa_upper = 1.0 / mnt_range;
				mantissa_middle = (mantissa_lower + mantissa_upper) / 2;
				for (int j=1; j<=(int)mnt_range; j++) {
					mantissa_lower = normalized + mantissa_lookup;
					mantissa_lookup += 1.0 / mnt_range; 
					mantissa_upper = normalized + mantissa_lookup;
					mantissa_middle = (mantissa_lower + mantissa_upper) / 2;
					if (mantissa == mantissa_upper || mantissa == mantissa_lower) {
						exact = 1;
					}
					if (mantissa <= mantissa_upper) {
						round_fp(mantissa_extract, sign_extract, mantissa, mantissa_middle,  mantissa_lower, mantissa_upper, &mantissa_rounded, &exponent_rounded);
						break;
					}
					mantissa_extract += 1;
				}
				//printf(CYAN "mnt_lower= %lf, mnt_upper = %lf, mnt_middle = %lf \n", mantissa_lower, mantissa_upper, mantissa_middle);
				//printf(CYAN "mnt_extract= %ld\n", mantissa_extract);
				//printf(CYAN "exp_extract= %ld\n", exponent_extract);
				//printf(CYAN "exp_rounded= %ld\n", exponent_rounded);
				//printf(CYAN "mnt_rounded= %lx\n", mantissa_rounded);
				//printf(CYAN "mantissa_size= %d\n", mantissa_size);
				*myfloat_h = (sign_extract << float_size-1) + ((exponent_rounded & exponent_en) << mantissa_size) + (mantissa_rounded & mantissa_en);
				break;
			}
		}
	}
	if (exponent_rounded == 1) {
		denormalized = 0;	
		mantissa_rounded_f = 1;
	}
	if (denormalized == 1)
		mantissa_rounded_f = 0;
	else
		mantissa_rounded_f = 1;
	for (int i=1; i<=mantissa_size; i++) {
		mantissa_rounded_f += (double)((mantissa_rounded >> (mantissa_size-i)) & lsb_en) * (1 / pow(2, i));
	}
	//printf(CYAN "mnt_rounded_f = %lf\n" CRESET, mantissa_rounded_f);
	if (sign_extract == 1) {
		*f_out = -pow(2, (double)exponent_rounded-(bias-denormalized)) * mantissa_rounded_f;
		if (fabs(*f_out) >= upper_bound) {
			*f_out = -INFINITY;
		}
	}
	else {
		*f_out =  pow(2, (double)exponent_rounded-(bias-denormalized)) * mantissa_rounded_f;
		if (fabs(*f_out) >= upper_bound) {
			*f_out = INFINITY;
		}
	}
	return exact;
}

int float_to_hex_2(number f, uint64_t* myfloat_h, double* f_out) {
	const uint64_t lsb_en = 1;
	uint64_t mantissa_en = mnt_range_2-1; 
	uint64_t exponent_en = exp_range_2-1; 
	uint64_t exponent_extract = 0;
	uint64_t mantissa_extract = 0;
	uint64_t mantissa_rounded = 0;
	uint64_t exponent_rounded = 0;
	double   denormalized = 0;
	double   normalized   = 0;
	double mantissa_rounded_f;
	uint64_t sign_extract = (f.int_i >> 63) & lsb_en;
	double exponent_lower  = pow(2,-(bias+mantissa_size-1));
	double exponent_upper  = pow(2,-(bias+mantissa_size));
	double mantissa_lookup = 0.0;
	double mantissa_lower;
	double mantissa_upper;
	double mantissa_middle;
	double mantissa;
	int exact = 0;
	int underflow = 0; 
	double absolute_float = fabs(f.float_i);
	if (isnan(f.float_i)) {
		// Generate a qnan (Quiet NaN)
		mantissa_rounded = pow(2,mantissa_size_2-1);
		exponent_rounded = exp_range_2-1; 
		*myfloat_h = (sign_extract << float_size_2-1) + (exponent_rounded << mantissa_size_2) + mantissa_rounded;
	}
	else if (absolute_float >= upper_bound_2) {
		mantissa_rounded = 0;
		exponent_rounded = exp_range_2-1; 
		*myfloat_h = (sign_extract << float_size_2-1) + (exponent_rounded << mantissa_size_2) + mantissa_rounded;
		//printf(CYAN "HERE\n");
	}
	else if (absolute_float == 0) { // if input float is 0 
		*myfloat_h = sign_extract << float_size_2-1;
		//printf(CYAN "HERE2\n");
	}
	else {
		if (absolute_float >= 0 && absolute_float < exponent_lower) {
			underflow = 1;
		}
		for (int i=-(bias_2+mantissa_size_2-1); i<=bias_2; i++) {
			exponent_lower = pow(2,i);
			exponent_upper = pow(2,i+1);
			if ((absolute_float >= exponent_lower && absolute_float < exponent_upper) || underflow == 1) {
				//printf(CYAN "EXPONENT: %d\n", i);
				if (i < -(bias_2-1)) {  // denormalized exponent
					exponent_extract = 0;
					mantissa = absolute_float * pow(2,bias_2-1);
					denormalized = 1;
					normalized   = 0;
				}
				else {
					exponent_extract = i + bias_2;
					mantissa = absolute_float / pow(2,i);
					denormalized = 0;
					normalized   = 1;
				}
				exponent_rounded = exponent_extract;
				//printf(CYAN "MANTISSA: %f\n", mantissa);
				mantissa_lower = mantissa_lookup;
				mantissa_upper = 1.0 / mnt_range_2;
				mantissa_middle = (mantissa_lower + mantissa_upper) / 2;
				for (int j=1; j<=(int)mnt_range_2; j++) {
					mantissa_lower = normalized + mantissa_lookup;
					mantissa_lookup += 1.0 / mnt_range_2; 
					mantissa_upper = normalized + mantissa_lookup;
					mantissa_middle = (mantissa_lower + mantissa_upper) / 2;
					if (mantissa == mantissa_upper || mantissa == mantissa_lower) {
						exact = 1;
					}
					if (mantissa <= mantissa_upper) {
						round_fp(mantissa_extract, sign_extract, mantissa, mantissa_middle,  mantissa_lower, mantissa_upper, &mantissa_rounded, &exponent_rounded);
						break;
					}
					mantissa_extract += 1;
				}
				//printf(CYAN "mnt_lower= %lf, mnt_upper = %lf, mnt_middle = %lf \n", mantissa_lower, mantissa_upper, mantissa_middle);
				//printf(CYAN "mnt_extract= %ld\n", mantissa_extract);
				//printf(CYAN "exp_extract= %ld\n", exponent_extract);
				//printf(CYAN "exp_rounded= %ld\n", exponent_rounded);
				//printf(CYAN "mnt_rounded= %lx\n", mantissa_rounded);
				//printf(CYAN "mantissa_size= %d\n", mantissa_size);
				*myfloat_h = (sign_extract << float_size_2-1) + ((exponent_rounded & exponent_en) << mantissa_size_2) + (mantissa_rounded & mantissa_en);
				break;
			}
		}
	}
	if (exponent_rounded == 1) {
		denormalized = 0;	
		mantissa_rounded_f = 1;
	}
	if (denormalized == 1)
		mantissa_rounded_f = 0;
	else
		mantissa_rounded_f = 1;
	for (int i=1; i<=mantissa_size_2; i++) {
		mantissa_rounded_f += (double)((mantissa_rounded >> (mantissa_size_2-i)) & lsb_en) * (1 / pow(2, i));
	}
	//printf(CYAN "mnt_rounded_f = %lf\n" CRESET, mantissa_rounded_f);
	if (sign_extract == 1) {
		*f_out = -pow(2, (double)exponent_rounded-(bias_2-denormalized)) * mantissa_rounded_f;
		if (fabs(*f_out) >= upper_bound_2) {
			*f_out = -INFINITY;
		}
	}
	else {
		*f_out =  pow(2, (double)exponent_rounded-(bias_2-denormalized)) * mantissa_rounded_f;
		if (fabs(*f_out) >= upper_bound_2) {
			*f_out = INFINITY;
		}
	}
	return exact;
}