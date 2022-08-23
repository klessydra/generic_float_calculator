#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

#include <string.h>

#include "types.h"
#include "defs.h" 
#include "funcs.h"

int float_size    = 0;
int exponent_size = 0;
int mantissa_size = 0;
int bias          = 0;

int float_size_2    = 0;
int exponent_size_2 = 0;
int mantissa_size_2 = 0;
int bias_2          = 0;

char custom_bias;
char custom_bias_2;

int op_type;
int choice_type;
int conv_type;
int round_mode;
int integer_width;

char file_print = 'Y';

double upper_bound;
double exp_range;
double mnt_range;

double upper_bound_2;
double exp_range_2;
double mnt_range_2;

int float_set = 0;
int exponent_set = 0;
int mantissa_set = 0;
int bias_set = 0;

int float_set_2 = 0;
int exponent_set_2 = 0;
int mantissa_set_2 = 0;
int bias_set_2 = 0;

FILE file1;
FILE file2;
FILE file3;
FILE file_num;
FILE file_add;
FILE file_sub;
FILE file_mul;
FILE file_div;
FILE file_sqrt;
FILE file_fma;

FILE* file1_ptr     = &file1;
FILE* file2_ptr     = &file2;
FILE* file3_ptr     = &file3;
FILE* file_num_ptr  = &file_num;
FILE* file_add_ptr  = &file_add;
FILE* file_sub_ptr  = &file_sub;
FILE* file_mul_ptr  = &file_mul;
FILE* file_div_ptr  = &file_div;
FILE* file_sqrt_ptr = &file_sqrt;
FILE* file_fma_ptr  = &file_fma;

int main(unsigned int argc, char** argv) {

	int exact;

	uint64_t myfloat_h  = 0x0;
	uint64_t mysum_h	= 0x0;
	uint64_t mysub_h	= 0x0;
	uint64_t mymul_h	= 0x0;
	uint64_t mydiv_h	= 0x0;

	double   f1_out, f2_out, f3_out, fsum_out, fsub_out, fmul_out, fdiv_out, fsqrt_out, fma_out = 0x0;
	number f1, f2, f3, fsum, fsub, fmul, fdiv, fsqrt, fma;

	error_count err_cnt;
	err_cnt.error_cnt_op_type = 0;
	err_cnt.error_cnt_conv_type = 0;
	err_cnt.error_cnt_round_mode = 0;

	printf("\n");

	parse_args(argc, argv);

	SET_FLOAT_SIZE:
	if (float_set == 0) {
		printf(PURPLE "Enter Float Size: " CRESET);
		scanf("%d", &float_size);
	}

	int valid_float = check_input_float();
	if (valid_float == FP_ERROR) {
		float_set = 0;
		goto SET_FLOAT_SIZE;		
	}

	SET_EXPONENT_SIZE:
	if (exponent_set == 0) {
		printf(PURPLE "Enter exponent size: " CRESET);
		scanf("%d", &exponent_size);
	}

	int valid_exponent = check_input_exponent();
	if (valid_exponent == FP_ERROR) {
		exponent_set = 0;
		goto SET_EXPONENT_SIZE;		
	}

	SET_MANTISSA_SIZE:
	if (mantissa_set == 0) {
		printf(PURPLE "Enter mantissa size: " CRESET);
		scanf("%d", &mantissa_size);
	}

	int valid_mantissa = check_input_mantissa();
	if (valid_mantissa == FP_ERROR) {
		mantissa_set = 0;
		goto SET_MANTISSA_SIZE;		
	}

    SET_CUSTOM_BIAS:
	if (bias_set == 0) {
		printf(BLUE "\nDo you want a custom bias? (Y/N) " CRESET);
		scanf(" %c", &custom_bias);
		int valid_bias = check_input_bias();
		if (valid_bias == FP_ERROR) {
			bias_set = 0;
			goto SET_CUSTOM_BIAS;		
		}
	}

	SET_OP_TYPE:

	printf("\nINSERT OP TYPE:\n"
				 "	(0)  GENERATE ALL\n"
				 "	(1)  GENERATE NUMBERS\n"
				 "	(2)  ADDITION\n"
				 "	(3)  SUBTRACTION\n"
				 "	(4)  MULTIPLICATION\n"
				 "	(5)  DIVISION\n"
				 "	(6)  SQUARE ROOT\n"
				 "	(7)  FUSED MULTIPLY-ADD\n"
				 "	(8)  CONVERSION\n"
				 "	(9)  FLOAT LOOKUP (returns hex):\n"
				 "	(10) HEX LOOKUP (returns float):\n"
				 "\nOP TYPE: "
				);
	scanf("%d", &op_type);

	int valid_op_type = check_input_op_type();
	if (valid_op_type == FP_ERROR) {
		goto SET_OP_TYPE;		
	}

	SET_ROUNDING_MODE:

	if (op_type != 1 && op_type != 10) {
		printf("\nINSERT ROUNDING MODE:\n"
					 "	(1)  RNE\n"
					 "	(2)  RTZ\n"
					 "	(3)  RDN\n"
					 "	(4)  RUP\n"
					 "	(5)  RMM\n"
					 "\nROUNDING MODE: "
					);
		scanf("%d", &round_mode);

		int valid_rounding_mode = check_input_rounding_mode();
		if (valid_rounding_mode == FP_ERROR) {
			goto SET_ROUNDING_MODE;		
		}
	}

	PRINT_RES:
	printf(BLUE "\nDo you want to Print the result to a file (Y/N): " CRESET);
	scanf(" %c", &file_print);	
	int valid_print_out_answer = check_input_print_answer();
	if (valid_print_out_answer == FP_ERROR) {
		goto PRINT_RES;		
	}

	char* filename1     = malloc(200*sizeof(char));
	char* filename2     = malloc(200*sizeof(char));
	char* filename3     = malloc(200*sizeof(char));
	char* filename_num  = malloc(200*sizeof(char));
	char* filename_add  = malloc(200*sizeof(char));
	char* filename_sub  = malloc(200*sizeof(char));
	char* filename_mul  = malloc(200*sizeof(char));
	char* filename_div  = malloc(200*sizeof(char));
	char* filename_sqrt = malloc(200*sizeof(char));
	char* filename_fma  = malloc(200*sizeof(char));
	sprintf(filename1,     "result/fp%d(1-%d-%d)_1_op_res.txt", float_size, exponent_size, mantissa_size);
	sprintf(filename2,     "result/fp%d(1-%d-%d)_2_op_res.txt", float_size, exponent_size, mantissa_size);
	sprintf(filename3,     "result/fp%d(1-%d-%d)_3_op_res.txt", float_size, exponent_size, mantissa_size);
	sprintf(filename_num,  "result/fp%d(1-%d-%d)_num_res.txt",  float_size, exponent_size, mantissa_size);
	sprintf(filename_add,  "result/fp%d(1-%d-%d)_add_res.txt",  float_size, exponent_size, mantissa_size);
	sprintf(filename_sub,  "result/fp%d(1-%d-%d)_sub_res.txt",  float_size, exponent_size, mantissa_size);
	sprintf(filename_mul,  "result/fp%d(1-%d-%d)_mul_res.txt",  float_size, exponent_size, mantissa_size);
	sprintf(filename_div,  "result/fp%d(1-%d-%d)_div_res.txt",  float_size, exponent_size, mantissa_size);
	sprintf(filename_sqrt, "result/fp%d(1-%d-%d)_sqrt_res.txt", float_size, exponent_size, mantissa_size);
	sprintf(filename_fma,  "result/fp%d(1-%d-%d)_fma_res.txt",  float_size, exponent_size, mantissa_size);

	//double mantissa_upper_bound = 1.0;
	//for (int i=1; i<=mantissa_size; i++) {
	//	mantissa_upper_bound += 1.0/pow(2,i);
	//}
	//printf(CYAN "mantissa upper bound = %f\n\n" CRESET, mantissa_upper_bound);
	//double num_upper_bound = pow(2,bias)* mantissa_upper_bound;// + mantissa_upper_bound);
	//printf(CYAN "upper bound = %f\n\n" CRESET, num_upper_bound);

	uint64_t number_of_floats = (uint64_t) pow(2,float_size);

	upper_bound = pow(2,(pow(2,(double)exponent_size)-1-bias));
	exp_range = pow(2,exponent_size);
	mnt_range = pow(2,mantissa_size);

	FILE *out1_result;
	FILE *out2_result;
	FILE *out3_result;
	FILE *out_num_result;
	FILE *out_add_result;
	FILE *out_sub_result;
	FILE *out_mul_result;
	FILE *out_div_result;
	FILE *out_sqrt_result;
	FILE *out_fma_result;
	//char green = ' ';

	char* black  = malloc(sizeof BLACK);
	char* red    = malloc(sizeof RED);
	char* green  = malloc(sizeof GREEN);
	char* yellow = malloc(sizeof YELLOW);
	char* blue   = malloc(sizeof BLUE);
	char* purple = malloc(sizeof PURPLE);
	char* cyan   = malloc(sizeof CYAN);
	char* white  = malloc(sizeof WHITE);
	char* creset = malloc(sizeof CRESET);

	if (file_print == 'Y' || file_print == 'y') {
		if (op_type == 0) {
			file1_ptr    = fopen(filename1, "w");
			file2_ptr    = fopen(filename2, "w");
			file3_ptr    = fopen(filename3, "w");
		}
		else if (op_type == 1)
			file_num_ptr = fopen(filename_num, "w");
		else if (op_type == 2)
			file_add_ptr = fopen(filename_add, "w");
		else if (op_type == 3)
			file_sub_ptr = fopen(filename_sub, "w");
		else if (op_type == 4)
			file_mul_ptr = fopen(filename_mul, "w");
		else if (op_type == 5)
			file_div_ptr = fopen(filename_div, "w");
		else if (op_type == 6)
			file_sqrt_ptr = fopen(filename_sqrt, "w");
		else if (op_type == 7)
			file_fma_ptr = fopen(filename_fma, "w");
		out1_result     = file1_ptr;
		out2_result     = file2_ptr;
		out3_result     = file3_ptr;
		out_num_result  = file_num_ptr;
		out_add_result  = file_add_ptr;
		out_sub_result  = file_sub_ptr;
		out_mul_result  = file_mul_ptr;
		out_div_result  = file_div_ptr;
		out_sqrt_result = file_sqrt_ptr;
		out_fma_result  = file_fma_ptr;
		black  = "";
		red    = "";
		green  = "";
		yellow = "";
		blue   = "";
		purple = "";
		cyan   = "";
		white  = "";
		creset = "";
	}
	else {
		out1_result     = stdout; 
		out2_result     = stdout; 
		out3_result     = stdout; 
		out_num_result  = stdout; 
		out_add_result  = stdout; 
		out_sub_result  = stdout; 
		out_mul_result  = stdout; 
		out_div_result  = stdout;
		out_sqrt_result = stdout;
		out_fma_result  = stdout;
		black  = BLACK;
		red    = RED;
		green  = GREEN;
		yellow = YELLOW;
		blue   = BLUE;
		purple = PURPLE;
		cyan   = CYAN;
		white  = WHITE;
		creset = CRESET;
	}

	if (op_type == 0) {
		fprintf(out2_result, "%s\nFLOAT_%d (1-%d-%d)\n", green, float_size, exponent_size, mantissa_size);
		fprintf(out2_result,  "%s\n\tOP_A\tSQRT\n", green);
		for (int i=0; i<number_of_floats; i++){
				f1.int_i = i;
				hex_to_float(f1, &f1_out);
				fsqrt.float_i = sqrt(f1_out);
				float_to_hex(fsqrt, &myfloat_h, &fsqrt_out);
				fprintf(out1_result, "%s\t0x%.*lx\t0x%.*lx\n%s", 
						cyan,
						float_size/4, f1.int_i, float_size/4, myfloat_h,
						creset
					  );
		}
		fprintf(out2_result, "%s\nFLOAT_%d (1-%d-%d)\n", green, float_size, exponent_size, mantissa_size);
		fprintf(out2_result,  "%s\n\tOP_A\tOP_B\tADD\tSUB\tMUL\tDIV\n", green);
		for (int i=0; i<number_of_floats; i++){
			for (int j=0; j<number_of_floats; j++){
				f1.int_i = i;
				f2.int_i = j;
				hex_to_float(f1, &f1_out);
				hex_to_float(f2, &f2_out);
				fsum.float_i = f1_out + f2_out;
				fsub.float_i = f1_out - f2_out;
				fmul.float_i = f1_out * f2_out;
				fdiv.float_i = f1_out / f2_out;
				float_to_hex(fsum, &mysum_h, &fsum_out);
				float_to_hex(fsub, &mysub_h, &fsub_out);
				float_to_hex(fmul, &mymul_h, &fmul_out);
				float_to_hex(fdiv, &mydiv_h, &fdiv_out);
				fprintf(out2_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s", 
						cyan,
						float_size/4, f1.int_i, float_size/4, f2.int_i,
						float_size/4, mysum_h,  float_size/4, mysub_h, 
						float_size/4, mymul_h,  float_size/4, mydiv_h,
						creset
					  );
				//printf(CYAN "\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\n\n" CRESET, 
				//		f1.int_i, f1_out, f2.int_i, f2_out, mysum_h, fsum_out, mysub_h, fsub_out, mymul_h, fmul_out, mydiv_h, fdiv_out);
			}
		}
		fprintf(out2_result, "%s\nFLOAT_%d (1-%d-%d)\n", green, float_size, exponent_size, mantissa_size);
		fprintf(out2_result,  "%s\n\tOP_A\tOP_B\tOP_C\tFMA\n", green);
		for (int i=0; i<number_of_floats; i++){
			for (int j=0; j<number_of_floats; j++){
				for (int k=0; k<number_of_floats; k++){
					f1.int_i = i;
					f2.int_i = j;
					f3.int_i = k;
					hex_to_float(f1, &f1_out);
					hex_to_float(f2, &f2_out);
					hex_to_float(f3, &f3_out);
					fma.float_i = (f1_out * f2_out) + f3_out;
					float_to_hex(fma, &myfloat_h, &fma_out);
					fprintf(out3_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s", 
							cyan,
							float_size/4, f1.int_i, float_size/4, f2.int_i, float_size/4, f3.int_i,
							float_size/4, myfloat_h,
							creset
						  );
				}
			}
		}
		fclose(out1_result);
		fclose(out2_result);
		fclose(out3_result);
	}

	if (op_type == 1) {
		fprintf(out_num_result, "%s\nFLOAT_%d (1-%d-%d)\n", green, float_size, exponent_size, mantissa_size);
		for (int i=0; i<number_of_floats; i++){
			f1.int_i = i;
			hex_to_float(f1,  &f1_out);
			fprintf(out_num_result, "%s\t0x%lx (%E)\n%s", cyan, f1.int_i, f1_out, creset);
		}
	}

	if (op_type == 2) {
		printf(GREEN "\nADDITION\n");
		ADDITION_OPTION:
		printf(      "\nCHOOSE OPTION:\n"
					 "	(1)  SINGLE ADDITION\n"
					 "	(2)  ALL POSSIBLE ADDITIONS\n"
					 "\nOPTION: "
					);
		scanf("%d", &choice_type);

		int valid_choice_type = check_input_choice_type();
		if (valid_choice_type == FP_ERROR) {
			goto ADDITION_OPTION;		
		}

		if (choice_type == 1) {
			printf(GREEN "\nInsert float 1 in hex: " CRESET);
			scanf("%lx", &f1.int_i);
			printf(GREEN "Insert float 2 in hex: " CRESET);
			scanf("%lx", &f2.int_i);
			hex_to_float(f1, &f1_out);
			hex_to_float(f2, &f2_out);
			fsum.float_i = f1_out + f2_out;
			exact = float_to_hex(fsum, &myfloat_h, &fsum_out);
			fprintf(out_add_result, "%s\n\tf1_out + f2_out = fsum.float_i\n%s", cyan, creset);
			if (exact == 1) {
				fprintf(out_add_result, "%s\t%lf + %lf = %lf\n%s", cyan, f1_out, f2_out, fsum.float_i, creset);
			}
			else {
				fprintf(out_add_result, "%s\t%lf + %lf = %lf (%sROUNDED: %s%lf)\n%s", cyan, f1_out, f2_out, fsum.float_i, yellow, cyan, fsum_out, creset);
			}
			fprintf(out_add_result, "%s\t(0x%lx) + (0x%lx) = (0x%lx)\n\n%s", cyan, f1.int_i, f2.int_i, myfloat_h, creset);
		/************************************************************************************************************************/
		}	else if (choice_type == 2) {
			for (int i=0; i<number_of_floats; i++){
				for (int j=0; j<number_of_floats; j++){
					f1.int_i = i;
					f2.int_i = j;
					hex_to_float(f1, &f1_out);
					hex_to_float(f2, &f2_out);
					fsum.float_i = f1_out + f2_out;
					float_to_hex(fsum, &mysum_h, &fsum_out);
					fprintf(out_add_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s", 
							cyan, 
							float_size/4, f1.int_i, float_size/4, f2.int_i,
							float_size/4, mysum_h,
							creset
					  	  );
				}
			}
		}
	}

	if (op_type == 3) {
		printf(GREEN "\nSUBTRACTION\n");
	SUBTRACTION_OPTION:
		printf(      "\nCHOOSE OPTION:\n"
					 "	(1)  SINGLE SUBTRACTION\n"
					 "	(2)  ALL POSSIBLE SUBTRACTIONS\n"
					 "\nOPTION: "
					);
		scanf("%d", &choice_type);

		int valid_choice_type = check_input_choice_type();
		if (valid_choice_type == FP_ERROR) {
			goto SUBTRACTION_OPTION;		
		}

		if (choice_type == 1) {
			printf(GREEN "\nInsert float 1 in hex: " CRESET);
			scanf("%lx", &f1.int_i);
			printf(GREEN "Insert float 2 in hex: " CRESET);
			scanf("%lx", &f2.int_i);
			hex_to_float(f1, &f1_out);
			hex_to_float(f2, &f2_out);
			fsub.float_i = f1_out - f2_out;
			exact = float_to_hex(fsub, &myfloat_h, &fsub_out);
			fprintf(out_sub_result, "%s\n\tf1_out - f2_out = fsub.float_i\n%s", cyan, creset);
			if (exact == 1) {
				fprintf(out_sub_result, "%s\t%lf - %lf = %lf\n%s", cyan, f1_out, f2_out, fsub.float_i, creset);
			}
			else {
				fprintf(out_sub_result, "%s\t%lf - %lf = %lf (%sROUNDED: %s%lf)\n%s", cyan, f1_out, f2_out, fsub.float_i, yellow, cyan, fsub_out, creset);
			}
			fprintf(out_sub_result, "%s\t(0x%lx) - (0x%lx) = (0x%lx)\n\n%s", cyan, f1.int_i, f2.int_i, myfloat_h, creset);
		/************************************************************************************************************************/
		}	else if (choice_type == 2) {
			for (int i=0; i<number_of_floats; i++){
				for (int j=0; j<number_of_floats; j++){
					f1.int_i = i;
					f2.int_i = j;
					hex_to_float(f1, &f1_out);
					hex_to_float(f2, &f2_out);
					fsub.float_i = f1_out - f2_out;
					float_to_hex(fsub, &mysub_h, &fsub_out);
					fprintf(out_sub_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s",
							cyan, 
							float_size/4, f1.int_i, float_size/4, f2.int_i,
							float_size/4, mysub_h,
							creset
					  	  );
				}
			}
		}
	}

	if (op_type == 4) {
		printf(GREEN "\nMULTIPLICATION\n");
	MULTIPLICATION_OPTION:
		printf(      "\nCHOOSE OPTION:\n"
					 "	(1)  SINGLE MULTIPLICATION\n"
					 "	(2)  ALL POSSIBLE MULTIPLICATIONS\n"
					 "\nOPTION: "
					);
		scanf("%d", &choice_type);

		int valid_choice_type = check_input_choice_type();
		if (valid_choice_type == FP_ERROR) {
			goto MULTIPLICATION_OPTION;		
		}

		if (choice_type == 1) {
			printf(GREEN "\nInsert float 1 in hex: " CRESET);
			scanf("%lx", &f1.int_i);
			printf(GREEN "Insert float 2 in hex: " CRESET);
			scanf("%lx", &f2.int_i);
			hex_to_float(f1, &f1_out);
			hex_to_float(f2, &f2_out);
			fmul.float_i = f1_out * f2_out;
			exact = float_to_hex(fmul, &myfloat_h, &fmul_out);
			fprintf(out_mul_result, "%s\n\tf1_out * f2_out = fmul.float_i\n%s",  cyan, creset);
			if (exact == 1) {
				fprintf(out_mul_result, "%s\t%lf * %lf = %lf\n%s", cyan, f1_out, f2_out, fmul.float_i, creset);
			}
			else {
				fprintf(out_mul_result, "%s\t%lf * %lf = %lf (%sROUNDED: %s%lf)\n%s", cyan, f1_out, f2_out, fmul.float_i, yellow, cyan, fmul_out, creset);
			}
			fprintf(out_mul_result, "%s\t(0x%lx) * (0x%lx) = (0x%lx)\n\n%s", cyan, f1.int_i, f2.int_i, myfloat_h, creset);
			/************************************************************************************************************************/
		}   else if (choice_type == 2) {
			for (int i=0; i<number_of_floats; i++){
				for (int j=0; j<number_of_floats; j++){
					f1.int_i = i;
					f2.int_i = j;
					hex_to_float(f1, &f1_out);
					hex_to_float(f2, &f2_out);
					fmul.float_i = f1_out * f2_out;
					float_to_hex(fmul, &mymul_h, &fmul_out);
					fprintf(out_mul_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s",
							cyan, 
							float_size/4, f1.int_i, float_size/4, f2.int_i,
							float_size/4, mymul_h,
							creset
				  	 	 );
				}	
			}
		}
	}

	if (op_type == 5) {
		printf(GREEN "\nDIVISION\n");
	DIVISION_OPTION:
		printf(      "\nCHOOSE OPTION:\n"
					 "	(1)  SINGLE DIVISION\n"
					 "	(2)  ALL POSSIBLE DIVISIONS\n"
					 "\nOPTION: "
					);
		scanf("%d", &choice_type);

		int valid_choice_type = check_input_choice_type();
		if (valid_choice_type == FP_ERROR) {
			goto DIVISION_OPTION;		
		}

		if (choice_type == 1) {
			printf(GREEN "\nInsert float 1 in hex: " CRESET);
			scanf("%lx", &f1.int_i);
			printf(GREEN "Insert float 2 in hex: " CRESET);
			scanf("%lx", &f2.int_i);
			hex_to_float(f1, &f1_out);
			hex_to_float(f2, &f2_out);
			fdiv.float_i = f1_out / f2_out;
			exact = float_to_hex(fdiv, &myfloat_h, &fdiv_out);
			fprintf(out_div_result, "%s\n\tf1_out / f2_out = fdiv.float_i\n%s", cyan, creset);
			if (exact == 1) {
				fprintf(out_div_result, "%s\t%lf / %lf = %lf\n%s", cyan, f1_out, f2_out, fdiv.float_i, creset);
			}
			else {
				fprintf(out_div_result, "%s\t%lf / %lf = %lf (%sROUNDED: %s%lf)\n%s", cyan, f1_out, f2_out, fdiv.float_i, yellow, cyan,  fdiv_out, creset);
			}
			fprintf(out_div_result, "%s\t(0x%lx) / (0x%lx) = (0x%lx)\n\n%s", cyan, f1.int_i, f2.int_i, myfloat_h, creset);
			/************************************************************************************************************************/
		}	else if (choice_type == 2) {
			for (int i=0; i<number_of_floats; i++){
				for (int j=0; j<number_of_floats; j++){
					f1.int_i = i;
					f2.int_i = j;
					hex_to_float(f1, &f1_out);
					hex_to_float(f2, &f2_out);
					fdiv.float_i = f1_out / f2_out;
					float_to_hex(fdiv, &mydiv_h, &fdiv_out);
					fprintf(out_div_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s",
							cyan,
							float_size/4, f1.int_i, float_size/4, f2.int_i,
							float_size/4, mydiv_h,
							creset
					  	  );
				}
			}
		}
	}

	if (op_type == 6) {
		printf(GREEN "\nSQUARE ROOT\n");
	SQRT_OPTION:
		printf(      "\nCHOOSE OPTION:\n"
					 "	(1)  SINGLE SQUARE ROOT\n"
					 "	(2)  ALL POSSIBLE SQUARE ROOTS\n"
					 "\nOPTION: "
					);
		scanf("%d", &choice_type);

		int valid_choice_type = check_input_choice_type();
		if (valid_choice_type == FP_ERROR) {
			goto SQRT_OPTION;		
		}

		if (choice_type == 1) {
			fprintf(out_sqrt_result, "%s\nInsert your float in hex: %s", cyan, creset);
			scanf("%lx", &f1.int_i);
			hex_to_float(f1, &f1_out);
			fsqrt.float_i = sqrt(f1_out);
			exact = float_to_hex(fsqrt, &myfloat_h, &fsqrt_out);
			fprintf(out_sqrt_result, "%s\n\tfsqrt(float_i)\n%s", cyan, creset);
			fprintf(out_sqrt_result, "%s\tfsqrt(%lf) = %lf\n%s", cyan, f1_out, fsqrt.float_i, creset);
			fprintf(out_sqrt_result, "%s\tfsqrt(0x%lx) = (0x%lx)\n\n%s", cyan, f1.int_i, myfloat_h, creset);
		/************************************************************************************************************************/
		}	else if (choice_type == 2) {
			for (int i=0; i<number_of_floats; i++){
				f1.int_i = i;
				hex_to_float(f1, &f1_out);
				fsqrt.float_i = sqrt(f1_out);
				float_to_hex(fsqrt, &myfloat_h, &fsqrt_out);
				fprintf(out_sqrt_result, "%s\t0x%.*lx\t0x%.*lx\n%s", 
						cyan, 
						float_size/4, f1.int_i, float_size/4, myfloat_h,
						creset
				  	  );
			}
		}
	}

	if (op_type == 7) {
		printf(GREEN "\nFUSED MULTIPLY-ADD\n");
	FMA_OPTION:
		printf(      "\nCHOOSE OPTION:\n"
					 "	(1)  SINGLE FUSED MULTIPLY-ADD\n"
					 "	(2)  ALL POSSIBLE FUSED MULTIPLY-ADD\n"
					 "\nOPTION: "
					);
		scanf("%d", &choice_type);

		int valid_choice_type = check_input_choice_type();
		if (valid_choice_type == FP_ERROR) {
			goto FMA_OPTION;		
		}

		if (choice_type == 1) {	
			printf(GREEN "\nInsert float 1 in hex: " CRESET);
			scanf("%lx", &f1.int_i);
			printf(GREEN "Insert float 2 in hex: " CRESET);
			scanf("%lx", &f2.int_i);
			printf(GREEN "Insert float 3 in hex: " CRESET);
			scanf("%lx", &f3.int_i);
			hex_to_float(f1, &f1_out);
			hex_to_float(f2, &f2_out);
			hex_to_float(f3, &f3_out);
			fma.float_i = (f1_out * f2_out) + f3_out;
			exact = float_to_hex(fma, &myfloat_h, &fma_out);
			fprintf(out_fma_result, "%s\n\t(f1_out * f2_out) + f3_out = fma.float_i\n%s", cyan, creset);
			fprintf(out_fma_result, "%s\t(%lf * %lf) + %lf = %lf\n%s", cyan, f1_out, f2_out, f3_out, fma.float_i, creset);
			fprintf(out_fma_result, "%s\t((0x%lx) * (0x%lx)) + (0x%lx) = (0x%lx)\n\n%s", cyan, f1.int_i, f2.int_i, f3.int_i, myfloat_h, creset);
		/************************************************************************************************************************/
		}   else if (choice_type == 2) {
			for (int i=0; i<number_of_floats; i++){
				for (int j=0; j<number_of_floats; j++){
					for (int k=0; k<number_of_floats; k++){
						f1.int_i = i;
						f2.int_i = j;
						f3.int_i = k;
						hex_to_float(f1, &f1_out);
						hex_to_float(f2, &f2_out);
						hex_to_float(f3, &f3_out);
						fma.float_i = (f1_out * f2_out) + f3_out;
						float_to_hex(fma, &myfloat_h, &fma_out);
						fprintf(out_fma_result, "%s\t0x%.*lx\t0x%.*lx\t0x%.*lx\t0x%.*lx\n%s", 
								cyan, 
								float_size/4, f1.int_i, float_size/4, f2.int_i,
								float_size/4, f3.int_i, float_size/4, myfloat_h,
								creset
						  	  );
					}
				}
			}
		}
	}

	if (op_type == 8) {
	  SET_CONV_TYPE:
		printf("\nINSERT CONVERSION TYPE:\n"
					 "	(1) FLOAT_%d_1 to FLOAT_%d_2\n"
					 "	(2) WIDENING\n"
					 "	(3) NARROWING\n"
					 "	(4) FLOAT_%d to INT\n"
					 "	(5) FLOAT_%d to UINT\n"
					 "	(6) INT to FLOAT_%d\n"
					 "	(7) UINT to FLOAT_%d\n"
					 "\nCONV TYPE: ",
						float_size, float_size, float_size, float_size , float_size, float_size
					);
		scanf("%d", &conv_type);

		int valid_conv_type = check_input_conv_type();
		if (valid_conv_type == FP_ERROR) {
			goto SET_CONV_TYPE;		
		}

		if (conv_type == 1) {
		  SET_FLOAT_SIZE_2:
		  	printf("\n\nEnter Second Float Size: ");
		  	scanf("%d", &float_size_2);  
		  	valid_float = check_input_float_2();
		  	if (valid_float == FP_ERROR) {
		  	float_set_2 = 0;
		  	goto SET_FLOAT_SIZE_2;		
		  	}  
		  SET_EXPONENT_SIZE_2:
		  	if (exponent_set_2 == 0) {
		  		printf(PURPLE "Enter exponent size: " CRESET);
		  		scanf("%d", &exponent_size_2);
		  	}  
		  	valid_exponent = check_input_exponent_2();
		  	if (valid_exponent == FP_ERROR) {
		  		exponent_set_2 = 0;
		  		goto SET_EXPONENT_SIZE_2;		
		  	}  
		  SET_MANTISSA_SIZE_2:
		  	if (mantissa_set_2 == 0) {
		  		printf(PURPLE "Enter mantissa size: " CRESET);
		  		scanf("%d", &mantissa_size_2);
		  	}  
		  	valid_mantissa = check_input_mantissa_2();
		  	if (valid_mantissa == FP_ERROR) {
		  		mantissa_set_2 = 0;
		  		goto SET_MANTISSA_SIZE_2;		
		  	}  
		  SET_CUSTOM_BIAS_2:
		  	if (bias_set_2 == 0) {
		  		printf(BLUE "\nDo you want a custom bias? (Y/N) " CRESET);
		  		scanf(" %c", &custom_bias_2);
		  		int valid_bias_2 = check_input_bias_2();
		  		if (valid_bias_2 == FP_ERROR) {
		  			bias_set_2 = 0;
		  			goto SET_CUSTOM_BIAS_2;		
		  		}
		  	}

			upper_bound_2 = pow(2,(pow(2,(double)exponent_size_2)-1-bias_2));
			exp_range_2 = pow(2,exponent_size_2);
			mnt_range_2 = pow(2,mantissa_size_2);

		  CONVERSION_OPTION_1:
		  	printf(      "\nCHOOSE OPTION:\n"
		  				 "	(1)  SINGLE CONVERSION\n"
		  				 "	(2)  ALL POSSIBLE CONVERSIONS\n"
		  				 "\nOPTION: "
		  				);
		  	scanf("%d", &choice_type);  
		  	int valid_choice_type = check_input_choice_type();
		  	if (valid_choice_type == FP_ERROR) {
		  		goto CONVERSION_OPTION_1;		
		  	}  
		  	if (choice_type == 1) {  
				printf(GREEN "\nInsert float 1 in hex: " CRESET);
				scanf("%lx", &f1.int_i);
				hex_to_float(f1, &f1_out);
		  		f2.float_i = f1_out;
		  		exact = float_to_hex_2(f2, &myfloat_h, &f2_out);
				printf(CYAN "\t f_1 = f_2\n");
				printf(CYAN "\t(0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, myfloat_h);
		  
		  	/************************************************************************************************************************/
		  	} else if (choice_type == 2) {
				for (int i=0; i<number_of_floats; i++){
					f1.int_i = i;
					hex_to_float(f1, &f1_out);
					f2.float_i = f1_out;
		  			exact = float_to_hex_2(f2, &myfloat_h, &f2_out);
					printf(CYAN "0x%lx 0x%lx\n" CRESET, f1.int_i, myfloat_h);
		  		}
			}
		}

		if (conv_type == 4 || conv_type == 5) {
			printf("\n\nINSERT INTEGER WIDTH: ");
			scanf("%d", &integer_width);
		}
		
	}

	if (op_type == 9) {
		printf("\nINSERT YOUR FLOAT: ");
		scanf("%lf", &f1.float_i); 
		exact = float_to_hex(f1, &myfloat_h, &f1_out);
		printf(CYAN "HEX = 0x%lx (%lf)\n" CRESET, myfloat_h, f1_out);
	}

	if (op_type == 10) {
		printf("\nINSERT YOUR HEX: ");
		scanf("%lx", &f1.int_i); 
		uint64_t myfloat_h = 0x0;
		hex_to_float(f1, &f1_out);
		printf(CYAN "FLOAT = %lf (0x%lx)\n" CRESET, f1_out, f1.int_i);
	}
	return 0;
};
