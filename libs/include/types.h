#include <inttypes.h>

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
	unsigned int error_cnt_op_type;
	unsigned int error_cnt_conv_type;
	unsigned int error_cnt_round_mode;
} error_count;

typedef union{
	double float_i;
	uint64_t int_i;
} number;
