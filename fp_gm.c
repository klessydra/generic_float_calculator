#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

#define BLACK  "\e[1;90m"
#define RED    "\e[1;91m"
#define GREEN  "\e[1;92m"
#define YELLOW "\e[1;93m"
#define BLUE   "\e[1;94m"
#define PURPLE "\e[1;95m"
#define CYAN   "\e[1;96m"
#define WHITE  "\e[1;97m"
#define WHITE  "\e[1;97m"

#define CRESET "\e[0m"


typedef struct {
  unsigned int mantissa;
  unsigned int exponent;
  unsigned int sign;
} generic_float_t;

typedef struct{
	unsigned int error_cnt_float_size;
	unsigned int error_cnt_exponent_size;
	unsigned int error_cnt_mantissa_size;
	unsigned int error_cnt_bias_size;
} error_count;

typedef union{
	double float_i;
	uint64_t int_i;
} number;

void round_fp(int round_type, int mantissa_size, uint64_t mantissa_extract, uint64_t sign_extract, double mantissa, double mantissa_middle, double mantissa_upper, uint64_t* mantissa_rounded, uint64_t* exponent_rounded);
int  float_to_hex(number f, int float_size, int exponent_size, int mantissa_size, int bias, int round_type, uint64_t* myfloat_h, double* f_out);
void hex_to_float(number f, int float_size, int exponent_size, int mantissa_size, int bias, double* f_out);

int main() {

	int float_size    = 0;
	int mantissa_size = 0;
	int exponent_size = 0;

	char custom_bias;
	int  bias          = 0;

	int op_type;
	int conv_type;
	int round_type;
	int integer_width;

	int exact;

	uint64_t myfloat_h  = 0x0;
	uint64_t mysum_h	= 0x0;
	uint64_t mysub_h	= 0x0;
	uint64_t mymul_h	= 0x0;
	uint64_t mydiv_h	= 0x0;

	double   f1_out, f2_out, f3_out, fsum_out, fsub_out, fmul_out, fdiv_out, fsqrt_out, fma_out = 0x0;
	number f1, f2, f3, fsum, fsub, fmul, fdiv, fsqrt, fma;

	error_count err_cnt;
	err_cnt.error_cnt_float_size = 0;
	err_cnt.error_cnt_exponent_size = 0;
	err_cnt.error_cnt_mantissa_size = 0;
	err_cnt.error_cnt_bias_size = 0;

	SET_FLOAT_SIZE:
	printf(PURPLE "\nEnter Float Size: " CRESET);
	scanf("%d", &float_size);

	if (float_size < 4) {
		if (err_cnt.error_cnt_float_size == 1) {
			printf(RED "ERROR: " WHITE "Invalid float size (PLEASE ENTER A FLOAT SIZE > 3)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid float size\n" CRESET);
			err_cnt.error_cnt_float_size++;
		}
		goto SET_FLOAT_SIZE;
	}

	SET_EXPONENT_SIZE:
	printf(PURPLE "Enter exponent size: " CRESET);
	scanf("%d", &exponent_size);

	if (exponent_size < 2 || exponent_size > float_size-2) {
		if (err_cnt.error_cnt_exponent_size == 1) {
			printf(RED "ERROR: " WHITE "Invalid exponent size (PLEASE ENTER AN EXPONENT SIZE > 2 && < FLOAT_SIZE -1)\n" CRESET);
		}
		else {
			printf(RED "ERROR" WHITE "invalid exponent size\n" CRESET);
			err_cnt.error_cnt_exponent_size++;
		}
		goto SET_EXPONENT_SIZE;
	}

	SET_MANTISSA_SIZE:
	printf(PURPLE "Enter mantissa size: " CRESET);
	scanf("%d", &mantissa_size);

	if (mantissa_size <= 0 || exponent_size + mantissa_size != float_size-1) {
		if (err_cnt.error_cnt_mantissa_size == 1) {
			printf(RED "ERROR: " WHITE "Invalid mantissa size (PLEASE ENTER A MANTISSA SIZE > 0 && < (FLOAT_SIZE - EXPONENT_SIZE -1))\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid mantissa size\n" CRESET);
			err_cnt.error_cnt_mantissa_size++;
		}
		goto SET_MANTISSA_SIZE;
	}

	puts("");
    SET_CUSTOM_BIAS:
	printf(BLUE "Do you want a custom bias? (Y/N) " CRESET);
	scanf(" %c", &custom_bias);

	if (custom_bias == 'y' || custom_bias == 'Y') {
    	printf("Enter bias: ");
    	scanf("%d", &bias);
    }
    else if (custom_bias == 'n' || custom_bias == 'N') {
    	printf(BLUE "Defaulting bias to 2^(%d)-1 = " CRESET "%d\n", exponent_size-1, (int)pow(2,exponent_size-1)-1);
    	bias = (int)pow(2,exponent_size-1)-1;
	}
	else {
		bias = '0';
		if (err_cnt.error_cnt_bias_size == 1) {
			printf(RED "ERROR: " WHITE "Unknown answer (PLEASE RESPOND \'Y\' OR \'y\' FOR YES, AND \'N\' OR \'n\' FOR NO)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Unknown answer\n" CRESET);
			err_cnt.error_cnt_bias_size++;
		}
		goto SET_CUSTOM_BIAS;
	}

	//double mantissa_upper_bound = 1.0;
	//for (int i=1; i<=mantissa_size; i++) {
	//	mantissa_upper_bound += 1.0/pow(2,i);
	//}
	//printf(CYAN "mantissa upper bound = %f\n\n" CRESET, mantissa_upper_bound);
	//double num_upper_bound = pow(2,bias)* mantissa_upper_bound;// + mantissa_upper_bound);
	//printf(CYAN "upper bound = %f\n\n" CRESET, num_upper_bound);

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

	if (op_type == 8) {
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

		if (conv_type == 4 || conv_type == 5) {
			printf("\n\nINSERT INTEGER WIDTH: ");
			scanf("%d", &integer_width);
		}
	}

	if (op_type != 1 && op_type != 10) {
		printf("\nINSERT ROUNDING MODE:\n"
					 "	(1)  RNE\n"
					 "	(2)  RTZ\n"
					 "	(3)  RDN\n"
					 "	(4)  RUP\n"
					 "	(5)  RMM\n"
					 "\nROUNDING MODE: "
					);
		scanf("%d", &round_type);
	}

	uint64_t number_of_floats = (uint64_t) pow(2,float_size);

	if (op_type == 0) {
		printf(GREEN "\nFLOAT_%d (1-%d-%d)\n", float_size, exponent_size, mantissa_size);
		printf(GREEN "\n\tOP_A\tOP_B\tADD\tSUB\tMUL\tDIV\n");
		for (int i=0; i<number_of_floats; i++){
			for (int j=0; j<number_of_floats; j++){
				f1.int_i = i;
				f2.int_i = j;
				hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
				hex_to_float(f2, float_size, exponent_size, mantissa_size, bias, &f2_out);
				fsum.float_i = f1_out + f2_out;
				fsub.float_i = f1_out - f2_out;
				fmul.float_i = f1_out * f2_out;
				fdiv.float_i = f1_out / f2_out;
				float_to_hex(fsum, float_size, exponent_size, mantissa_size, bias, round_type, &mysum_h, &fsum_out);
				float_to_hex(fsub, float_size, exponent_size, mantissa_size, bias, round_type, &mysub_h, &fsub_out);
				float_to_hex(fmul, float_size, exponent_size, mantissa_size, bias, round_type, &mymul_h, &fmul_out);
				float_to_hex(fdiv, float_size, exponent_size, mantissa_size, bias, round_type, &mydiv_h, &fdiv_out);
				if (float_size <= 4)
					printf(CYAN "\t0x%.01lx\t0x%.01lx\t0x%.01lx\t0x%.01lx\t0x%.01lx\t0x%.01lx\n" CRESET, f1.int_i, f2.int_i, mysum_h, mysub_h, mymul_h, mydiv_h);
				else if (float_size <= 8)
					printf(CYAN "\t0x%.02lx\t0x%.02lx\t0x%.02lx\t0x%.02lx\t0x%.02lx\t0x%.02lx\n" CRESET, f1.int_i, f2.int_i, mysum_h, mysub_h, mymul_h, mydiv_h);
				else if (float_size <= 16)
					printf(CYAN "\t0x%.04lx\t0x%.04lx\t0x%.04lx\t0x%.04lx\t0x%.04lx\t0x%.04lx\n" CRESET, f1.int_i, f2.int_i, mysum_h, mysub_h, mymul_h, mydiv_h);
				else if (float_size <= 32)
					printf(CYAN "\t0x%.08lx\t0x%.08lx\t0x%.08lx\t0x%.08lx\t0x%.08lx\t0x%.08lx\n" CRESET, f1.int_i, f2.int_i, mysum_h, mysub_h, mymul_h, mydiv_h);
				else if (float_size <= 64)
					printf(CYAN "\t0x%.16lx\t0x%.16lx\t0x%.16lx\t0x%.16lx\t0x%.16lx\t0x%.16lx\n" CRESET, f1.int_i, f2.int_i, mysum_h, mysub_h, mymul_h, mydiv_h);

				//printf(CYAN "\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\t%lx (%lf)\n" CRESET, 
				//		f1.int_i, f1_out, f2.int_i, f2_out, mysum_h, fsum_out, mysub_h, fsub_out, mymul_h, fmul_out, mydiv_h, fdiv_out);
			}
		}
	}

	if (op_type == 1) {
		printf(GREEN "\nFLOAT_%d (1-%d-%d)\n", float_size, exponent_size, mantissa_size);
		for (int i=0; i<number_of_floats; i++){
			f1.int_i = i;
			hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
			printf(CYAN "\t0x%lx (%E)\n" CRESET, f1.int_i, f1_out);
		}
	}

	if (op_type == 2) {
		printf(GREEN "\nADDITION\n");
		printf(GREEN "\nInsert float 1 in hex: " CRESET);
		scanf("%lx", &f1.int_i);
		printf(GREEN "Insert float 2 in hex: " CRESET);
		scanf("%lx", &f2.int_i);
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		hex_to_float(f2, float_size, exponent_size, mantissa_size, bias, &f2_out);
		fsum.float_i = f1_out + f2_out;
		exact = float_to_hex(fsum, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &fsum_out);
		printf(CYAN "\n\tf1_out + f2_out = fsum.float_i\n" CRESET);
		if (exact == 1) {
			printf(CYAN "\t%lf + %lf = %lf\n" CRESET, f1_out, f2_out, fsum.float_i);
		}
		else {
			printf(CYAN "\t%lf + %lf = %lf (" YELLOW "ROUNDED: " CYAN "%lf)\n" CRESET, f1_out, f2_out, fsum.float_i, fsum_out);
		}
		printf(CYAN "\t(0x%lx) + (0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, f2.int_i, myfloat_h);
	}

	if (op_type == 3) {
		printf(GREEN "\nSUBTRACTION\n");
		printf(GREEN "\nInsert float 1 in hex: " CRESET);
		scanf("%lx", &f1.int_i);
		printf(GREEN "Insert float 2 in hex: " CRESET);
		scanf("%lx", &f2.int_i);
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		hex_to_float(f2, float_size, exponent_size, mantissa_size, bias, &f2_out);
		fsub.float_i = f1_out - f2_out;
		exact = float_to_hex(fsub, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &fsub_out);
		printf(CYAN "\n\tf1_out - f2_out = fsub.float_i\n" CRESET);
		if (exact == 1) {
			printf(CYAN "\t%lf - %lf = %lf\n" CRESET, f1_out, f2_out, fsub.float_i);
		}
		else {
			printf(CYAN "\t%lf - %lf = %lf (" YELLOW "ROUNDED: " CYAN "%lf)\n" CRESET, f1_out, f2_out, fsub.float_i, fsub_out);
		}
		printf(CYAN "\t(0x%lx) - (0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, f2.int_i, myfloat_h);
	}

	if (op_type == 4) {
		printf(GREEN "\nMULTIPLICATION\n");
		printf(GREEN "\nInsert float 1 in hex: " CRESET);
		scanf("%lx", &f1.int_i);
		printf(GREEN "Insert float 2 in hex: " CRESET);
		scanf("%lx", &f2.int_i);
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		hex_to_float(f2, float_size, exponent_size, mantissa_size, bias, &f2_out);
		fmul.float_i = f1_out * f2_out;
		exact = float_to_hex(fmul, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &fmul_out);
		printf(CYAN "\n\tf1_out * f2_out = fmul.float_i\n" CRESET);
		if (exact == 1) {
			printf(CYAN "\t%lf * %lf = %lf\n" CRESET, f1_out, f2_out, fmul.float_i);
		}
		else {
			printf(CYAN "\t%lf * %lf = %lf (" YELLOW "ROUNDED: " CYAN "%lf)\n" CRESET, f1_out, f2_out, fmul.float_i, fmul_out);
		}
		printf(CYAN "\t(0x%lx) * (0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, f2.int_i, myfloat_h);
	}


	if (op_type == 5) {
		printf(GREEN "\nDIVISION\n");
		printf(GREEN "\nInsert float 1 in hex: " CRESET);
		scanf("%lx", &f1.int_i);
		printf(GREEN "Insert float 2 in hex: " CRESET);
		scanf("%lx", &f2.int_i);
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		hex_to_float(f2, float_size, exponent_size, mantissa_size, bias, &f2_out);
		fdiv.float_i = f1_out / f2_out;
		exact = float_to_hex(fdiv, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &fdiv_out);
		printf(CYAN "\n\tf1_out / f2_out = fdiv.float_i\n" CRESET);
		if (exact == 1) {
			printf(CYAN "\t%lf / %lf = %lf\n" CRESET, f1_out, f2_out, fdiv.float_i);
		}
		else {
			printf(CYAN "\t%lf / %lf = %lf (" YELLOW "ROUNDED: " CYAN "%lf)\n" CRESET, f1_out, f2_out, fdiv.float_i, fdiv_out);
		}
		printf(CYAN "\t(0x%lx) / (0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, f2.int_i, myfloat_h);
	}

	if (op_type == 6) {
		printf(GREEN "\nSQUARE ROOT\n");
		printf(GREEN "\nInsert your float in hex: " CRESET);
		scanf("%lx", &f1.int_i);
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		fsqrt.float_i = sqrt(f1_out);
		exact = float_to_hex(fsqrt, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &fsqrt_out);
		printf(CYAN "\n\tfsqrt(float_i)\n" CRESET);
		printf(CYAN "\tfsqrt(%lf) = %lf\n" CRESET, f1_out, fsqrt.float_i);
		printf(CYAN "\tfsqrt(0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, myfloat_h);
	}

	if (op_type == 7) {
		printf(GREEN "\nFUSED MULTIPLY-ADD\n");
		printf(GREEN "\nInsert float 1 in hex: " CRESET);
		scanf("%lx", &f1.int_i);
		printf(GREEN "Insert float 2 in hex: " CRESET);
		scanf("%lx", &f2.int_i);
		printf(GREEN "Insert float 3 in hex: " CRESET);
		scanf("%lx", &f3.int_i);
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		hex_to_float(f2, float_size, exponent_size, mantissa_size, bias, &f2_out);
		hex_to_float(f3, float_size, exponent_size, mantissa_size, bias, &f3_out);
		fma.float_i = (f1_out * f2_out) + f3_out;
		exact = float_to_hex(fma, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &fma_out);
		printf(CYAN "\n\t(f1_out * f2_out) + f3_out = fma.float_i\n" CRESET);
		printf(CYAN "\t(%lf * %lf) + %lf = %lf\n" CRESET, f1_out, f2_out, f3_out, fma.float_i);
		printf(CYAN "\t((0x%lx) * (0x%lx)) + (0x%lx) = (0x%lx)\n\n" CRESET, f1.int_i, f2.int_i, f3.int_i, myfloat_h);
	}

	if (op_type == 9) {
		printf("\nINSERT YOUR FLOAT: ");
		scanf("%lf", &f1.float_i); 
		exact = float_to_hex(f1, float_size, exponent_size, mantissa_size, bias, round_type, &myfloat_h, &f1_out);
		printf(CYAN "HEX = 0x%lx (%lf)\n" CRESET, myfloat_h, f1_out);
	}

	if (op_type == 10) {
		printf("\nINSERT YOUR HEX: ");
		scanf("%lx", &f1.int_i); 
		uint64_t myfloat_h = 0x0;
		hex_to_float(f1, float_size, exponent_size, mantissa_size, bias, &f1_out);
		printf(CYAN "FLOAT = %lf (0x%lx)\n" CRESET, f1_out, f1.int_i);
	}

	return 0;
};


void hex_to_float(number f, int float_size, int exponent_size, int mantissa_size, int bias, double* f_out) {
	const uint64_t lsb_en = 1;
	int64_t sign_extract     = 0;
	int64_t exponent_extract = 0;
	int64_t mantissa_extract = 0;
	double   mantissa_extract_f = 0;
	int64_t mantissa_en = pow(2,mantissa_size)-1; 
	int64_t exponent_en = pow(2,exponent_size)-1; 
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
			if (exponent_extract == (int) pow(2,exponent_size)-1) {
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
			if (exponent_extract == (int) pow(2,exponent_size)-1) {
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

int float_to_hex(number f, int float_size, int exponent_size, int mantissa_size, int bias, int round_type, uint64_t* myfloat_h, double* f_out) {
	const uint64_t lsb_en = 1;
	uint64_t mantissa_en = pow(2,mantissa_size)-1; 
	uint64_t exponent_en = pow(2,exponent_size)-1; 
	uint64_t exponent_extract = 0;
	uint64_t mantissa_extract = 0;
	uint64_t mantissa_rounded = 0;
	uint64_t exponent_rounded = 0;
	double   denormalized = 0;
	double   normalized   = 0;
	double mantissa_rounded_f;
	uint64_t sign_extract = (f.int_i >> 63) & lsb_en;
	double mantissa_lookup = 0.0;
	double mantissa_lower;
	double mantissa_upper;
	double mantissa_middle;
	int exact = 0;
	if (isnan(f.float_i)) {
		// Generate a qnan (Quiet NaN)
		mantissa_rounded = pow(2,mantissa_size-1);
		exponent_rounded = pow(2,exponent_size)-1; 
		*myfloat_h = (sign_extract << float_size-1) + (exponent_rounded << mantissa_size) + mantissa_rounded;
	}
	else if (fabs(f.float_i) >= pow(2,(pow(2,(double)exponent_size)-1-bias))) {
		mantissa_rounded = 0;
		exponent_rounded = pow(2,exponent_size)-1; 
		*myfloat_h = (sign_extract << float_size-1) + (exponent_rounded << mantissa_size) + mantissa_rounded;
		//printf(CYAN "HERE\n");
	}
	else if (fabs(f.float_i) == 0) { // if input float is 0 
		*myfloat_h = sign_extract << float_size-1;
		//printf(CYAN "HERE2\n");
	}
	else {
		for (int i=-(bias+mantissa_size-1); i<=bias; i++) {
			if (fabs(f.float_i) >= pow(2,i) && fabs(f.float_i) < pow(2,i+1)) {
				//printf(CYAN "EXPONENT: %d\n", i);
				double mantissa;
				if (i < -(bias-1)) {  // denormalized exponent
					exponent_extract = 0;
					mantissa = fabs(f.float_i) * pow(2,bias-1);
					denormalized = 1;
					normalized   = 0;
				}
				else {
					exponent_extract = i + bias;
					mantissa = fabs(f.float_i) / pow(2,i);
					denormalized = 0;
					normalized   = 1;
				}
				exponent_rounded = exponent_extract;
				//printf(CYAN "MANTISSA: %f\n", mantissa);
				mantissa_lower = mantissa_lookup;
				mantissa_upper = 1.0 / pow(2,mantissa_size);
				mantissa_middle = (mantissa_lower + mantissa_upper) / 2;
				for (int j=1; j<=(int)pow(2,mantissa_size); j++) {
					mantissa_lower = normalized + mantissa_lookup;
					mantissa_lookup += 1.0 / pow(2,mantissa_size); 
					mantissa_upper = normalized + mantissa_lookup;
					mantissa_middle = (mantissa_lower + mantissa_upper) / 2;
					if (mantissa == mantissa_upper || mantissa == mantissa_lower) {
						exact = 1;
					}
					if (mantissa <= mantissa_upper) {
						round_fp(round_type, mantissa_size, mantissa_extract, sign_extract, mantissa, mantissa_middle, mantissa_upper, &mantissa_rounded, &exponent_rounded);
						break;
					}
					mantissa_extract += 1;
				}
				//printf(CYAN "mnt_lower= %lf, mnt_upper = %lf, mnt_middle = %lf \n", mantissa_lower, mantissa_upper, mantissa_middle);
				//printf(CYAN "exp_rounded= %ld\n", exponent_rounded);
				//printf(CYAN "mnt_rounded= %lx\n", mantissa_rounded);
				//printf(CYAN "mantissa_size= %d\n", mantissa_size);
				*myfloat_h = (sign_extract << float_size-1) + ((exponent_rounded & exponent_en) << mantissa_size) + (mantissa_rounded & mantissa_en);
			}
		}
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
		if (fabs(*f_out) >= pow(2,(pow(2,(double)exponent_size)-1-bias))) {
			*f_out = -INFINITY;
		}
	}
	else {
		*f_out =  pow(2, (double)exponent_rounded-(bias-denormalized)) * mantissa_rounded_f;
		if (fabs(*f_out) >= pow(2,(pow(2,(double)exponent_size)-1-bias))) {
			*f_out = INFINITY;
		}
	}
	return exact;
}

void round_fp(int round_type, int mantissa_size, uint64_t mantissa_extract, uint64_t sign_extract, double mantissa, double mantissa_middle, double mantissa_upper, uint64_t* mantissa_rounded, uint64_t* exponent_rounded) {
	const uint64_t lsb_en = 1;
	uint64_t nearest_even_shift;
	if ((mantissa_extract + 1) & lsb_en == 1) {  
		nearest_even_shift = -1;
	}
	else {
		nearest_even_shift = 1;
	}
	if (round_type == 1) { // RNE
		if (mantissa == mantissa_middle) {
			if (mantissa_extract == (int)(pow(2,mantissa_size)-1.0) && nearest_even_shift == 1) {
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
			if (mantissa_extract == (int)(pow(2,mantissa_size)-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;
		}
		else { // mantissa < mantissa_middle
			*mantissa_rounded = mantissa_extract;
		}
	}
	else if (round_type == 2) { // RTZ
		if (mantissa == mantissa_upper) {  // take the upper bound if the mantissa is equal to it, else take always the lower bound
			*mantissa_rounded = mantissa_extract + 1;	
		}
		else {
			*mantissa_rounded = mantissa_extract;
		}
	}
	else if (round_type == 3) { // RDN
		if (sign_extract == 0) {
			if (mantissa == mantissa_upper) {  // take the upper bound if the mantissa is equal to it, else take always the lower bound
				*mantissa_rounded = mantissa_extract + 1;	
			}
			else {
				*mantissa_rounded = mantissa_extract;
			}
		}
		else {
			if (mantissa_extract == (int)(pow(2,mantissa_size)-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;
		}		
	}
	else if (round_type == 4) { // RUP
		if (sign_extract == 0) {
			if (mantissa_extract == (int)(pow(2,mantissa_size)-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;
		}
		else {
			if (mantissa == mantissa_upper) {  // take the upper bound if the mantissa is equal to it, else take always the lower bound
				*mantissa_rounded = mantissa_extract + 1;	
			}
			else {
				*mantissa_rounded = mantissa_extract;
			}
		}		
	}
	else if (round_type == 5) { // RMM
		if (mantissa >= mantissa_middle) {
			if (mantissa_extract == (int)(pow(2,mantissa_size)-1.0)) {
				*exponent_rounded += 1;
			}
			*mantissa_rounded = mantissa_extract + 1;
		}
		else {
			*mantissa_rounded = mantissa_extract;
		}
	}
}