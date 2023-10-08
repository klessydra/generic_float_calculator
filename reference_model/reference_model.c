#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>
#include <fenv.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"
#include "op_defines.h"

#define BLACK  "\e[1;90m"
#define RED    "\e[1;91m"
#define GREEN  "\e[1;92m"
#define YELLOW "\e[1;93m"
#define BLUE   "\e[1;94m"
#define PURPLE "\e[1;95m"
#define CYAN   "\e[1;96m"
#define WHITE  "\e[1;97m"

#define CRESET "\e[0m"

FILE *out1_result;

char* black;
char* red;
char* green;
char* yellow;
char* blue;
char* purple;
char* cyan;
char* white;
char* creset;

#define RET_SUCCESS  1
#define RET_FAIL    -1

char* toBinary(int num, int bitCount);
//long long int toBinary(int num);
int64_t rand_gen(int64_t lower, int64_t upper);

void* src1;
void* src2;
void* src3;
void* res;

void float_type(char* op_type);
int op_index(char* op_type);
int check_op();

char* FLAGS;
char file_print = 'N';

void* op(void* src1, void* src2, void* src3, int en_print);

char* op_type; // has the string that defines the op_type
int op_type_int = 0;  // has the integer representation of op_type, this would relieve us from constatntly comparing strings
int f_type = 0; // has the float type set
int f_size; // the following sizes are supported, 8, 16, 32, 64, 128 
int num_tabs;
char format[100];

typedef struct{
  uint64_t sign;
  uint64_t exponent;
  uint64_t mantissa;
  unsigned int implicit : 1;  // 3-bits 
  double   mantissa_f; // float representtion of the mantissa with the implicit bit
  double   fp;
} float_rep;


unsigned int exponent_size;
unsigned int mantissa_size;
double exp_range;
double mnt_range;

uint64_t operand_count;
uint64_t mantissa_en;
uint64_t exponent_en;

int64_t bias;

float_rep src1_f;
float_rep src2_f;
float_rep src3_f;
float_rep res_f;

int64_t f_min;
int64_t f_max;
uint64_t f_maxu;

int main(int argc, char **argv) {

  op_type = argv[1];

  unsigned int round_mode;
  char* round_mod;
  unsigned int round_set = 0;

  unsigned int random_op_count = 0;
  unsigned int do_random = 0;
  unsigned int do_all = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i] ,"-r") == 0) {
      if (i+1 <= argc) {
        i++;
        round_mode = strtol(argv[i], NULL, 10);
        if (round_mode == 1) {
           round_mod = "RNE";
        }
        else if (round_mode == 2) {
           round_mod = "RTZ";
        }
        else if (round_mode == 3) {
           round_mod = "RDN";
        }
        else if (round_mode == 4) {
           round_mod = "RUP";
        }
        else if (round_mode == 5) {
           round_mod = "RMM";
        }
        else {
          printf(RED "\nERROR" CRESET": Unsupported rounding mode, choose a number from 0-4\n\n");
          exit(1);
        }
        round_set = 1;
      }
      else {
        printf("ERROR: No Argument for rounding mode set\n");
      }
    }
    else if (strcmp(argv[i] ,"--all") == 0) {
      do_all = 1;
      if (do_random == 1) {
        printf("ERROR: Cannot do \"all\" operations, since \"random\" mode is already set, \n");
      }
    }
    else if (strcmp(argv[i] ,"--random") == 0) {
      do_random = 1;
      if (do_all == 1) {
        printf("ERROR: Cannot do \"random\" operations, since \"all\" mode is already set, \n");
      }
      if (i+1 < argc) {
        i++;
        random_op_count = strtol(argv[i], NULL, 10);
        if (random_op_count > 0) {
          printf("Random mode set, number of ops = " GREEN "%d\n" CRESET, random_op_count);
        }
        else {
          printf("Invalid number of random ops\n");
        }
      }
      else {
        printf("Random mode set\n" "NOTE: No Argument for random set, setting number of random operations to 1000\n");
        random_op_count = 1000;
      }
    }
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
        printf(WHITE
          "USAGE: reference_model [OPERATION] [INPUTS] [ARGUMENT]\n"
          "Floating Point Reference model\n\n"
          "Operations supported:\n"
          "       fadd,     fsub,      frsub,    fmul,      fdiv,   frdiv,\n"
          "       fsqrt,    frsqrt,    fmacc,    fnmacc,    fmsac,  fnmsac,\n"
          "       fmadd,    fnamdd,    fmsub,    fnmsub,    fmin,   fmax,\n"
          "       feq,      fne,       fle,      flt,       fge,    fgt,\n"
          "       fcvt.x.f, fcvt.xu.f, fcvt.f.x, fcvt.f.xu, fclass\n"
          "\n"
          "Usage Example 1: ./reference_model fadd8_1 0x30 0x55 -r 3\n"
          "Usage Example 2: ./reference_model fmsac8_2 0x30 0x55 0x86\n"
          "Usage Example 3: ./reference_model fcvt.f.xu.64 5\n"
          "Usage Example 4: ./reference_model fmul16 --random\n"
          "Usage Example 5: ./reference_model frdiv32--random 50\n"
          "Usage Example 6: ./reference_model fsqrt--all\n"
          "\n" CRESET
          "\nProgram Options: \n"
          "  -r, --round_mode <arg>    \n"
          "                            Sets the round mode \n"
          "                                (1)  RNE\n"
          "                                (2)  RTZ\n"
          "                                (3)  RDN\n"
          "                                (4)  RUP\n"
          "                                (5)  RMM\n"
          "  --random <op_count>\n"
          "                            radomises the input of the operationn\n"
          "                            when set with no arg, 1000 random will be done\n\n"
          "  --all \n"
          "                            Perform all combination of inputs on the selected operation\n\n"
          "\n");
        exit(0);
    }
  }

  op_type_int = op_index(op_type);

  if (round_set == 1) {
    printf("Round mode set to " GREEN "%s\n" CRESET, round_mod);
  }
  else {
    printf("No eounding mode set, defaulting to " GREEN "RNE\n" CRESET);
    round_mode = 1;
  }
  round_mode--; // because the round modes in "softfloat_roundingMode" start from 0

  black  = malloc(sizeof BLACK);
  red    = malloc(sizeof RED);
  green  = malloc(sizeof GREEN);
  yellow = malloc(sizeof YELLOW);
  blue   = malloc(sizeof BLUE);
  purple = malloc(sizeof PURPLE);
  cyan   = malloc(sizeof CYAN);
  white  = malloc(sizeof WHITE);
  creset = malloc(sizeof CRESET);

  if (file_print == 'Y' || file_print == 'y') {
    out1_result;
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
    out1_result = stdout;
    black         = BLACK;
    red           = RED;
    green         = GREEN;
    yellow        = YELLOW;
    blue          = BLUE;
    purple        = PURPLE;
    cyan          = CYAN;
    white         = WHITE;
    creset        = CRESET;
  }

  printf("\n");


    if (f_type == F8_1) {
      src1 = malloc(sizeof(float8_1_t));
      src2 = malloc(sizeof(float8_1_t));
      src3 = malloc(sizeof(float8_1_t));
      res  = malloc(sizeof(float8_1_t));
      if (argc >= 3) *((uint8_t*)src1) = strtoul(argv[2], NULL, 16);
      if (argc >= 4) *((uint8_t*)src2) = strtoul(argv[3], NULL, 16);
      if (argc >= 5) *((uint8_t*)src3) = strtoul(argv[4], NULL, 16);
    }
    else if (f_type == F8_2) {
      src1 = malloc(sizeof(float8_2_t));
      src2 = malloc(sizeof(float8_2_t));
      src3 = malloc(sizeof(float8_2_t));
      res  = malloc(sizeof(float8_2_t));
      if (argc >= 3) *((uint8_t*)src1) = strtoul(argv[2], NULL, 16);
      if (argc >= 4) *((uint8_t*)src2) = strtoul(argv[3], NULL, 16);
      if (argc >= 5) *((uint8_t*)src3) = strtoul(argv[4], NULL, 16);
    }
    else if (f_type == F16) {
      src1 = malloc(sizeof(float16_t));
      src2 = malloc(sizeof(float16_t));
      src3 = malloc(sizeof(float16_t));
      res  = malloc(sizeof(float16_t));
      if (argc >= 3) *((uint16_t*)src1) = strtoul(argv[2], NULL, 16);
      if (argc >= 4) *((uint16_t*)src2) = strtoul(argv[3], NULL, 16);
      if (argc >= 5) *((uint16_t*)src3) = strtoul(argv[4], NULL, 16);
    }
    else if (f_type == F32) {
      src1 = malloc(sizeof(float32_t));
      src2 = malloc(sizeof(float32_t));
      src3 = malloc(sizeof(float32_t));
      res  = malloc(sizeof(float32_t));
      if (argc >= 3) *((uint32_t*)src1) = strtoul(argv[2], NULL, 16);
      if (argc >= 4) *((uint32_t*)src2) = strtoul(argv[3], NULL, 16);
      if (argc >= 5) *((uint32_t*)src3) = strtoul(argv[4], NULL, 16);
    }
    else if (f_type == F64) {
      src1 = malloc(sizeof(float64_t));
      src2 = malloc(sizeof(float64_t));
      src3 = malloc(sizeof(float64_t));
      res  = malloc(sizeof(float64_t));
      if (argc >= 3) *((uint64_t*)src1) = strtoul(argv[2], NULL, 16);
      if (argc >= 4) *((uint64_t*)src2) = strtoul(argv[3], NULL, 16);
      if (argc >= 5) *((uint64_t*)src3) = strtoul(argv[4], NULL, 16);
    }

    if (do_all) { // do all possible combinations
      printf("src1%ssrc2%sres%sFLAGS\n", format, format, format);
      if (operand_count == 1) {
        for (int i = 0; i <= f_maxu; i++) {
            *(uint64_t*)src1 = i;
            res = op(src1, src2, src3, 0);
            if (operand_count == 1) printf("%.*lx\t%.*lx\t(%s)\n", f_size/4, *((uint64_t*)src1), f_size/4, *((uint64_t*)res), FLAGS);
            softfloat_exceptionFlags = 0;
        }
      }
      else if (operand_count == 2) {
        for (int i = 0; i <= f_maxu; i++) {
          for (int j = 0; j <= f_maxu; j++) {
            *(uint64_t*)src1 = i;
            *(uint64_t*)src2 = j;
            res = op(src1, src2, src3, 0);
            printf("%.*lx\t%.*lx\t%.*lx\t(%s)\n", f_size/4, *((uint64_t*)src1), f_size/4, *((uint64_t*)src2), f_size/4, *((uint64_t*)res), FLAGS);
            softfloat_exceptionFlags = 0;
          }
        }
      }
      else if (operand_count == 3) {
        for (int i = 0; i <= f_maxu; i++) {
          for (int j = 0; j <= f_maxu; j++) {
            for (int k = 0; k <= f_maxu; k++) {
            *(uint64_t*)src1 = i;
            *(uint64_t*)src2 = j;
            *(uint64_t*)src3 = k;
              res = op(src1, src2, src3, 0);
              printf("%.*lx\t%.*lx\t%.*lx\t%.*lx\t(%s)\n", f_size/4, *((uint64_t*)src1), f_size/4, *((uint64_t*)src2), f_size/4, *((uint64_t*)src3), f_size/4, *((uint64_t*)res), FLAGS);
              softfloat_exceptionFlags = 0;
            }
          }
        }
      }
    }
    else if (do_random) { // do a random set of operation
      printf("src1%ssrc2%sres%sFLAGS\n", format, format, format);
      for (int i = 0; i < random_op_count; i++) {
        *(uint64_t*)src1 = rand_gen(0, f_maxu);
        *(uint64_t*)src2 = rand_gen(0, f_maxu);
        *(uint64_t*)src3 = rand_gen(0, f_maxu);
        res = op(src1, src2, src3, 0);
          if (operand_count == 1) printf("%.*lx\t%.*lx\t(%s)\n", f_size/4, *((uint64_t*)src1), f_size/4, *((uint64_t*)res), FLAGS);
          if (operand_count == 2) printf("%.*lx\t%.*lx\t%.*lx\t(%s)\n", f_size/4, *((uint64_t*)src1), f_size/4, *((uint64_t*)src2), f_size/4, *((uint64_t*)res), FLAGS);
          if (operand_count == 3) printf("%.*lx\t%.*lx\t%.*lx\t%.*lx\t(%s)\n", f_size/4, *((uint64_t*)src1), f_size/4, *((uint64_t*)src2), f_size/4, *((uint64_t*)src3), f_size/4, *((uint64_t*)res), FLAGS);
        softfloat_exceptionFlags = 0;
      }
    }
    else { // do a single operation
      softfloat_roundingMode = round_mode;
      res = op(src1, src2, src3, 1);
    }
/*

    // f32_to_ui32
    //float32_t src_i32 = { .v = 0xc0600000 };
    //int_fast32_t res_i32 = f32_to_ui32(src_i32, 0, true);
    //printf("src_f32 = %x, res_i32 = %x\n", src_i32, res_i32);
    //printf("softfloat_exceptionFlags = 0x%x\n", softfloat_exceptionFlags);
    //softfloat_exceptionFlags = 0;

    // bf16_to_f8
    // Iterate over all possible input combinations
    //for (int i = 0; i <= UINT16_MAX; i++) {
    //    float16_t input;
    //    input.v = i; 
    //    //float8_1_t output = bf16_to_f8_1(input);
    //    float8_2_t output = bf16_to_f8_2(input);
    //    // Print the input and its corresponding output result
    //    printf("0x%x 0x%x\n", input.v, output.v);
    //}
    //float16_t a;
    //a.v = 0x8000;
    //float8_1_t output = bf16_to_f8_1(a);
    //float8_2_t output = bf16_to_f8_2(a);
    //printf("0x%x 0x%x\n", a.v, output.v);

*/
    return 0;
}

char* toBinary(int num, int bitCount) {
    // Allocate memory for the binary representation string + null terminator
    char* binaryRepresentation = (char*)malloc(bitCount + 1);
    
    // Iterate through each bit, and fill the string from the end to the beginning
    for(int i = 0; i < bitCount; i++) {
        binaryRepresentation[bitCount - 1 - i] = (num % 2) + '0'; // Convert bit to character '0' or '1'
        num /= 2;
    }
    
    // Null-terminate the string
    binaryRepresentation[bitCount] = '\0';
    
    return binaryRepresentation;
}



int64_t rand_gen(int64_t lower, int64_t upper) {
    static bool seed_set = false; // Flag to check if the seed has been set
    
    if (!seed_set) { // If the seed has not been set yet
        srand(time(NULL)); // Seed the random number generator with the current time
        seed_set = true; // Set the flag to indicate that the seed has been set
    }

    if (upper == INT64_MAX && lower == INT64_MIN) {
        int64_t upperHalf = (int64_t) rand() << 32;
        int64_t lowerHalf = (int64_t) rand();
        return upperHalf | lowerHalf;
    } else if (upper == UINT64_MAX && lower == 0) {
        uint64_t upperHalf = (uint64_t) rand() << 32;
        uint64_t lowerHalf = (uint64_t) rand();
        return upperHalf | lowerHalf;
    }

    int64_t range = upper - lower + 1; // Calculate the range of values
    int64_t random_num = rand() % range; // Generate a random integer within the range
  
    return lower + random_num; // Scale and shift the random integer to fit within the specified range
}

void float_type(char* op_type) {
  if (strstr(op_type, "8_1")) {
    f_type   = F8_1;
    f_size   = 8;
    f_min    = INT8_MIN;
    f_max    = INT8_MAX;
    f_maxu   = UINT8_MAX;
    num_tabs = 1;
    exponent_size = 4;
    mantissa_size = 3;
    bias = 7;
  }
  else if (strstr(op_type, "8_2")) {
    f_type   = F8_2;
    f_size   = 8;
    f_min    = INT8_MIN;
    f_max    = INT8_MAX;
    f_maxu   = UINT8_MAX;
    num_tabs = 1;
    exponent_size = 5;
    mantissa_size = 2;
    bias = 15;
  }
  else if (strstr(op_type, "16")) {
    f_type   = F16;
    f_size   = 16;
    f_min    = INT16_MIN;
    f_max    = INT16_MAX;
    f_maxu   = UINT16_MAX;
    num_tabs = 1;
    exponent_size = 5;
    mantissa_size = 10;
    bias = 15;
  }
  else if (strstr(op_type, "32")) {
    f_type  = F32;
    f_size  = 32;
    f_min   = INT32_MIN;
    f_max   = INT32_MAX;
    f_maxu  = UINT32_MAX;
    num_tabs = 2;
    exponent_size = 8;
    mantissa_size = 23;
    bias = 127;
  }
  else if (strstr(op_type, "64")) {
    f_type   = F64;
    f_size   = 64;
    f_min    = INT64_MIN;
    f_max    = INT64_MAX;
    f_maxu   = UINT64_MAX;
    num_tabs = 3;
    exponent_size = 11;
    mantissa_size = 52;
    bias = 1023;
  }
  else if (strstr(op_type, "128")) {
    f_type = F128;
    f_size = 128;
    //f_max  = UINT128_MAX;
    num_tabs = 4;
    //exponent_size = 11;
    //mantissa_size = 52;
  }
  exp_range = pow(2, exponent_size);
  mnt_range = pow(2, mantissa_size);
  mantissa_en = mnt_range-1;
  exponent_en = exp_range-1;
  memset(format, '\t', num_tabs);   // Fill the string with tabs
  format[num_tabs] = '\0';          // Null-terminate the string
}

int op_index(char* op_type) {
  float_type(op_type);

  if ((strcmp(op_type ,"fadd8_1") == 0) ||
      (strcmp(op_type ,"fadd8_2") == 0) ||
      (strcmp(op_type ,"fadd16")  == 0) ||
      (strcmp(op_type ,"fadd32")  == 0) ||
      (strcmp(op_type ,"fadd64")  == 0) ||
      (strcmp(op_type ,"fadd128") == 0)) {
    op_type_int = FADD;
    operand_count= 2;
  }
  else if ((strcmp(op_type ,"fsub8_1") == 0) ||
           (strcmp(op_type ,"fsub8_2") == 0) ||
           (strcmp(op_type ,"fsub16")  == 0) ||
           (strcmp(op_type ,"fsub32")  == 0) ||
           (strcmp(op_type ,"fsub64")  == 0) ||
           (strcmp(op_type ,"fsub128") == 0)) {
    op_type_int = FSUB;
    operand_count= 2;
  }
  else if ((strcmp(op_type ,"frsub8_1") == 0) ||
           (strcmp(op_type ,"frsub8_2") == 0) ||
           (strcmp(op_type ,"frsub16")  == 0) ||
           (strcmp(op_type ,"frsub32")  == 0) ||
           (strcmp(op_type ,"frsub64")  == 0) ||
           (strcmp(op_type ,"frsub128") == 0)) {
    op_type_int = FRSUB;
    operand_count= 2;
  }
  else if ((strcmp(op_type ,"fmul8_1") == 0) ||
           (strcmp(op_type ,"fmul8_2") == 0) ||
           (strcmp(op_type ,"fmul16")  == 0) ||
           (strcmp(op_type ,"fmul32")  == 0) ||
           (strcmp(op_type ,"fmul64")  == 0) ||
           (strcmp(op_type ,"fmul128") == 0)) {
    op_type_int = FMUL;
    operand_count= 2;
  }
  else if ((strcmp(op_type ,"fdiv8_1") == 0) ||
           (strcmp(op_type ,"fdiv8_2") == 0) ||
           (strcmp(op_type ,"fdiv16")  == 0) ||
           (strcmp(op_type ,"fdiv32")  == 0) ||
           (strcmp(op_type ,"fdiv64")  == 0) ||
           (strcmp(op_type ,"fdiv128") == 0)) {
    op_type_int = FDIV;
    operand_count= 2;
  }
  else if ((strcmp(op_type ,"frdiv8_1") == 0) ||
           (strcmp(op_type ,"frdiv8_2") == 0) ||
           (strcmp(op_type ,"frdiv16")  == 0) ||
           (strcmp(op_type ,"frdiv32")  == 0) ||
           (strcmp(op_type ,"frdiv64")  == 0) ||
           (strcmp(op_type ,"frdiv128") == 0)) {
    op_type_int = FRDIV;
    operand_count= 2;
  }
  else if ((strcmp(op_type ,"frec8_1") == 0) ||
           (strcmp(op_type ,"frec8_2") == 0) ||
           (strcmp(op_type ,"frec16")  == 0) ||
           (strcmp(op_type ,"frec32")  == 0) ||
           (strcmp(op_type ,"frec64")  == 0) ||
           (strcmp(op_type ,"frec128") == 0)) {
    op_type_int = FREC;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fsqrt8_1") == 0) ||
           (strcmp(op_type ,"fsqrt8_2") == 0) ||
           (strcmp(op_type ,"fsqrt16")  == 0) ||
           (strcmp(op_type ,"fsqrt32")  == 0) ||
           (strcmp(op_type ,"fsqrt64")  == 0) ||
           (strcmp(op_type ,"fsqrt128") == 0)) {
    op_type_int = FSQRT;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"frsqrt8_1") == 0) ||
           (strcmp(op_type ,"frsqrt8_2") == 0) ||
           (strcmp(op_type ,"frsqrt16")  == 0) ||
           (strcmp(op_type ,"frsqrt32")  == 0) ||
           (strcmp(op_type ,"frsqrt64")  == 0) ||
           (strcmp(op_type ,"frsqrt128") == 0)) {
    op_type_int = FRSQRT;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fmacc8_1") == 0) ||
           (strcmp(op_type ,"fmacc8_2") == 0) ||
           (strcmp(op_type ,"fmacc16")  == 0) ||
           (strcmp(op_type ,"fmacc32")  == 0) ||
           (strcmp(op_type ,"fmacc64")  == 0) ||
           (strcmp(op_type ,"fmacc128") == 0)) {
    op_type_int = FMACC;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fnmacc8_1") == 0) ||
           (strcmp(op_type ,"fnmacc8_2") == 0) ||
           (strcmp(op_type ,"fnmacc16")  == 0) ||
           (strcmp(op_type ,"fnmacc32")  == 0) ||
           (strcmp(op_type ,"fnmacc64")  == 0) ||
           (strcmp(op_type ,"fnmacc128") == 0)) {
    op_type_int = FNMACC;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fmsac8_1") == 0) ||
           (strcmp(op_type ,"fmsac8_2") == 0) ||
           (strcmp(op_type ,"fmsac16")  == 0) ||
           (strcmp(op_type ,"fmsac32")  == 0) ||
           (strcmp(op_type ,"fmsac64")  == 0) ||
           (strcmp(op_type ,"fmsac128") == 0)) {
    op_type_int = FMSAC;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fnmsac8_1") == 0) ||
           (strcmp(op_type ,"fnmsac8_2") == 0) ||
           (strcmp(op_type ,"fnmsac16")  == 0) ||
           (strcmp(op_type ,"fnmsac32")  == 0) ||
           (strcmp(op_type ,"fnmsac64")  == 0) ||
           (strcmp(op_type ,"fnmsac128") == 0)) {
    op_type_int = FNMSAC;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fmadd8_1") == 0) ||
           (strcmp(op_type ,"fmadd8_2") == 0) ||
           (strcmp(op_type ,"fmadd16")  == 0) ||
           (strcmp(op_type ,"fmadd32")  == 0) ||
           (strcmp(op_type ,"fmadd64")  == 0) ||
           (strcmp(op_type ,"fmadd128") == 0)) {
    op_type_int = FMADD;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fnmadd8_1") == 0) ||
           (strcmp(op_type ,"fnmadd8_2") == 0) ||
           (strcmp(op_type ,"fnmadd16")  == 0) ||
           (strcmp(op_type ,"fnmadd32")  == 0) ||
           (strcmp(op_type ,"fnmadd64")  == 0) ||
           (strcmp(op_type ,"fnmadd128") == 0)) {
    op_type_int = FNMADD;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fmsub8_1") == 0) ||
           (strcmp(op_type ,"fmsub8_2") == 0) ||
           (strcmp(op_type ,"fmsub16")  == 0) ||
           (strcmp(op_type ,"fmsub32")  == 0) ||
           (strcmp(op_type ,"fmsub64")  == 0) ||
           (strcmp(op_type ,"fmsub128") == 0)) {
    op_type_int = FMSUB;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fnmsub8_1") == 0) ||
           (strcmp(op_type ,"fnmsub8_2") == 0) ||
           (strcmp(op_type ,"fnmsub16")  == 0) ||
           (strcmp(op_type ,"fnmsub32")  == 0) ||
           (strcmp(op_type ,"fnmsub64")  == 0) ||
           (strcmp(op_type ,"fnmsub128") == 0)) {
    op_type_int = FNMSUB;
    operand_count= 3;
  }
  else if ((strcmp(op_type ,"fclass8_1") == 0) ||
           (strcmp(op_type ,"fclass8_2") == 0) ||
           (strcmp(op_type ,"fclass16")  == 0) ||
           (strcmp(op_type ,"fclass32")  == 0) ||
           (strcmp(op_type ,"fclass64")  == 0) ||
           (strcmp(op_type ,"fclass128") == 0)) {
    op_type_int = FCLASS;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fmin8_1") == 0) ||
           (strcmp(op_type ,"fmin8_2") == 0) ||
           (strcmp(op_type ,"fmin16")  == 0) ||
           (strcmp(op_type ,"fmin32")  == 0) ||
           (strcmp(op_type ,"fmin64")  == 0) ||
           (strcmp(op_type ,"fmin128") == 0)) {
    op_type_int = FMIN;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fmax8_1") == 0) ||
           (strcmp(op_type ,"fmax8_2") == 0) ||
           (strcmp(op_type ,"fmax16")  == 0) ||
           (strcmp(op_type ,"fmax32")  == 0) ||
           (strcmp(op_type ,"fmax64")  == 0) ||
           (strcmp(op_type ,"fmax128") == 0)) {
    op_type_int = FMAX;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"feq8_1") == 0) ||
           (strcmp(op_type ,"feq8_2") == 0) ||
           (strcmp(op_type ,"feq16")  == 0) ||
           (strcmp(op_type ,"feq32")  == 0) ||
           (strcmp(op_type ,"feq64")  == 0) ||
           (strcmp(op_type ,"feq128") == 0)) {
    op_type_int = FEQ;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fne8_1") == 0) ||
           (strcmp(op_type ,"fne8_2") == 0) ||
           (strcmp(op_type ,"fne16")  == 0) ||
           (strcmp(op_type ,"fne32")  == 0) ||
           (strcmp(op_type ,"fne64")  == 0) ||
           (strcmp(op_type ,"fne128") == 0)) {
    op_type_int = FNE;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fle8_1") == 0) ||
           (strcmp(op_type ,"fle8_2") == 0) ||
           (strcmp(op_type ,"fle16")  == 0) ||
           (strcmp(op_type ,"fle32")  == 0) ||
           (strcmp(op_type ,"fle64")  == 0) ||
           (strcmp(op_type ,"fle128") == 0)) {
    op_type_int = FLE;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"flt8_1") == 0) ||
           (strcmp(op_type ,"flt8_2") == 0) ||
           (strcmp(op_type ,"flt16")  == 0) ||
           (strcmp(op_type ,"flt32")  == 0) ||
           (strcmp(op_type ,"flt64")  == 0) ||
           (strcmp(op_type ,"flt128") == 0)) {
    op_type_int = FLT;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fge8_1") == 0) ||
           (strcmp(op_type ,"fge8_2") == 0) ||
           (strcmp(op_type ,"fge16")  == 0) ||
           (strcmp(op_type ,"fge32")  == 0) ||
           (strcmp(op_type ,"fge64")  == 0) ||
           (strcmp(op_type ,"fge128") == 0)) {
    op_type_int = FGE;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fgt8_1") == 0) ||
           (strcmp(op_type ,"fgt8_2") == 0) ||
           (strcmp(op_type ,"fgt16")  == 0) ||
           (strcmp(op_type ,"fgt32")  == 0) ||
           (strcmp(op_type ,"fgt64")  == 0) ||
           (strcmp(op_type ,"fgt128") == 0)) {
    op_type_int = FGT;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fcvt.x.f.8_1") == 0) ||
           (strcmp(op_type ,"fcvt.x.f.8_2") == 0) ||
           (strcmp(op_type ,"fcvt.x.f.16")  == 0) ||
           (strcmp(op_type ,"fcvt.x.f.32")  == 0) ||
           (strcmp(op_type ,"fcvt.x.f.64")  == 0) ||
           (strcmp(op_type ,"fcvt.x.f.128") == 0)) {
    op_type_int = FCVT_X_F;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fcvt.xu.f.8_1") == 0) ||
           (strcmp(op_type ,"fcvt.xu.f.8_2") == 0) ||
           (strcmp(op_type ,"fcvt.xu.f.16")  == 0) ||
           (strcmp(op_type ,"fcvt.xu.f.32")  == 0) ||
           (strcmp(op_type ,"fcvt.xu.f.64")  == 0) ||
           (strcmp(op_type ,"fcvt.xu.f.128") == 0)) {
    op_type_int = FCVT_XU_F;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fcvt.f.x.8_1") == 0) ||
           (strcmp(op_type ,"fcvt.f.x.8_2") == 0) ||
           (strcmp(op_type ,"fcvt.f.x.16")  == 0) ||
           (strcmp(op_type ,"fcvt.f.x.32")  == 0) ||
           (strcmp(op_type ,"fcvt.f.x.64")  == 0) ||
           (strcmp(op_type ,"fcvt.f.x.128") == 0)) {
    op_type_int = FCVT_F_X;
    operand_count= 1;
  }
  else if ((strcmp(op_type ,"fcvt.f.xu.8_1") == 0) ||
           (strcmp(op_type ,"fcvt.f.xu.8_2") == 0) ||
           (strcmp(op_type ,"fcvt.f.xu.16")  == 0) ||
           (strcmp(op_type ,"fcvt.f.xu.32")  == 0) ||
           (strcmp(op_type ,"fcvt.f.xu.64")  == 0) ||
           (strcmp(op_type ,"fcvt.f.xu.128") == 0)) {
    op_type_int = FCVT_F_XU;
    operand_count= 1;
  }
  else {
    goto UNKNOWN_OP;
  }
  //printf(RED"ERRROR:" CRESET "This instruction is not supported yet for this float type");
  //exit(1);
  printf("\nOperation: " BLUE "%s\n" CRESET, op_type);
  return op_type_int;

  UNKNOWN_OP:
  printf(RED "\nERROR" CRESET ": Unknown op type " GREEN "%s\n\n" CRESET, op_type);
  exit(1);
}

float_rep hex_to_float(void* src) {
  // AAA return inf and NaN when the case is input is infinity of NaN
  const uint64_t lsb_en = 1;
  int bias_compeensate;
  float_rep src_f;
  src_f.sign     = (*(uint64_t*)src >> (f_size-1)) & lsb_en;
  src_f.exponent = (*(uint64_t*)src >> mantissa_size) & exponent_en;  // calculate the exponent
  src_f.mantissa = (*(uint64_t*)src & mantissa_en);  // calculate the mantissa
  src_f.mantissa_f = 0;
  if (src_f.exponent > 0) {
    src_f.implicit = 1;
    bias_compeensate = 0;
  } 
  else {
    src_f.implicit = 0;
    bias_compeensate = 1;
  }
  for (int i=1; i<=mantissa_size; i++) {  // represent the mantissa in the normalized floating point format
    src_f.mantissa_f += (double)((src_f.mantissa >> (mantissa_size-i)) & lsb_en) * (1 / pow(2, i));
  }
  src_f.mantissa_f += src_f.implicit; // add the implicit bit to get the final mantissa representation
  if (src_f.sign) {
    src_f.fp = -pow(2, (double)src_f.exponent-(bias-bias_compeensate)) * src_f.mantissa_f;  // calculate the float
  }
  else {
    src_f.fp = pow(2, (double)src_f.exponent-(bias-bias_compeensate)) * src_f.mantissa_f;  // calculate the float
  }
  if (src_f.exponent == exp_range-1) { // if the exponent bits are all ones
    if (src_f.mantissa == 0) { // if the mantissa bits are all zero
      if (src_f.sign == 0) {
        src_f.fp = INFINITY;  // return inf
      }
      else {
        src_f.fp = -INFINITY; // return -inf
      }
    }
    else {
      src_f.fp = - 0.0 / 0.0; // return a -NaN
      src_f.fp = -src_f.fp;   // the standard does not dictate the sign of the NaN, but we chosoe positive sign, which is why we negate the gcc output which always returns a negative NaN
    }
  }
  return src_f;
}

void* op(void* src1, void* src2, void* src3, int en_print) {
  if (en_print) {
    src1_f = hex_to_float(src1);
    src2_f = hex_to_float(src2);
    src3_f = hex_to_float(src3);
  }
  if (f_type == F8_1) {
    float8_1_t s1 = *(float8_1_t*) src1;
    float8_1_t s2 = *(float8_1_t*) src2;
    float8_1_t s3 = *(float8_1_t*) src3;
    float8_1_t res = {.v = 0};
    if (op_type_int == FADD) {
      res = f8_1_add(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s+ %s%le %s= %s%le\n"
          "\t%s0x%x %s+ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSUB) {
      res = f8_1_sub(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSUB) {
      res = f8_1_sub(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMUL) {
      res = f8_1_mul(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s* %s%le %s= %s%le\n"
          "\t%s0x%x %s* %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FDIV) {
      res = f8_1_div(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRDIV) {
      res = f8_1_div(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FREC) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f8_1_recip7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      printf(YELLOW" \nWARNING" WHITE ": frec8_1 operation apparently giving erreneous results when dividing (e.g. 1/1 gives a result of 0.5)\n\n" CRESET);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s1 / %s%le%s = %s%le\n"
          "\t%s1 / %s0x%x%s = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f8_1_sqrt(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfsqrt(%s%le%s) = %s%le\n"
          "\t%sfsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f8_1_rsqrte7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfrsqrt(%s%le%s) = %s%le\n"
          "\t%sfrsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMACC) {
      res = f8_1_mulAdd(s1, s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMACC) {
      res = f8_1_mulAdd(s1, f8_1(s2.v ^ F8_1_SIGN), f8_1(s3.v ^ F8_1_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSAC) {
      res = f8_1_mulAdd(s1, s2, f8_1(s3.v ^ F8_1_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSAC) {
      res = f8_1_mulAdd(f8_1(s1.v ^ F8_1_SIGN), s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMADD) {
      res = f8_1_mulAdd(s3, s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMADD) {
      res = f8_1_mulAdd(f8_1(s3.v ^ F8_1_SIGN), s1, f8_1(s2.v ^ F8_1_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSUB) {
      res = f8_1_mulAdd(s3, s1, f8_1(s2.v ^ F8_1_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSUB) {
      res = f8_1_mulAdd(f8_1(s3.v ^ F8_1_SIGN), s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMIN) {
      res = f8_1_min(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmin(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmin(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FMAX) {
      res = f8_1_max(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmax(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmax(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FEQ) {
      bool res_bool = f8_1_eq(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfeq(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfeq(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False",
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FNE) {
      bool res_bool = !f8_1_eq(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfne(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfne(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLE) {
      bool res_bool = f8_1_le(s1, s2);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfle(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfle(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLT) {
      bool res_bool = f8_1_lt(s1, s2);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sflt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sflt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGE) {
      bool res_bool = f8_1_le(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfge(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfge(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGT) {
      bool res_bool = f8_1_lt(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfgt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfgt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_X_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f8_1_to_i8(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.x.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.x.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_XU_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f8_1_to_ui8(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.xu.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.xu.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_X) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res   = i32_to_f8_1((int8_t)s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.x(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.x(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, (int8_t)s2.v, white, green, res_f.fp, 
          white, green, s2.v,         white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_XU) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = ui32_to_f8_1(s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.xu(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.xu(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, s2.v, white, green, res_f.fp,
          white, green, s2.v, white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }

    // ui8 to f8_1
    //if (strcmp(op_type, "fcvt8_1.f.xu") == 0) {
    //  uint32_t vs2_u = strtol(argv[2], NULL, 16);
    //  float8_1_t res_8 = ui32_to_f8_1(vs2_u);
    //  printf("src_ui32 = %x, res_f8_1 = %x (FF = 0x%x)\n", vs2_u, res_8.v, softfloat_exceptionFlags);
    //}

    // f8_to_i8
    //float8_1_t src_i8 = { .v = 0xc3 };
    //int_fast8_t res_i8 = f8_1_to_ui8(src_i8, 0, true);
    //printf("src_f8 = %x, res_i8 = %x\n", src_i8, res_i8);
    //printf("softfloat_exceptionFlags = 0x%x\n", softfloat_exceptionFlags);
    else if (op_type_int == FCLASS) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      uint64_t class_res; // there are 10 FP classes represented in 1-hot bit encoding, thus 10-bits are needed. This variable is made becasue the 8-bit "res" variable cannot hold all the classes.
      class_res = f8_1_classify(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      char* class_num = toBinary(class_res, 10);
      char* class = (char*) malloc(50);;
      if (class_res == 1<<0) class = "Negative Infinity"; else if (class_res == 1<<1) class = "Negative Normalized";
      else if (class_res == 1<<2) class = "Negative Denormalized"; else if (class_res == 1<<3) class = "Negative Zero";
      else if (class_res == 1<<4) class = "Positive Zero"; else if (class_res == 1<<5) class = "Positive Denormalized";
      else if (class_res == 1<<6) class = "Positive Normalized"; else if (class_res == 1<<7) class = "Positive Infinity";
      else if (class_res == 1<<8) class = "Signalling NaN"; else if (class_res == 1<<9) class = "Quiet NaN";
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfclass(%s%le%s) = %s%s_b %s%s\n"
          "\t%sfclass(%s0x%x%s) = %s%s_b  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, class_num, PURPLE, class,
          white, green, s2.v,      white, green, class_num, white, yellow, FLAGS, white, creset);
      }
    }
    float8_1_t* res_ptr = malloc(sizeof(float8_1_t));
    *res_ptr = res;
    return res_ptr;
  }
  else if (f_type == F8_2) {
    float8_2_t s1 = *(float8_2_t*) src1;
    float8_2_t s2 = *(float8_2_t*) src2;
    float8_2_t s3 = *(float8_2_t*) src3;
    float8_2_t res;
    if (op_type_int == FADD) {
      res = f8_2_add(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s+ %s%le %s= %s%le\n"
          "\t%s0x%x %s+ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSUB) {
      res = f8_2_sub(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSUB) {
      res = f8_2_sub(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMUL) {
      res = f8_2_mul(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s* %s%le %s= %s%le\n"
          "\t%s0x%x %s* %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FDIV) {
      res = f8_2_div(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRDIV) {
      res = f8_2_div(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FREC) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f8_2_recip7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      printf(YELLOW" \nWARNING" WHITE ": frec8_2 operation apparently giving erreneous results when dividing (e.g. 1/1 gives a result of 0.5)\n\n" CRESET);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s1 / %s%le%s = %s%le\n"
          "\t%s1 / %s0x%x%s = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f8_2_sqrt(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfsqrt(%s%le%s) = %s%le\n"
          "\t%sfsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f8_2_rsqrte7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfrsqrt(%s%le%s) = %s%le\n"
          "\t%sfrsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMACC) {
      res = f8_2_mulAdd(s1, s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMACC) {
      res = f8_2_mulAdd(s1, f8_2(s2.v ^ F8_2_SIGN), f8_2(s3.v ^ F8_2_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSAC) {
      res = f8_2_mulAdd(s1, s2, f8_2(s3.v ^ F8_2_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSAC) {
      res = f8_2_mulAdd(f8_2(s1.v ^ F8_2_SIGN), s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMADD) {
      res = f8_2_mulAdd(s3, s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMADD) {
      res = f8_2_mulAdd(f8_2(s3.v ^ F8_2_SIGN), s1, f8_2(s2.v ^ F8_2_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSUB) {
      res = f8_2_mulAdd(s3, s1, f8_2(s2.v ^ F8_2_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSUB) {
      res = f8_2_mulAdd(f8_2(s3.v ^ F8_2_SIGN), s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMIN) {
      res = f8_2_min(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmin(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmin(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FMAX) {
      res = f8_2_max(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmax(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmax(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FEQ) {
      bool res_bool = f8_2_eq(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfeq(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfeq(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False",
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FNE) {
      bool res_bool = !f8_2_eq(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfne(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfne(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLE) {
      bool res_bool = f8_2_le(s1, s2);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfle(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfle(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLT) {
      bool res_bool = f8_2_lt(s1, s2);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sflt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sflt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGE) {
      bool res_bool = f8_2_le(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfge(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfge(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGT) {
      bool res_bool = f8_2_lt(s2, s1);
      res.v = (uint8_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfgt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfgt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_X_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f8_2_to_i8(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.x.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.x.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_XU_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f8_2_to_ui8(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.xu.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.xu.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_X) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res   = i32_to_f8_2((int8_t)s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.x(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.x(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, (int8_t)s2.v, white, green, res_f.fp, 
          white, green, s2.v,         white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_XU) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = ui32_to_f8_2(s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.xu(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.xu(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, s2.v, white, green, res_f.fp,
          white, green, s2.v, white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCLASS) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      uint64_t class_res; // there are 10 FP classes represented in 1-hot bit encoding, thus 10-bits are needed. This variable is made becasue the 8-bit "res" variable cannot hold all the classes.
      class_res = f8_2_classify(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      char* class_num = toBinary(class_res, 10);
      char* class = (char*) malloc(50);;
      if (class_res == 1<<0) class = "Negative Infinity"; else if (class_res == 1<<1) class = "Negative Normalized";
      else if (class_res == 1<<2) class = "Negative Denormalized"; else if (class_res == 1<<3) class = "Negative Zero";
      else if (class_res == 1<<4) class = "Positive Zero"; else if (class_res == 1<<5) class = "Positive Denormalized";
      else if (class_res == 1<<6) class = "Positive Normalized"; else if (class_res == 1<<7) class = "Positive Infinity";
      else if (class_res == 1<<8) class = "Signalling NaN"; else if (class_res == 1<<9) class = "Quiet NaN";
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfclass(%s%le%s) = %s%s_b %s%s\n"
          "\t%sfclass(%s0x%x%s) = %s%s_b  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, class_num, PURPLE, class,
          white, green, s2.v,      white, green, class_num, white, yellow, FLAGS, white, creset);
      }
    }
    float8_2_t* res_ptr = malloc(sizeof(float8_2_t));
    *res_ptr = res;
    return res_ptr;
  }
  else if (f_type == F16) {
    float16_t s1 = *(float16_t*) src1;
    float16_t s2 = *(float16_t*) src2;
    float16_t s3 = *(float16_t*) src3;
    float16_t res;
    if (op_type_int == FADD) {
      res = f16_add(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s+ %s%le %s= %s%le\n"
          "\t%s0x%x %s+ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSUB) {
      res = f16_sub(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSUB) {
      res = f16_sub(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMUL) {
      res = f16_mul(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s* %s%le %s= %s%le\n"
          "\t%s0x%x %s* %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FDIV) {
      res = f16_div(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRDIV) {
      res = f16_div(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FREC) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f16_recip7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s1 / %s%le%s = %s%le\n"
          "\t%s1 / %s0x%x%s = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f16_sqrt(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfsqrt(%s%le%s) = %s%le\n"
          "\t%sfsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f16_rsqrte7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfrsqrt(%s%le%s) = %s%le\n"
          "\t%sfrsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMACC) {
      res = f16_mulAdd(s1, s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMACC) {
      res = f16_mulAdd(s1, f16(s2.v ^ F16_SIGN), f16(s3.v ^ F16_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSAC) {
      res = f16_mulAdd(s1, s2, f16(s3.v ^ F16_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSAC) {
      res = f16_mulAdd(f16(s1.v ^ F16_SIGN), s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMADD) {
      res = f16_mulAdd(s3, s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMADD) {
      res = f16_mulAdd(f16(s3.v ^ F16_SIGN), s1, f16(s2.v ^ F16_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSUB) {
      res = f16_mulAdd(s3, s1, f16(s2.v ^ F16_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSUB) {
      res = f16_mulAdd(f16(s3.v ^ F16_SIGN), s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMIN) {
      res = f16_min(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmin(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmin(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FMAX) {
      res = f16_max(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmax(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmax(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FEQ) {
      bool res_bool = f16_eq(s2, s1);
      res.v = (uint16_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfeq(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfeq(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False",
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FNE) {
      bool res_bool = !f16_eq(s2, s1);
      res.v = (uint16_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfne(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfne(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLE) {
      bool res_bool = f16_le(s1, s2);
      res.v = (uint16_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfle(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfle(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLT) {
      bool res_bool = f16_lt(s1, s2);
      res.v = (uint16_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sflt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sflt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGE) {
      bool res_bool = f16_le(s2, s1);
      res.v = (uint16_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfge(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfge(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGT) {
      bool res_bool = f16_lt(s2, s1);
      res.v = (uint16_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfgt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfgt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_X_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f16_to_i16(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.x.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.x.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_XU_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f16_to_ui16(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.xu.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.xu.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_X) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res   = i32_to_f16((int16_t)s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.x(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.x(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, (int16_t)s2.v, white, green, res_f.fp, 
          white, green, s2.v,         white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_XU) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = ui32_to_f16(s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.xu(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.xu(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, s2.v, white, green, res_f.fp,
          white, green, s2.v, white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCLASS) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f16_classify(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      char* class_num = toBinary(res.v, 10);
      char* class = (char*) malloc(50);;
      if (res.v == 1<<0) class = "Negative Infinity"; else if (res.v == 1<<1) class = "Negative Normalized";
      else if (res.v == 1<<2) class = "Negative Denormalized"; else if (res.v == 1<<3) class = "Negative Zero";
      else if (res.v == 1<<4) class = "Positive Zero"; else if (res.v == 1<<5) class = "Positive Denormalized";
      else if (res.v == 1<<6) class = "Positive Normalized"; else if (res.v == 1<<7) class = "Positive Infinity";
      else if (res.v == 1<<8) class = "Signalling NaN"; else if (res.v == 1<<9) class = "Quiet NaN";
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfclass(%s%le%s) = %s%s_b %s%s\n"
          "\t%sfclass(%s0x%x%s) = %s%s_b  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, class_num, PURPLE, class,
          white, green, s2.v,      white, green, class_num, white, yellow, FLAGS, white, creset);
      };
    }
    float16_t* res_ptr = malloc(sizeof(float16_t));
    *res_ptr = res;
    return res_ptr;
  }
  else if (f_type == F32) {
    float32_t s1 = *(float32_t*) src1;
    float32_t s2 = *(float32_t*) src2;
    float32_t s3 = *(float32_t*) src3;
    float32_t res;
    if (op_type_int == FADD) {
      res = f32_add(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s+ %s%le %s= %s%le\n"
          "\t%s0x%x %s+ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSUB) {
      res = f32_sub(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSUB) {
      res = f32_sub(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%x %s- %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMUL) {
      res = f32_mul(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s* %s%le %s= %s%le\n"
          "\t%s0x%x %s* %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FDIV) {
      res = f32_div(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRDIV) {
      res = f32_div(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%x %s/ %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FREC) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f32_recip7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s1 / %s%le%s = %s%le\n"
          "\t%s1 / %s0x%x%s = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f32_sqrt(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfsqrt(%s%le%s) = %s%le\n"
          "\t%sfsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f32_rsqrte7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfrsqrt(%s%le%s) = %s%le\n"
          "\t%sfrsqrt(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMACC) {
      res = f32_mulAdd(s1, s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMACC) {
      res = f32_mulAdd(s1, f32(s2.v ^ F32_SIGN), f32(s3.v ^ F32_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSAC) {
      res = f32_mulAdd(s1, s2, f32(s3.v ^ F32_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSAC) {
      res = f32_mulAdd(f32(s1.v ^ F32_SIGN), s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMADD) {
      res = f32_mulAdd(s3, s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMADD) {
      res = f32_mulAdd(f32(s3.v ^ F32_SIGN), s1, f32(s2.v ^ F32_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSUB) {
      res = f32_mulAdd(s3, s1, f32(s2.v ^ F32_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%x %s* %s0x%x%s) - %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSUB) {
      res = f32_mulAdd(f32(s3.v ^ F32_SIGN), s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%x %s* %s0x%x%s) + %s0x%x %s= %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMIN) {
      res = f32_min(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmin(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmin(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FMAX) {
      res = f32_max(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmax(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmax(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FEQ) {
      bool res_bool = f32_eq(s2, s1);
      res.v = (uint32_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfeq(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfeq(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False",
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FNE) {
      bool res_bool = !f32_eq(s2, s1);
      res.v = (uint32_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfne(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfne(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLE) {
      bool res_bool = f32_le(s1, s2);
      res.v = (uint32_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfle(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfle(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLT) {
      bool res_bool = f32_lt(s1, s2);
      res.v = (uint32_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sflt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sflt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGE) {
      bool res_bool = f32_le(s2, s1);
      res.v = (uint32_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfge(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfge(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGT) {
      bool res_bool = f32_lt(s2, s1);
      res.v = (uint32_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfgt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfgt(%s0x%x%s, %s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_X_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f32_to_i32(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.x.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.x.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_XU_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f32_to_ui32(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.xu.f(%s%le%s) = %s%d\n"
          "\t%sfcvt.xu.f(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_X) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res   = i32_to_f32((int32_t)s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.x(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.x(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, (int32_t)s2.v, white, green, res_f.fp, 
          white, green, s2.v,         white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_XU) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = ui32_to_f32(s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.xu(%s%d%s) = %s%le\n"
          "\t%sfcvt.f.xu(%s0x%x%s) = %s0x%x  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, s2.v, white, green, res_f.fp,
          white, green, s2.v, white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCLASS) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f32_classify(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      char* class_num = toBinary(res.v, 10);
      char* class = (char*) malloc(50);;
      if (res.v == 1<<0) class = "Negative Infinity"; else if (res.v == 1<<1) class = "Negative Normalized";
      else if (res.v == 1<<2) class = "Negative Denormalized"; else if (res.v == 1<<3) class = "Negative Zero";
      else if (res.v == 1<<4) class = "Positive Zero"; else if (res.v == 1<<5) class = "Positive Denormalized";
      else if (res.v == 1<<6) class = "Positive Normalized"; else if (res.v == 1<<7) class = "Positive Infinity";
      else if (res.v == 1<<8) class = "Signalling NaN"; else if (res.v == 1<<9) class = "Quiet NaN";
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfclass(%s%le%s) = %s%s_b %s%s\n"
          "\t%sfclass(%s0x%x%s) = %s%s_b  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, class_num, PURPLE, class,
          white, green, s2.v,      white, green, class_num, white, yellow, FLAGS, white, creset);
      };
    }
    float32_t* res_ptr = malloc(sizeof(float32_t));
    *res_ptr = res;
    return res_ptr;
  }
  else if (f_type == F64) {
    float64_t s1 = *(float64_t*) src1;
    float64_t s2 = *(float64_t*) src2;
    float64_t s3 = *(float64_t*) src3;
    float64_t res;
    if (op_type_int == FADD) {
      res = f64_add(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s+ %s%le %s= %s%le\n"
          "\t%s0x%lx %s+ %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSUB) {
      res = f64_sub(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%lx %s- %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSUB) {
      res = f64_sub(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s- %s%le %s= %s%le\n"
          "\t%s0x%lx %s- %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMUL) {
      res = f64_mul(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s* %s%le %s= %s%le\n"
          "\t%s0x%lx %s* %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FDIV) {
      res = f64_div(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%lx %s/ %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRDIV) {
      res = f64_div(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s%s:\n"
          "\t%s%le %s/ %s%le %s= %s%le\n"
          "\t%s0x%lx %s/ %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, white, 
          green, src1_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FREC) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f64_recip7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s1 / %s%le%s = %s%le\n"
          "\t%s1 / %s0x%lx%s = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f64_sqrt(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfsqrt(%s%le%s) = %s%le\n"
          "\t%sfsqrt(%s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FRSQRT) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = f64_rsqrte7(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfrsqrt(%s%le%s) = %s%le\n"
          "\t%sfrsqrt(%s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res_f.fp,
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMACC) {
      res = f64_mulAdd(s1, s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%lx %s* %s0x%lx%s) + %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMACC) {
      res = f64_mulAdd(s1, f64(s2.v ^ F64_SIGN), f64(s3.v ^ F64_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%lx %s* %s0x%lx%s) - %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSAC) {
      res = f64_mulAdd(s1, s2, f64(s3.v ^ F64_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%lx %s* %s0x%lx%s) - %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSAC) {
      res = f64_mulAdd(f64(s1.v ^ F64_SIGN), s2, s3);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%lx %s* %s0x%lx%s) + %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, src3_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s2.v,      white, green, s3.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMADD) {
      res = f64_mulAdd(s3, s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s(%s0x%lx %s* %s0x%lx%s) + %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMADD) {
      res = f64_mulAdd(f64(s3.v ^ F64_SIGN), s1, f64(s2.v ^ F64_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s-(%s0x%lx %s* %s0x%lx%s) - %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FMSUB) {
      res = f64_mulAdd(s3, s1, f64(s2.v ^ F64_SIGN));
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s(%s%le %s* %s%le%s) - %s%le %s= %s%le\n"
          "\t%s(%s0x%lx %s* %s0x%lx%s) - %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      };
    }
    else if (op_type_int == FNMSUB) {
      res = f64_mulAdd(f64(s3.v ^ F64_SIGN), s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%s-(%s%le %s* %s%le%s) + %s%le %s= %s%le\n"
          "\t%s-(%s0x%lx %s* %s0x%lx%s) + %s0x%lx %s= %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src3_f.fp, white, green, src2_f.fp, white, green, res_f.fp, 
          white, green, s1.v,      white, green, s3.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }

    else if (op_type_int == FMIN) {
      res = f64_min(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmin(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmin(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FMAX) {
      res = f64_max(s2, s1);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfmax(%s%le%s, %s%le%s) = %s%le\n"
          "\t%sfmax(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_f.fp, 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FEQ) {
      bool res_bool = f64_eq(s2, s1);
      res.v = (uint64_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfeq(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfeq(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False",
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FNE) {
      bool res_bool = !f64_eq(s2, s1);
      res.v = (uint64_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfne(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfne(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, src1_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s2.v,      white, green, s1.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLE) {
      bool res_bool = f64_le(s1, s2);
      res.v = (uint64_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfle(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfle(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FLT) {
      bool res_bool = f64_lt(s1, s2);
      res.v = (uint64_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sflt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sflt(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGE) {
      bool res_bool = f64_le(s2, s1);
      res.v = (uint64_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfge(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfge(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FGT) {
      bool res_bool = f64_lt(s2, s1);
      res.v = (uint64_t) res_bool;
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        fprintf(out1_result, "%s%s:\n"
          "\t%sfgt(%s%le%s, %s%le%s) = %s%s\n"
          "\t%sfgt(%s0x%lx%s, %s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src1_f.fp, white, green, src2_f.fp, white, green, res_bool ? "True" : RED "False", 
          white, green, s1.v,      white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_X_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f64_to_i64(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.x.f(%s%le%s) = %s%ld\n"
          "\t%sfcvt.x.f(%s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_XU_F) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = f64_to_ui64(s2, 0, true);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.xu.f(%s%le%s) = %s%ld\n"
          "\t%sfcvt.xu.f(%s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, src2_f.fp, white, green, res.v, 
          white, green, s2.v,      white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_X) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res   = i64_to_f64((int64_t)s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.x(%s%ld%s) = %s%le\n"
          "\t%sfcvt.f.x(%s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, (int64_t)s2.v, white, green, res_f.fp, 
          white, green, s2.v,         white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCVT_F_XU) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res = ui64_to_f64(s2.v);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfcvt.f.xu(%s%ld%s) = %s%le\n"
          "\t%sfcvt.f.xu(%s0x%lx%s) = %s0x%lx  %s(%s%s%s)%s\n\n", 
          blue, op_type, 
          white, green, s2.v, white, green, res_f.fp,
          white, green, s2.v, white, green, res.v, white, yellow, FLAGS, white, creset);
      }
    }
    else if (op_type_int == FCLASS) {
      s2 = s1;                     // in RVV, single operand instructions read vs2, so src1 is considered as src2 here
      src2_f = hex_to_float(src1); // redo the conversion to float considering src1 as if it were src2
      res.v = (int64_t)f64_classify(s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      char* class_num = toBinary(res.v, 10);
      char* class = (char*) malloc(50);
      if (res.v == 1<<0) class = "Negative Infinity"; else if (res.v == 1<<1) class = "Negative Normalized";
      else if (res.v == 1<<2) class = "Negative Denormalized"; else if (res.v == 1<<3) class = "Negative Zero";
      else if (res.v == 1<<4) class = "Positive Zero"; else if (res.v == 1<<5) class = "Positive Denormalized";
      else if (res.v == 1<<6) class = "Positive Normalized"; else if (res.v == 1<<7) class = "Positive Infinity";
      else if (res.v == 1<<8) class = "Signalling NaN"; else if (res.v == 1<<9) class = "Quiet NaN";
      if (en_print) {
        res_f = hex_to_float(&res);
        fprintf(out1_result, "%s%s:\n"
          "\t%sfclass(%s%le%s) = %s%s_b %s%s\n"
          "\t%sfclass(%s0x%lx%s) = %s%s_b  %s(%s%s%s)%s\n\n", 
          blue, op_type,
          white, green, src2_f.fp, white, green, class_num, PURPLE, class,
          white, green, s2.v,      white, green, class_num, white, yellow, FLAGS, white, creset);
      }
    }
    float64_t* res_ptr = malloc(sizeof(float64_t));
    *res_ptr = res;
    return res_ptr;
  }
  else if (f_type == F128) {
    float128_t s1 = *(float128_t*) src1;
    float128_t s2 = *(float128_t*) src2;
    float128_t s3 = *(float128_t*) src3;
    float128_t res;
    if (op_type_int == FADD) {
      res = f128_add(s1, s2);
      FLAGS = toBinary(softfloat_exceptionFlags, 5);
      if (en_print) fprintf(out1_result, "%s%s%s: %s0x%ln %s+ %s0x%ln %s= %s0x%ln  %s(%s%s%s)%s\n\n" , blue, op_type,  white, green, s1.v, white, green, s2.v, white, green, res.v, white, yellow, FLAGS, white, creset);
    }
    float128_t* res_ptr = malloc(sizeof(float128_t));
    *res_ptr = res;
    return res_ptr;
  }
}
