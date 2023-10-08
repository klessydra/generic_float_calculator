
#define F8_1_SIGN ((uint8_t)1  << 7)
#define F8_2_SIGN ((uint8_t)1  << 7)
#define F16_SIGN  ((uint32_t)1 << 15)
#define F32_SIGN  ((uint32_t)1 << 31)
#define F64_SIGN  ((uint64_t)1 << 63)
#define F128_SIGN ((uint64_t)1 << 127)

float8_1_t f8_1(uint8_t v) { float8_1_t f = {.v = v}; return  f; }
float8_2_t f8_2(uint8_t v) { float8_2_t f = {.v = v}; return  f; }
float16_t  f16(uint16_t v) { float16_t  f = {.v = v}; return  f; }
float32_t  f32(uint32_t v) { float32_t  f = {.v = v}; return  f; }
float64_t  f64(uint64_t v) { float64_t  f = {.v = v}; return  f; }

#define F8_1 1
#define F8_2 2
#define F16  3
#define F32  4
#define F64  5
#define F128 6


#define FADD      1
#define FSUB      2
#define FRSUB     3
#define FMUL      4
#define FDIV      5
#define FREC      6
#define FRDIV     7
#define FSQRT     8
#define FRSQRT    9
#define FMACC     10
#define FNMACC    11
#define FMSAC     12
#define FNMSAC    13
#define FMADD     14
#define FNMADD    15
#define FMSUB     16
#define FNMSUB    17
#define FMIN      18
#define FMAX      19
#define FEQ       20
#define FNE       21
#define FLE       22
#define FLT       23
#define FGE       24
#define FGT       25
#define FCVT_X_F  26
#define FCVT_XU_F 27
#define FCVT_F_X  28
#define FCVT_F_XU 29
#define FCLASS    30

