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
					"                                (2)  ARITHMETIC\n"
					"                                (3)  CONVERSION\n"
					"                                (4)  FLOAT LOOKUP (returns hex):\n"
					"                                (5)  HEX LOOKUP (returns float):\n"
					"       --arith_op <arg> \n"
					"                            Sets the type of operation:\n"
					"                                (1)  ADDITION\n"
					"                                (2)  SUBTRACTION\n"
					"                                (3)  MULTIPLICATION\n"
					"                                (4)  DIVISION\n"
					"                                (5)  SQUARE ROOT\n"
					"                                (6)  FUSED MULTIPLY-ADD\n"
					"       --conv_op <arg> \n"
					"                            Sets the type of conversion-operation\n"
					"                                 (1)  FLOAT_type_1 to FLOAT_type_2\n"
					"                                 (2)  FLOAT to INT\n"
					"                                 (3)  FLOAT to UINT\n"
					"                                 (4)  INT to FLOAT\n"
					"                                 (5)  UINT to FLOAT\n\n"
					"  -fo, --file_out \n"
					"                            Sets the output to be printed to a file\n\n"
					"  --single_case \n"
					"                            Perform a single case\n\n"
					"  --random_cases \n"
					"                            Perform random cases\n\n"
					"  --all_cases \n"
					"                            Perform all cases\n\n"
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
			else if (strcmp(argv[i], "--op") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				op_set = 1;
				op_type = atoi(argv[i]);
				printf(WHITE "OP TYPE: " CRESET "%d\n", op_type);
			}
			else if (strcmp(argv[i], "--arith_op") == 0) {
				if (conv_op_set == 1) {
					printf(RED "ERROR: " CRESET "Cannot set both ARTH_TYPE and CONV_TYPE\n");
					exit(1);
				} 
				i++;
				check_argc_index(i, argc, argv);
				arith_op_set = 1;
				arith_type = atoi(argv[i]);
				printf(WHITE "ARITHMETIC OP TYPE: " CRESET "%d\n", arith_type);
			}
			else if (strcmp(argv[i], "--conv_op") == 0) {
				if (arith_op_set == 1) {
					printf(RED "ERROR: " CRESET "Cannot set both ARTH_TYPE and CONV_TYPE\n");
					exit(1);
				}
				i++;
				check_argc_index(i, argc, argv);
				conv_op_set = 1;
				conv_type = atoi(argv[i]);
				printf(WHITE "CONVERSION OP TYPE: " CRESET "%d\n", conv_type);
			}
			else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--round_mode") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				round_mode_set  = 1;
				round_mode = atoi(argv[i]);
				printf(WHITE "ROUND MODE: " CRESET "%d\n", round_mode);
			}
			else if (strcmp(argv[i], "-fo") == 0 || strcmp(argv[i], "--file_out") == 0) {
				i++;
				check_argc_index(i, argc, argv);
				file_out_set  = 1;
				if (strcmp(argv[i], "Y") == 0 || strcmp(argv[i], "y") == 0) {
					file_print = 'Y';
					printf(BLUE "WRITE FILE OUT: " CRESET "YES\n");
				}
				else if (strcmp(argv[i], "N") == 0 || strcmp(argv[i], "n") == 0) {
					file_print = 'N';
					printf(BLUE "WRITE FILE OUT: " CRESET "NO\n");
				}
				else {
					printf(RED "ERROR:" CRESET" Unknown argument to please \"-fo \\--file_out\", please choose \'Y\'/\'y\'' or \'N\'/\'n\'\n");
				}
			}
			else if (strcmp(argv[i], "--single_case") == 0) {
				choice_set  = 1;
				choice_type = 1;
				printf(GREEN "TEST SCOPE: " CRESET "SINGLE RESULT\n");
			}
			else if (strcmp(argv[i], "--random_cases") == 0) {
				choice_set  = 1;
				choice_type = 2;
				printf(GREEN "TEST SCOPE: " CRESET "RANDOM SET OF RESULTS\n");
			}
			else if (strcmp(argv[i], "--all_cases") == 0) {
				choice_set  = 1;
				choice_type = 3;
				printf(GREEN "TEST SCOPE: " CRESET "ALL RESULTS\n");
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