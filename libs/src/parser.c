#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "defs.h" 
#include "vars.h" 
#include "funcs.h"


void parse_args(unsigned int argc, char** argv) {
	for (int i=0; i<=argc; i++) {
		//printf("argv[%d] = %s\n", i, argv[i]);
		if (i < argc) {
			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
				printf(		"\nProgram Options: \n"
							"\t-f, --float size  Sets the size of the float\n"
							"\t-e, --exp_size    Sets the size of the exponent\n"
							"\t-m, --mnt_size    Sets the size of the mantissa\n"
							"\t-b, --bias        Sets the bias\n"
							"\n");
				exit(0);
			}
			if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--float_size") == 0) {
				i++;
				float_set  = 1;
				float_size = atoi(argv[i]);
				printf(PURPLE "Float size: " CRESET "%d\n" , float_size);
			}
			else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--exp_size") == 0) {
				i++;
				exponent_set  = 1;
				exponent_size = atoi(argv[i]);
				printf(PURPLE "Exponent size: " CRESET "%d\n", exponent_size);
			}
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mnt_size") == 0) {
				i++;
				mantissa_set  = 1;
				mantissa_size = atoi(argv[i]);
				printf(PURPLE "Mantissa size: " CRESET "%d\n", mantissa_size);
			}
			else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bias") == 0) {
				i++;
				bias_set = 1;
				bias = atoi(argv[i]);
				custom_bias = 'Y';
				printf(PURPLE "Bias: " CRESET "%d\n", bias);
			}
		}
	}
}