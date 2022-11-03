#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "defs.h" 
#include "vars.h" 
#include "funcs.h"


void check_argc_index(int index, int argc, char** argv) {
	if (index >= argc) {
		printf(RED "\nERROR: " CRESET "Missing argument for \"%s\"\n", argv[index-1]);
		exit(1);
	}
}

void parse_args(unsigned int argc, char** argv) {
	for (int i=0; i<=argc; i++) {
		//printf("argv[%d] = %s\n", i, argv[i]);
		if (i < argc) {
			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
				printf(		
					"USAGE: fp_gm [OPTION]... [VALUE]...\n"
					"Generic Floating Point Calculator\n"
					"\nProgram Options: \n"
					"  -f,  --float_size <arg>   \n"
					"                            Sets the size of the float\n"
					"  -e,  --exp_size <arg>     \n"
					"                            Sets the size of the exponent\n"
					"  -m,  --mnt_size <arg>     \n"
					"                            Sets the size of the mantissa\n"
					"  -b,  --bias <arg>         \n"
					"                            Sets the bias\n"
					"  -f2, --float2_size <arg>  \n"
					"                            Sets the size of the second float \n"
					"                            (float to float conversion only)\n"
					"  -e2, --exp2_size <arg>    \n"
					"                            Sets the size of the second exponent\n"
					"                            (float to float conversion only)\n"
					"  -m2, --mnt2_size <arg>    \n"
					"                            Sets the size of the second mantissa\n"
					"                            (float to float conversion only)\n"
					"  -b2, --bias2 <arg>        \n"
					"                            Sets the second bias \n"
					"                            (float to float conversion only)\n"
					"  -r, --round_mode <arg>    \n"
					"                            Sets the round mode \n"
					"                                (1)  RNE\n"
					"                                (2)  RTZ\n"
					"                                (3)  RDN\n"
					"                                (4)  RUP\n"
					"                                (5)  RMM\n"
					"       --op <arg> \n"
					"                            Sets the type of operation:\n"
					"                                (0)  GENERATE ALL\n"
					"                                (1)  GENERATE NUMBERS\n"
					"                                (2)  ADDITION\n"
					"                                (3)  SUBTRACTION\n"
					"                                (4)  MULTIPLICATION\n"
					"                                (5)  DIVISION\n"
					"                                (6)  SQUARE ROOT\n"
					"                                (7)  FUSED MULTIPLY-ADD\n"
					"                                (8)  CONVERSION\n"
					"                                (9)  FLOAT LOOKUP (returns hex):\n"
					"                                (10) HEX LOOKUP (returns float):\n"
					"       --cop <arg> \n"
					"                            Sets the type of conversion-operation\n"
					"                                 (1) FLOAT_type_1 to FLOAT_type_2\n"
					"                                 (2) FLOAT to INT\n"
					"                                 (3) FLOAT to UINT\n"
					"                                 (4) INT to FLOAT\n"
					"                                 (5) UINT to FLOAT\n"
					"  -fo, --file_out \n"
					"                            Sets the output to be printed to a file\n"
					"  -s, --single_case \n"
					"                            Perform a single case\n"
					"  -a, --all_cases \n"
					"                            Perform all cases\n"
					"\n");
				exit(0);
			}
			if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--float_size") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				float_set  = 1;
				float_size = atoi(argv[i]);
				printf(PURPLE "Float size: " CRESET "%d\n" , float_size);
			}
			else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--exp_size") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				exponent_set  = 1;
				exponent_size = atoi(argv[i]);
				printf(PURPLE "Exponent size: " CRESET "%d\n", exponent_size);
			}
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mnt_size") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				mantissa_set  = 1;
				mantissa_size = atoi(argv[i]);
				printf(PURPLE "Mantissa size: " CRESET "%d\n", mantissa_size);
			}
			else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bias") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				bias_set = 1;
				bias = atoi(argv[i]);
				custom_bias = 'Y';
				printf(PURPLE "Bias: " CRESET "%d\n", bias);
			}
			else if (strcmp(argv[i], "-f2") == 0 || strcmp(argv[i], "--float2_size") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				float2_set  = 1;
				float2_size = atoi(argv[i]);
				printf(PURPLE "Float 2 size: " CRESET "%d\n" , float2_size);
			}
			else if (strcmp(argv[i], "-e2") == 0 || strcmp(argv[i], "--exp2_size") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				exponent2_set  = 1;
				exponent2_size = atoi(argv[i]);
				printf(PURPLE "Exponent 2 size: " CRESET "%d\n", exponent2_size);
			}
			else if (strcmp(argv[i], "-m2") == 0 || strcmp(argv[i], "--mnt2_size") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				mantissa2_set  = 1;
				mantissa2_size = atoi(argv[i]);
				printf(PURPLE "Mantissa 2 size: " CRESET "%d\n", mantissa2_size);
			}
			else if (strcmp(argv[i], "-b2") == 0 || strcmp(argv[i], "--bias2") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				bias2_set = 1;
				bias_2 = atoi(argv[i]);
				custom_bias = 'Y';
				printf(PURPLE "Bias 2: " CRESET "%d\n", bias_2);
			}
			else  {
				if (i > 0) {
					printf(RED "\nERROR: " CRESET "Unknown option \"%s\"\n", argv[i]);
					exit(1);
				}
			}
		}
	}
}