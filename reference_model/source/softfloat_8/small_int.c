#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// Helper function to mask the value according to bitSize
uint8_t maskValue(uint8_t value, uint8_t bitSize) {
    uint8_t mask = (1 << bitSize) - 1;
    return value & mask;
}

// Helper function to sign extend the sign value for prints
int8_t signExtend(uint8_t value, uint8_t bitSize, bool isSigned) {
    if (isSigned) {
        // Check if the 4th bit is set (sign bit for a 4-bit number)
        if (value >> (bitSize-1)) {
            // Sign bit is set; extend the sign into the upper 4 bits
            return value + (0xFF - (pow(2,bitSize)-1));
        } else {
            // return input
            return value;
        }
    }
    // If not signed, return the original value
    return value;
}

// Function prototypes
uint8_t add_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t sub_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t mul_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t mulh_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t div_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t rem_small(uint8_t a, uint8_t b, uint8_t bitSize);

uint8_t mul_add_small(uint8_t a, uint8_t b, uint8_t c, uint8_t bitSize);
uint8_t mul_sub_small(uint8_t a, uint8_t b, uint8_t c, uint8_t bitSize);

bool eq_small(uint8_t a, uint8_t b, uint8_t bitSize);
bool ne_small(uint8_t a, uint8_t b, uint8_t bitSize);
bool gt_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned);
bool ge_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned);
bool lt_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned);
bool le_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned);

uint8_t and_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t nand_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t or_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t nor_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t xor_small(uint8_t a, uint8_t b, uint8_t bitSize);
uint8_t xnor_small(uint8_t a, uint8_t b, uint8_t bitSize);

uint8_t sll_small(uint8_t value, uint8_t shift, uint8_t bitSize);
uint8_t srl_small(uint8_t value, uint8_t shift, uint8_t bitSize); // Logical Shift Right
uint8_t sra_small(uint8_t value, uint8_t shift, uint8_t bitSize); // Arithmetic Shift Right

//int main() {
//    // Example usage
//    uint8_t a = 0xF; // 4-bit number: 1111
//    uint8_t b = 0x9; // 4-bit number: 1001
//    uint8_t int_width = 2;
//    uint8_t size = pow(2,int_width);
//    bool isSigned = false;
//    for (uint8_t op=0; op<=21; op++) {
//        puts("");
//        for (uint8_t i=0; i<=size-1; i++) {
//            for (uint8_t j=0;  j<=size-1; j++) {
//                if (op == 0)  printf("%x + %x = %X\n", i, j, add_small(i, j, int_width));
//                if (op == 1)  printf("%x - %x = %X\n", i, j, sub_small(i, j, int_width));
//                if (op == 2)  printf("%x * %x = %X\n", i, j, (mulh_small(i, j, int_width) << int_width) + mul_small(i, j, int_width));
//                if (op == 3)  printf("%x / %x = %X\n", i, j, div_small(i, j, int_width));
//                if (op == 4)  printf("%x %% %x = %X\n", i, j, rem_small(i, j, int_width));
//                //if (k == 4) printf("%x %% %x = %X\n", i, j, !i|!j ? 0 : i%j); // of dividend or devisor zero, the rem is 0 (to bypass float exception)
//                for (uint8_t k=0; k<=size-1; k++) {
//                    if (op == 5)  printf("%x * %x + %x = %X\n", i, j, k, mul_add_small(i, j, k, int_width));
//                    if (op == 6)  printf("%x * %x - %x = %X\n", i, j, k, mul_sub_small(i, j, k, int_width));
//                }
//                if (op == 7)  printf("%d == %d (%s)\n", signExtend(i, int_width, isSigned), signExtend(j, int_width, isSigned), eq_small(i, j, int_width) ? "TRUE" :  "FALSE");
//                if (op == 8)  printf("%d != %d (%s)\n", signExtend(i, int_width, isSigned), signExtend(j, int_width, isSigned), ne_small(i, j, int_width) ? "TRUE" :  "FALSE");
//                if (op == 9)  printf("%d > %d  (%s)\n", signExtend(i, int_width, isSigned), signExtend(j, int_width, isSigned), gt_small(i, j, int_width, isSigned) ? "TRUE" :  "FALSE");
//                if (op == 10) printf("%d >= %d (%s)\n", signExtend(i, int_width, isSigned), signExtend(j, int_width, isSigned), ge_small(i, j, int_width, isSigned) ? "TRUE" :  "FALSE");
//                if (op == 11) printf("%d < %d  (%s)\n", signExtend(i, int_width, isSigned), signExtend(j, int_width, isSigned), lt_small(i, j, int_width, isSigned) ? "TRUE" :  "FALSE");
//                if (op == 12) printf("%d <= %d (%s)\n", signExtend(i, int_width, isSigned), signExtend(j, int_width, isSigned), le_small(i, j, int_width, isSigned) ? "TRUE" :  "FALSE");
//                if (op == 13) printf("%x & %x = %x\n",     i, j, and_small(i, j, int_width)  );
//                if (op == 14) printf("~(%x & %x) = %x\n",  i, j, nand_small(i, j, int_width) );
//                if (op == 15) printf("%x | %x = %x\n",     i, j, or_small(i, j, int_width)   );
//                if (op == 16) printf("~(%x | %x) = %x\n",  i, j, nor_small(i, j, int_width)  );
//                if (op == 17) printf("%x ^ %x = %x\n",     i, j, xor_small(i, j, int_width)  );
//                if (op == 18) printf("~(%x ^ %x) %x)\n",   i, j, xnor_small(i, j, int_width) );
//                if (op == 19) printf("srl(%x, %x) = %x\n", i, j, srl_small(i, j, int_width)  );
//                if (op == 20) printf("sra(%x, %x) = %x\n", i, j, sra_small(i, j, int_width)  );
//                if (op == 21) printf("sll(%x, %x) = %x\n", i, j, sll_small(i, j, int_width)  );
//            }
//        }
//    }
//    return 0;
//}

// Add function, making sure the result fits in the desired bit size
uint8_t add_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    uint8_t mask = (1 << bitSize) - 1;
    return (a + b) & mask;
}

// Subtract function, making sure the result fits in the desired bit size
uint8_t sub_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    uint8_t mask = (1 << bitSize) - 1;
    return (a - b) & mask;
}

// Multiply function, for 4-bit multiplication it takes the upper 4 bits of the 8-bit result
uint8_t mul_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    uint8_t product = a * b;
    if (bitSize == 4) {
        // For 4-bit, shift right to keep the upper 4 bits
        return product >> 4;
    } else {
        // For 1-bit, 2-bit, 3-bit, ensure the result fits in the desired bit size
        uint8_t mask = (1 << bitSize) - 1;
        return product & mask;
    }
}

// Multiply function, returns the upper half of the product
uint8_t mulh_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    uint16_t product = (uint16_t)a * (uint16_t)b; // Use a larger type to capture the full product
    uint8_t shiftAmount = bitSize * 2 - bitSize; // Calculate the shift amount to get the upper bits
    
    // Shift to get the upper half of the result based on the original bit size
    return (product >> shiftAmount) & ((1 << bitSize) - 1);
}

// Divide function, making sure the result fits in the desired bit size
uint8_t div_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    if (b == 0) {
        //printf("Error: Division by zero\n");
        return pow(2, bitSize)-1;
    }
    uint8_t mask = (1 << bitSize) - 1;
    return (a / b) & mask;
}

// Remainder operation
uint8_t rem_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    if (b == 0) {
        //printf("Error: Division by zero\n");
        return 0; // Error case
    }
    return maskValue(a % b, bitSize);
}

// mul_add function: Performs (a * b) + c
uint8_t mul_add_small(uint8_t a, uint8_t b, uint8_t c, uint8_t bitSize) {
    uint16_t result = a * b + c;; // Use a larger type to prevent overflow during operation
    // Apply bitSize limit
    uint8_t mask = (1 << bitSize) - 1;
    return (uint8_t)(result & mask);
}

// mul_sub function: Performs (a * b) - c
uint8_t mul_sub_small(uint8_t a, uint8_t b, uint8_t c, uint8_t bitSize) {
    uint16_t result = a * b - c; // Use a larger type to prevent overflow during operation
    // Apply bitSize limit
    uint8_t mask = (1 << bitSize) - 1;
    return (uint8_t)(result & mask);
}

// eq function: Checks if a is equal to b
bool eq_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return maskValue(a, bitSize) == maskValue(b, bitSize);
}

// ne function: Checks if a is not equal to b
bool ne_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return maskValue(a, bitSize) != maskValue(b, bitSize);
}

// gt function: Checks if a is greater than b
bool gt_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned) {
    if (isSigned) {
        int8_t maskedA = (int8_t)maskValue(a, bitSize);
        int8_t maskedB = (int8_t)maskValue(b, bitSize);
        // Adjust for sign extension
        int8_t signBit = 1 << (bitSize - 1);
        maskedA = (maskedA ^ signBit) - signBit;
        maskedB = (maskedB ^ signBit) - signBit;
        return maskedA > maskedB;
    } else {
        return maskValue(a, bitSize) > maskValue(b, bitSize);
    }
}

// ge function: Checks if a is greater than or equal to b
bool ge_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned) {
    return !lt_small(a, b, bitSize, isSigned);
}

// lt function: Checks if a is less than b
bool lt_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned) {
    if (isSigned) {
        int8_t maskedA = (int8_t)maskValue(a, bitSize);
        int8_t maskedB = (int8_t)maskValue(b, bitSize);
        // Adjust for sign extension
        int8_t signBit = 1 << (bitSize - 1);
        maskedA = (maskedA ^ signBit) - signBit;
        maskedB = (maskedB ^ signBit) - signBit;
        return maskedA < maskedB;
    } else {
        return maskValue(a, bitSize) < maskValue(b, bitSize);
    }
}

// le function: Checks if a is less than or equal to b
bool le_small(uint8_t a, uint8_t b, uint8_t bitSize, bool isSigned) {
    return !gt_small(a, b, bitSize, isSigned);
}

// AND operation
uint8_t and_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return maskValue(a, bitSize) & maskValue(b, bitSize);
}

// NAND operation
uint8_t nand_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return ~(maskValue(a, bitSize) & maskValue(b, bitSize)) & ((1 << bitSize) - 1);
}

// OR operation
uint8_t or_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return maskValue(a, bitSize) | maskValue(b, bitSize);
}

// NOR operation
uint8_t nor_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return ~(maskValue(a, bitSize) | maskValue(b, bitSize)) & ((1 << bitSize) - 1);
}

// XOR operation
uint8_t xor_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return maskValue(a, bitSize) ^ maskValue(b, bitSize);
}

// XNOR operation
uint8_t xnor_small(uint8_t a, uint8_t b, uint8_t bitSize) {
    return ~(maskValue(a, bitSize) ^ maskValue(b, bitSize)) & ((1 << bitSize) - 1);
}

// Shift left operation
uint8_t sll_small(uint8_t value, uint8_t shift, uint8_t bitSize) {
    return maskValue(value << shift, bitSize);
}

// Logical Shift Right operation
uint8_t srl_small(uint8_t value, uint8_t shift, uint8_t bitSize) {
    return maskValue(value >> shift, bitSize);
}

// Arithmetic Shift Right operation
uint8_t sra_small(uint8_t value, uint8_t shift, uint8_t bitSize) {
    if (bitSize == 8) { // Directly perform ASHR if 8-bit
        return (int8_t)value >> shift;
    }
    // For smaller bit sizes, need to handle sign extension manually
    int8_t mask = (1 << bitSize) - 1;
    int8_t signedValue = (int8_t)(value & mask); // Apply mask to simulate smaller bit size
    // Extend sign if negative
    if (signedValue & (1 << (bitSize - 1))) {
        signedValue |= ~mask;
    }
    return (signedValue >> shift) & mask;
}
