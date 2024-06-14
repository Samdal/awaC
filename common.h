#ifndef AWA_COMMON_H_
#define AWA_COMMON_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

static char awascii_table[64] = "AWawJELYHOSIUMjelyhosiumPCNTpcntBDFGRbdfgr0123456789 .,!'()~_/;\n";
static uint8_t awascii_table_inverse[256] = {
    ['A'] = 0, ['W'] = 1, ['a'] = 2, ['w'] = 3,
    ['J'] = 4, ['E'] = 5, ['L'] = 6, ['Y'] = 7,
    ['H'] = 8, ['O'] = 9, ['S'] = 10, ['I'] = 11, ['U'] = 12, ['M'] = 13,
    ['j'] = 14, ['e'] = 15, ['l'] = 16, ['y'] = 17,
    ['h'] = 18, ['o'] = 19, ['s'] = 20, ['i'] = 21, ['u'] = 22, ['m'] = 23,
    ['P'] = 24, ['C'] = 25, ['N'] = 26, ['T'] = 27,
    ['p'] = 28, ['c'] = 29, ['n'] = 30, ['t'] = 31,
    ['B'] = 32, ['D'] = 33, ['F'] = 34, ['G'] = 35, ['R'] = 36,
    ['b'] = 37, ['d'] = 38, ['f'] = 39, ['g'] = 40, ['r'] = 41,
    ['0'] = 42, ['1'] = 43, ['2'] = 44, ['3'] = 45, ['4'] = 46, ['5'] = 47, ['6'] = 48, ['7'] = 49, ['8'] = 50, ['9'] = 51,
    [' '] = 52, ['.'] = 53, [','] = 54, ['!'] = 55, ['\''] = 56, ['('] = 57, [')'] = 58, ['~'] = 59, ['_'] = 60, ['/'] = 61, [';'] = 62, ['\n'] = 63,
};

enum awatisms {
    AWA_NOP             = 0x00,
    AWA_PRINT           = 0x01,
    AWA_PRINT_NUM       = 0x02,
    AWA_READ            = 0x03,
    AWA_READ_NUM        = 0x04,
    AWA_TERMINATE       = 0x1F,

    AWA_BLOW            = 0x05,
    AWA_SUBMERGE        = 0x06,
    AWA_POP             = 0x07,
    AWA_DUPLICATE       = 0x08,
    AWA_SURROUND        = 0x09,
    AWA_MERGE           = 0x0A,

    AWA_ADD             = 0x0B,
    AWA_SUBTRACT        = 0x0C,
    AWA_MULTIPLY        = 0x0D,
    AWA_DIVIDE          = 0x0E,
    AWA_COUNT           = 0x0F,

    AWA_LABEL           = 0x10,
    AWA_JUMP            = 0x11,
    AWA_EQUAL_TO        = 0x12,
    AWA_LESS_THAN       = 0x13,
    AWA_GREATER_THAN    = 0x14,

    AWA_SYSCALL         = 0x15,
    AWA_DOUBLE_POP      = 0x16,

};
#define AWA_OPCODE_MAX 32

#ifdef __linux
#include "sys/syscall.h"
#include "unistd.h"
#define awa_syscall(...) syscall(__VA_ARGS__)
#endif

static char* awatism_opcode_names[AWA_OPCODE_MAX] = {
    [AWA_NOP]           = "nop",
    [AWA_PRINT]         = "prn",
    [AWA_PRINT_NUM]     = "pr1",
    [AWA_READ]          = "red",
    [AWA_READ_NUM]      = "r3d",
    [AWA_TERMINATE]     = "trm",

    [AWA_BLOW]          = "blo",
    [AWA_SUBMERGE]      = "sbm",
    [AWA_POP]           = "pop",
    [AWA_DUPLICATE]     = "dpl",
    [AWA_SURROUND]      = "srn",
    [AWA_MERGE]         = "mrg",

    [AWA_ADD]           = "4dd",
    [AWA_SUBTRACT]      = "sub",
    [AWA_MULTIPLY]      = "mul",
    [AWA_DIVIDE]        = "div",
    [AWA_COUNT]         = "cnt",

    [AWA_LABEL]         = "lbl",
    [AWA_JUMP]          = "jmp",
    [AWA_EQUAL_TO]      = "eql",
    [AWA_LESS_THAN]     = "lss",
    [AWA_GREATER_THAN]  = "gr8",

    [AWA_SYSCALL]       = "sys",
    [AWA_DOUBLE_POP]    = "p0p",
};

static uint8_t awatism_param_sizes_bits[AWA_OPCODE_MAX] = {
    [AWA_BLOW]      = 8,
    [AWA_SUBMERGE]  = 5,
    [AWA_SURROUND]  = 5,
    [AWA_LABEL]     = 5,
    [AWA_JUMP]      = 5,
    [AWA_SYSCALL]   = 10,
};

typedef struct awatism awatism_t;
 struct awatism {
    uint8_t opcode;
    union {
        uint8_t    u8;
        int8_t     s8;
        uint16_t  u16;
    };

    awatism_t* next;

    int current_pos_bytes;
};

#define llist_queue_push(f,l,n) ((f)==0) ? (f)=(l)=(n) : ((l)->next=(n), (l)=(n), (n)->next=0)
#define llist_queue_pop(f,l) ((f)==(l)) ? ((f)=(l)=0) : ((f)=(f)->next)

#define llist_stack_push(f,n) ((n)->next=(f), (f)=(n))
#define llist_stack_pop(f) ((f)==0) ? 0 : ((f)=(f->next))

#define awa_debug_printf(...) printf(__VA_ARGS__)

typedef struct awa_bubble awa_bubble_t;
struct awa_bubble {
    awa_bubble_t* next;
    awa_bubble_t* first;
    awa_bubble_t* last;
    int val;
};

typedef struct {
    bool failed_parse;

    char* source;

    awatism_t* awatism_first;
    awatism_t* awatism_last;

    awatism_t* awalabel_table[64];

    awa_bubble_t* bubble_first;

    bool cmp_skip_next_flag;
    bool last_print_was_number_flag;
} awa_program_t;

typedef struct {
    const char* awatalk;
    int pos;
    int total_size;
} awa_parser_t;

typedef struct {
    const char* line_start;
    int col;
    int line;
    int line_len;
} awa_line_info_t;

static awa_line_info_t awa_get_line_and_col(awa_parser_t parser)
{
    awa_line_info_t res = {0};
    res.line_start = parser.awatalk;
    for (;;) {
        const char* current_newline = strchr(res.line_start == parser.awatalk ? parser.awatalk : res.line_start, '\n');
        if (!current_newline) break;
        if (parser.pos < current_newline - parser.awatalk) break;
        res.line_start = current_newline + 1;
        res.line++;
    }
    const char* newline_after = strchr(res.line_start, '\n');
    if (newline_after) res.line_len = newline_after - res.line_start - 1;
    else               res.line_len = strlen(res.line_start);
    res.col = parser.pos - (res.line_start - parser.awatalk);
    return res;
}

static void awa_print_line(awa_line_info_t line)
{
    awa_debug_printf("%.*s\n", line.line_len+1, line.line_start);
}

static void awa_print_awatism(awatism_t awatism) {
    awa_debug_printf("%s", awatism_opcode_names[awatism.opcode]);
    switch (awatism.opcode) {
    case AWA_BLOW:
    {
        if (awatism.u8 < 64) {
            if (awascii_table[awatism.u8] == '\n')
                awa_debug_printf(" %d [\\n]", (int)awatism.s8);
            else
                awa_debug_printf(" %d [%c]", (int)awatism.s8, awascii_table[awatism.s8]);
        } else {
            awa_debug_printf(" %d", (int)awatism.s8);
        }
        break;
    } break;
    case AWA_SUBMERGE:
    case AWA_SURROUND:
    case AWA_LABEL:
    case AWA_JUMP:
    case AWA_SYSCALL:
    {
        awa_debug_printf(" %d", (int)awatism.u8);
        break;
    } break;
    }
}

static void awa_recursive_debug_print_bubble(awa_bubble_t* bubble) {
    if (bubble->first) {
        awa_debug_printf("(");
        for (awa_bubble_t* b = bubble->first; b; b = b->next) {
            if (b != bubble->first) awa_debug_printf(", ");
            awa_recursive_debug_print_bubble(b);
        }
        awa_debug_printf(")");
    } else {
        awa_debug_printf("%d", bubble->val);
    }
}


#endif // AWA_COMMON_H_
