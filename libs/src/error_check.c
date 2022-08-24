#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "vars.h"
#include "defs.h"

int check_input_float() {
	static unsigned int error_cnt_float_size = 0;
	if (float_size < 4) { // 3-bit custom floats and smaller aren't allowed
		if (error_cnt_float_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_float_size > 0) {
			printf(RED "ERROR: " WHITE "Invalid float size (PLEASE ENTER A FLOAT SIZE > 3)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid float size\n" CRESET);
		}
		error_cnt_float_size++;
		printf("\n");
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_float_2() {
	static unsigned int error_cnt_float_size = 0;
	if (float_size_2 < 4) { // 3-bit custom floats and smaller aren't allowed
		if (error_cnt_float_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_float_size > 0) {
			printf(RED "ERROR: " WHITE "Invalid float size (PLEASE ENTER A FLOAT SIZE > 3)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid float size\n" CRESET);
		}
		error_cnt_float_size++;
		printf("\n");
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_exponent() {
	static unsigned int error_cnt_exponent_size = 0;
	if (exponent_size < 2 || exponent_size > float_size-2) {
		if (error_cnt_exponent_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_exponent_size > 0) {
			printf(RED "ERROR: " WHITE "Invalid exponent size (PLEASE ENTER AN EXPONENT SIZE > 1 && < FLOAT_SIZE -1)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid exponent size\n" CRESET);
		}
		error_cnt_exponent_size++;
		printf("\n");
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_exponent_2() {
	static unsigned int error_cnt_exponent_size = 0;
	if (exponent_size_2 < 2 || exponent_size_2 > float_size_2-2) {
		if (error_cnt_exponent_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_exponent_size > 0) {
			printf(RED "ERROR: " WHITE "Invalid exponent size (PLEASE ENTER AN EXPONENT SIZE > 1 && < FLOAT_SIZE -1)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid exponent size\n" CRESET);
		}
		error_cnt_exponent_size++;
		printf("\n");
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_mantissa() {
	static unsigned int error_cnt_mantissa_size = 0;
	if (mantissa_size <= 0 || exponent_size + mantissa_size != float_size-1) {
		if (error_cnt_mantissa_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_mantissa_size > 0) {
			printf(RED "ERROR: " WHITE "Invalid mantissa size (PLEASE ENTER A MANTISSA SIZE > 0 && < (FLOAT_SIZE - EXPONENT_SIZE -1))\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid mantissa size\n" CRESET);
		}
		error_cnt_mantissa_size++;
		printf("\n");
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_mantissa_2() {
	static unsigned int error_cnt_mantissa_size = 0;
	if (mantissa_size_2 <= 0 || exponent_size_2 + mantissa_size_2 != float_size_2-1) {
		if (error_cnt_mantissa_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_mantissa_size > 0) {
			printf(RED "ERROR: " WHITE "Invalid mantissa size (PLEASE ENTER A MANTISSA SIZE > 0 && < (FLOAT_SIZE - EXPONENT_SIZE -1))\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid mantissa size\n" CRESET);
		}
		error_cnt_mantissa_size++;
		printf("\n");
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_bias() {
	static unsigned int error_cnt_bias_size = 0;
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
		if (error_cnt_bias_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}
		if (error_cnt_bias_size > 0) {
			printf(RED "ERROR: " WHITE "Unknown answer (PLEASE RESPOND \'Y\' OR \'y\' FOR YES, AND \'N\' OR \'n\' FOR NO)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Unknown answer\n" CRESET);
		}
		error_cnt_bias_size++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_bias_2() {
	static unsigned int error_cnt_bias_size = 0;
	if (custom_bias_2 == 'y' || custom_bias_2 == 'Y') {
    	printf("Enter bias: ");
    	scanf("%d", &bias_2);
    }
    else if (custom_bias_2 == 'n' || custom_bias_2 == 'N') {
    	printf(BLUE "Defaulting bias to 2^(%d)-1 = " CRESET "%d\n", exponent_size_2-1, (int)pow(2,exponent_size_2-1)-1);
    	bias_2 = (int)pow(2,exponent_size_2-1)-1;
	}
	else {
		bias_2 = '0';
		if (error_cnt_bias_size == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}
		if (error_cnt_bias_size > 0) {
			printf(RED "ERROR: " WHITE "Unknown answer (PLEASE RESPOND \'Y\' OR \'y\' FOR YES, AND \'N\' OR \'n\' FOR NO)\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Unknown answer\n" CRESET);
		}
		error_cnt_bias_size++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_op_type() {
	static unsigned int error_cnt_op_type = 0;
	if (op_type < 0 || op_type > 10) {
		if (error_cnt_op_type == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_op_type > 0) {
			printf(RED "ERROR: " WHITE "Invalid op type (PLEASE ENTER A VALID OP TYPE NUMBER [0-10]\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid op type\n" CRESET);
		}
		error_cnt_op_type++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_choice_type() {
	static unsigned int error_cnt_choice_type = 0;
	if (choice_type < 1 || choice_type > 2) {
		if (error_cnt_choice_type == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_choice_type > 0) {
			printf(RED "ERROR: " WHITE "Invalid choice (PLEASE ENTER A VALID CHOICE NUMBER [1-2]\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid choice\n" CRESET);
		}
		error_cnt_choice_type++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}


int check_input_conv_type() {
	static unsigned int error_cnt_conv_type = 0;
	if (conv_type < 1 || conv_type > 5) {
		if (error_cnt_conv_type == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_conv_type > 0) {
			printf(RED "ERROR: " WHITE "Invalid conversion type (PLEASE ENTER A VALID CONV TYPE NUMBER [1-5]\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid conversion type\n" CRESET);
		}
		error_cnt_conv_type++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_rounding_mode() {
	static unsigned int error_cnt_round_mode = 0;
	if (round_mode < 1 || round_mode > 5) {
		if (error_cnt_round_mode == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_round_mode > 0) {
			printf(RED "ERROR: " WHITE "Invalid rounding mode (PLEASE ENTER A VALID ROUND MODE NUMBER [1-5]\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid rounding mode\n" CRESET);
		}
		error_cnt_round_mode++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_int_width() {
	static unsigned int error_cnt_int_width = 0;
	if (integer_width < 2) {
		if (error_cnt_int_width == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_int_width > 0) {
			printf(RED "ERROR: " WHITE "Invalid integer width (PLEASE ENTER A VALID INTEGER WIDTH NUMBER [>0]\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid integer width\n" CRESET);
		}
		error_cnt_int_width++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}

int check_input_valid_uint() {
	static unsigned int error_cnt_valid_uint = 0;
	if (int_input < 0) {
		if (error_cnt_valid_uint == 10) {
			printf(RED "ERROR: " WHITE "Too many invalid trials, exiting! \n" CRESET);
			exit(-1);
		}		
		if (error_cnt_valid_uint > 0) {
			printf(RED "ERROR: " WHITE "Invalid unsigned integer (PLEASE ENTER A VALID UNSIGNED INTEGER [>=0]\n" CRESET);
		}
		else {
			printf(RED "ERROR: " WHITE "Invalid unsigned integer\n" CRESET);
		}
		error_cnt_valid_uint++;
		return FP_ERROR;
	}
	return FP_SUCCESS;
}
