
void parse_args(unsigned int argc, char** argv);

void round_fp(uint64_t mantissa_extract, uint64_t sign_extract, double mantissa, double mantissa_middle, double mantissa_lower, double mantissa_upper, uint64_t* mantissa_rounded, uint64_t* exponent_rounded);
void round_fp2(uint64_t mantissa_extract, uint64_t sign_extract, double mantissa, double mantissa_middle, double mantissa_lower, double mantissa_upper, uint64_t* mantissa_rounded, uint64_t* exponent_rounded);
int  float_to_hex(number f, uint64_t* myfloat_h, double* f_out);
int float_to_hex_2(number f, uint64_t* myfloat_h, double* f_out);
void hex_to_float(number f, double* f_out);

int check_input_float();
int check_input_exponent();
int check_input_mantissa();
int check_input_bias();
int check_input_float_2();
int check_input_exponent_2();
int check_input_mantissa_2();
int check_input_bias_2();
int check_input_op_type();
int check_input_arith_type();
int check_input_conv_type();
int check_input_rounding_mode();
int check_input_choice_type();
int check_input_print_answer();
int check_input_int_width();
int check_input_valid_uint();

int is_NaN_inf (number f);
int rand_gen(int lower, int upper);