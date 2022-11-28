#include "instruction.h"
#include <stdlib.h>
#include <string.h>

char *print_instruction(instruction_t *instr) {
    char *str = malloc(50);
    (void)instr;
    strncat(str, "abc", 50);
    return str;
}
