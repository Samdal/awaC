#ifndef AWA_PARSER_H_
#define AWA_PARSER_H_

#include "common.h"

static void awa_skip_to_next_valid(awa_parser_t* parser)
{
    while (!strchr(" AWaw\0", parser->awatalk[parser->pos])) parser->pos++;
}

// returns true on error
static bool awa_parse_bits(awa_parser_t* parser, uint8_t bits, uint8_t* out)
{
    *out = 0;

    for (int bit_iter = 0; bit_iter < bits; bit_iter++) {
        const char* awa_values[2] = {" awa", "wa"};
        int awa_v = 0;
        bool found = false;

        for (; awa_v < 2; awa_v++) {
            for (int i = 0; i < strlen(awa_values[awa_v]); i++) {
                awa_skip_to_next_valid(parser);
                if (tolower(parser->awatalk[parser->pos]) != awa_values[awa_v][i]) {
                    if (i == 0) goto continue_awa_v;
                    else        goto break_awa_v;
                }
                parser->pos++;
            }
            found = true;
            break;
        continue_awa_v:;
        }
    break_awa_v:;

        if (!found) {
            awa_line_info_t line = awa_get_line_and_col(*parser);
            if (parser->pos == parser->total_size)
                awa_debug_printf("%d:%d error: awa parser reached end of input\n", line.line+1, line.col);
            else
                awa_debug_printf("%d:%d error: awa parser got unexpected input starting here\n", line.line+1, line.col);
            awa_print_line(line);
            for (int i = 0; i<line.col;i++) awa_debug_printf(" ");
            awa_debug_printf("^~~\n");
            return true;
        }

        *out <<= 1;
        *out |= awa_v;
    }

    return false;
}

static awa_program_t awatalk_to_program(char* awatalk_string, bool print_program)
{
    awa_program_t res = {.source = awatalk_string};
    if (!awatalk_string) {
        awa_debug_printf("awatalk string was null!\n");
        res.failed_parse = true;
        return res;
    }

    awa_parser_t parser = {
        .awatalk = awatalk_string,
        .total_size = strlen(awatalk_string),
    };

    const char* awa_start = "awa";
    for (int i = 0; i < 3; i++) {
        awa_skip_to_next_valid(&parser);
        if (tolower(parser.awatalk[parser.pos]) != awa_start[i]) {
            awa_debug_printf("awatalk string does not start with \"awa\"!\n");
            res.failed_parse = true;
            return res;
        }
        parser.pos++;
    }

    parser.pos = 3;
    while (parser.pos < parser.total_size) {
        awa_skip_to_next_valid(&parser);
        awatism_t awatism = {.current_pos_bytes = parser.pos};
        bool opcode_err = awa_parse_bits(&parser, 5, &awatism.opcode);
        if (opcode_err) {
            awa_debug_printf("NOTE: awa parser error above happened while parsing an opcode\n");
            res.failed_parse = true;
            break;
        }
        if (awatism_opcode_names[awatism.opcode] == NULL) {
            awa_line_info_t line = awa_get_line_and_col((awa_parser_t){res.source, awatism.current_pos_bytes});
            awa_debug_printf("%d:%d error: opcode is invalid\n", line.line+1, line.col);
            awa_print_line(line);
            awa_debug_printf("^~~\n");
            res.failed_parse = true;
            break;
        }
        bool parameter_err = awa_parse_bits(&parser, awatism_param_sizes_bits[awatism.opcode], &awatism.u8);
        if (parameter_err) {
            awa_debug_printf("NOTE: awa parser error above happened while parsing a paramter of %s\n", awatism_opcode_names[awatism.opcode]);
            res.failed_parse = true;
            break;
        }
        if (print_program) {
            awa_print_awatism(awatism); awa_debug_printf("\n");
        }
        awa_skip_to_next_valid(&parser);

        awatism_t* n = malloc(sizeof(*n));
        *n = awatism;
        llist_queue_push(res.awatism_first, res.awatism_last, n);
        if (n->opcode == AWA_LABEL)
            res.awalabel_table[n->u8] = n;
    }

    return res;
}

awa_program_t awatalk_file_to_program(const char* filename, bool print_program) {
    char* buffer = 0;
    FILE* fp = fopen(filename, "rb");
    size_t sz = 0;
    if (fp) {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(sz + 1);
        if (buffer) fread(buffer, 1, sz, fp);
        fclose(fp);
        buffer[sz] = '\0';
    } else {
        fprintf(stderr, "unable to open file %s: %s\n", filename, strerror(errno));
    }
    return awatalk_to_program(buffer, print_program);
}

void awa_program_free(awa_program_t* prog) {
    free(prog->source);
    for (awatism_t *next, *a = prog->awatism_first; a; a = next) {
        next = a->next;
        free(a);
    }
    *prog = (awa_program_t){0};
}

#endif // AWA_PARSER_H_
