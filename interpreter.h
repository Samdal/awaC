#ifndef AWA_INTERPRETER_H_
#define AWA_INTERPRETER_H_

#include "common.h"

static void awa_recursive_bubble_free(awa_bubble_t* bubble) {
    if (bubble) {
        for (awa_bubble_t *next, *b = bubble->first; b; b = next) {
            next = b->next;
            awa_recursive_bubble_free(b);
        }
        free(bubble);
    }
}
#define bubble_pop(prog) do {                       \
        awa_bubble_t* tmp = (prog)->bubble_first;   \
        llist_stack_pop((prog)->bubble_first);      \
        tmp->next = NULL;                           \
        awa_recursive_bubble_free(tmp);             \
    } while (0);


static void awa_recursive_print_bubble(awa_bubble_t* bubble) {
    if (bubble->first) {
        for (awa_bubble_t* b = bubble->first; b; b = b->next) {
            awa_recursive_print_bubble(b);
        }
    } else {
        if (bubble->val <= 63)
            putchar(awascii_table[bubble->val]);
    }
}
static void awa_recursive_print_num_bubble(awa_bubble_t* bubble, bool first) {
    if (bubble->first) {
        for (awa_bubble_t* b = bubble->first; b; b = b->next) {
            awa_recursive_print_num_bubble(b, false);
        }
    } else {
        if (!first) putchar(' ');
        printf("%d", bubble->val);
    }
}

static void awa_recursive_bubble_deep_copy(awa_bubble_t* to, awa_bubble_t* from) {
    for (awa_bubble_t* from_inside = from->first; from_inside; from_inside = from_inside->next) {
        awa_bubble_t* b = calloc(sizeof(*b), 1);
        awa_recursive_bubble_deep_copy(b, from_inside);
        llist_queue_push(to->first, to->last, b);
    }
    to->val = from->val;
}

enum awa_arithmetic_op {
    AWA_ARITH_ADD,
    AWA_ARITH_SUB,
    AWA_ARITH_MUL,
    AWA_ARITH_DIV,
};
static void awa_bubble_arithmetic_val(awa_bubble_t* new, awa_bubble_t* a, awa_bubble_t* b, enum awa_arithmetic_op op) {
    switch(op) {
    case AWA_ARITH_ADD: new->val = a->val + b->val; break;
    case AWA_ARITH_SUB: new->val = a->val - b->val; break;
    case AWA_ARITH_MUL: new->val = a->val * b->val; break;
    case AWA_ARITH_DIV: {
        awa_bubble_t* res = calloc(sizeof(*res), 1);
        res->val = a->val / b->val;
        awa_bubble_t* rem = calloc(sizeof(*rem), 1);
        rem->val = a->val % b->val;
        llist_queue_push(new->first, new->last, res);
        llist_queue_push(new->first, new->last, rem);
        break;
    }
    }
}

static awa_bubble_t* awa_recursive_bubble_arithmetic(awa_bubble_t* a, awa_bubble_t* b, enum awa_arithmetic_op op) {
    awa_bubble_t* new = calloc(sizeof(*new), 1);
    if (!a->first && !b->first) {
        awa_bubble_arithmetic_val(new, a, b, op);
    } else if (a->first && !b->first) {
        for (awa_bubble_t* a_inside = a->first; a_inside; a_inside = a_inside->next) {
            awa_bubble_t* new_inside = awa_recursive_bubble_arithmetic(a_inside, b, op);
            llist_queue_push(new->first, new->last, new_inside);
        }
    } else if (!a->first && b->first) {
        for (awa_bubble_t* b_inside = b->first; b_inside; b_inside = b_inside->next) {
            awa_bubble_t* new_inside = awa_recursive_bubble_arithmetic(a, b_inside, op);
            llist_queue_push(new->first, new->last, new_inside);
        }
    } else {
        for (awa_bubble_t* b_inside = b->first, *a_inside = a->first; b_inside && a_inside; b_inside = b_inside->next, a_inside = a_inside->next) {
            awa_bubble_t* new_inside = awa_recursive_bubble_arithmetic(a_inside, b_inside, op);
            llist_queue_push(new->first, new->last, new_inside);
        }
    }
    return new;
}

static void awa_bubble_arithmetic(awa_program_t* program, enum awa_arithmetic_op op) {
    if (program->bubble_first) {
        if (program->bubble_first->next) {
            awa_bubble_t* new = awa_recursive_bubble_arithmetic(program->bubble_first, program->bubble_first->next, op);
            new->next = program->bubble_first->next->next;

            awa_recursive_bubble_free(program->bubble_first->next);
            awa_recursive_bubble_free(program->bubble_first);
            program->bubble_first = new;
        }
    }
}

enum awa_cmp_op {
    AWA_CMP_EQ,
    AWA_CMP_LESS,
    AWA_CMP_GRTR,
};
static bool awa_bubble_cmp_val(awa_bubble_t* a, awa_bubble_t* b, enum awa_cmp_op op) {
    switch(op) {
    case AWA_CMP_EQ: return a->val == b->val;
    case AWA_CMP_LESS: return a->val < b->val;
    case AWA_CMP_GRTR: return a->val > b->val;
    }
    return false;
}
static bool awa_recursive_bubble_cmp(awa_bubble_t* a, awa_bubble_t* b, enum awa_cmp_op op) {
    if (!a->first && !b->first) {
        if (!awa_bubble_cmp_val(a, b, op))
            return false;
    } else if (a->first && !b->first) {
        for (awa_bubble_t* a_inside = a->first; a_inside; a_inside = a_inside->next) {
            if (!awa_recursive_bubble_cmp(a_inside, b, op))
                return false;
        }
    } else if (!a->first && b->first) {
        for (awa_bubble_t* b_inside = b->first; b_inside; b_inside = b_inside->next) {
            if (!awa_recursive_bubble_cmp(a, b_inside, op))
                return false;
        }
    } else {
        awa_bubble_t* b_inside = b->first, *a_inside = a->first;
        for (; b_inside && a_inside; b_inside = b_inside->next, a_inside = a_inside->next) {
            if (!awa_recursive_bubble_cmp(a_inside, b_inside, op))
                return false;
        }
        if (a_inside != b_inside)
            return false;
    }
    return true;
}
static void awa_bubble_cmp(awa_program_t* program, enum awa_cmp_op op) {
    if (program->bubble_first) {
        if (program->bubble_first->next) {
            if (!awa_recursive_bubble_cmp(program->bubble_first, program->bubble_first->next, op)) {
                program->cmp_skip_next_flag = true;
            }
        }
    }
}

static size_t awa_recursive_get_bubble_count(awa_bubble_t* bubble) {
    if (bubble->first) {
        size_t count = 0;
        for (awa_bubble_t* b = bubble->first; b; b = b->next)
            count += awa_recursive_get_bubble_count(b);
        return count;
    } else {
        return 1;
    }
}
static void awa_recursive_to_bytes(awa_bubble_t* bubble, uint8_t** bytes) {
    if (bubble->first) {
        for (awa_bubble_t* b = bubble->first; b; b = b->next)
            awa_recursive_to_bytes(b, bytes);
    } else {
        **bytes = bubble->val;
        *bytes += 1;
    }
}
static size_t awa_get_syscall_param(bool* is_ptr_out, awa_bubble_t* bubble) {
    if (bubble->first) {
        size_t count = awa_recursive_get_bubble_count(bubble);
        uint8_t* ptr = malloc(count);
        {
            uint8_t* tmp = ptr;
            awa_recursive_to_bytes(bubble, &tmp);
        }
        *is_ptr_out = true;
        return (size_t)ptr;
    } else {
        *is_ptr_out = false;
        return bubble->val;
    }
}

static void awa_program_run(awa_program_t program, bool print_debug, bool step_through, bool enable_syscalls) {
    for (awatism_t* current_awatism = program.awatism_first; current_awatism; current_awatism = current_awatism->next) {
    rerun_current_awatism:;
        if (print_debug) {
            printf("\t<<<");
            printf("(");
            for (awa_bubble_t* b = program.bubble_first; b; b = b->next) {
                if (b != program.bubble_first) printf(", ");
                awa_recursive_debug_print_bubble(b);
            }
            printf(")");
            printf(" | ");
            awa_print_awatism(*current_awatism);
            if (program.cmp_skip_next_flag)
            printf(" | SKIPPING");
            printf(">>>\n");
        }
        if (step_through)
            getchar();

        if (program.cmp_skip_next_flag) {
            program.cmp_skip_next_flag = false;
            continue;
        }
        switch((enum awatisms)current_awatism->opcode) {
        case AWA_NOP: {
            // nothing
        } break;
        case AWA_PRINT: {
            if (program.bubble_first) {
                awa_recursive_print_bubble(program.bubble_first);
                program.last_print_was_number_flag = false;
                bubble_pop(&program);
            }
        } break;
        case AWA_PRINT_NUM: {
            if (program.bubble_first) {
                awa_recursive_print_num_bubble(program.bubble_first, !program.last_print_was_number_flag);
                program.last_print_was_number_flag = true;
                bubble_pop(&program);
            }
        } break;
        case AWA_READ: {
            printf(">>>AWA INTERPRETER IS REQUESTING INPUT(text): ");
            char buf[1024] = {0};
            fgets(buf, sizeof(buf), stdin);
            int i = 0;
            awa_bubble_t* new = calloc(sizeof(*new), 1);
            for (; buf[i] && buf[i] != '\n'; i++) {
                uint8_t c = awascii_table_inverse[(uint8_t)buf[i]];
                if (c && buf[i] != 'A') {
                    awa_bubble_t* b = calloc(sizeof(*b), 1);
                    b->val = c;
                    llist_queue_push(new->first, new->last, b);
                }
            }
            if (new->first) llist_stack_push(program.bubble_first, new);
            else            free(new);
            printf(">>>AWA INTERPRETER READ INPUT [%.*s]\n", i, buf);
        } break;
        case AWA_READ_NUM: {
            printf(">>>AWA INTERPRETER IS REQUESTING INPUT(numb): ");
            char buf[1024] = {0};
            fgets(buf, sizeof(buf), stdin);
            awa_bubble_t* b = calloc(sizeof(*b), 1);
            sscanf(buf, "%d", &b->val);
            llist_stack_push(program.bubble_first, b);
            printf(">>>AWA INTERPRETER READ INPUT [%d]\n", b->val);
        } break;
        case AWA_TERMINATE: {
            goto program_end;
        } break;
        case AWA_BLOW: {
            awa_bubble_t* b = calloc(sizeof(*b), 1);
            b->val = current_awatism->s8;
            llist_stack_push(program.bubble_first, b);
        } break;
        case AWA_SUBMERGE: {
            if (program.bubble_first && program.bubble_first->next) {
                awa_bubble_t* sub = program.bubble_first;
                int depth = current_awatism->u8;
                llist_stack_pop(program.bubble_first);

                awa_bubble_t* last = program.bubble_first;
                int i = 1;
                for (awa_bubble_t* b = last->next; b && i != depth; b = b->next, i++) {
                    last = b;
                }
                sub->next = last->next;
                last->next = sub;
            }
        } break;
        case AWA_POP: {
            if (program.bubble_first) {
                awa_bubble_t* tmp = program.bubble_first;
                llist_stack_pop(program.bubble_first);
                if (tmp->first) {
                    while (tmp->first) {
                        if (tmp->first->next == NULL) {
                            llist_stack_push(program.bubble_first, tmp->first);
                            break;
                        }
                        for (awa_bubble_t* b = tmp->first; b; b = b->next) {
                            if (b->next->next == NULL) {
                                llist_stack_push(program.bubble_first, b->next);
                                b->next = NULL;
                                break;
                            }
                        }
                    }
                }
                free(tmp);
            }
        } break;
        case AWA_DUPLICATE: {
            if (program.bubble_first) {
                awa_bubble_t* b = calloc(sizeof(*b), 1);
                awa_recursive_bubble_deep_copy(b, program.bubble_first);
                llist_stack_push(program.bubble_first, b);
            }
        } break;
        case AWA_SURROUND: {
            if (program.bubble_first && current_awatism->u8) {
                awa_bubble_t* b = calloc(sizeof(*b), 1);
                for (int i = 0; i < current_awatism->u8; i++) {
                    awa_bubble_t* b_inside = program.bubble_first;
                    if (!b_inside) break;
                    llist_stack_pop(program.bubble_first);
                    b_inside->next = NULL;
                    llist_queue_push(b->first, b->last, b_inside);
                }
                llist_stack_push(program.bubble_first, b);
            }
        } break;
        case AWA_MERGE: {
            awa_bubble_t* two[2];
            if (program.bubble_first) {
                two[0] = program.bubble_first;
                if (program.bubble_first->next) {
                    two[1] = program.bubble_first->next;

                    if (!two[0]->first && !two[1]->first) {
                        two[0]->val += two[1]->val;
                        two[0]->next = two[1]->next;
                        free(two[1]);
                    } else {
                        awa_bubble_t* new = calloc(sizeof(*new), 1);
                        new->next = two[1]->next;
                        program.bubble_first = new;

                        for (int i = 0; i < 2; i++) {
                            if (two[i]->first) {
                                for (awa_bubble_t* next, *b = two[i]->first; b; b = next) {
                                    next = b->next;
                                    llist_queue_push(new->first, new->last, b);
                                }
                                free(two[i]);
                            } else {
                                llist_queue_push(new->first, new->last, two[i]);
                            }
                        }
                    }
                }
            }
        } break;
        case AWA_ADD: {
            awa_bubble_arithmetic(&program, AWA_ARITH_ADD);
        } break;
        case AWA_SUBTRACT: {
            awa_bubble_arithmetic(&program, AWA_ARITH_SUB);
        } break;
        case AWA_MULTIPLY: {
            awa_bubble_arithmetic(&program, AWA_ARITH_MUL);
        } break;
        case AWA_DIVIDE: {
            awa_bubble_arithmetic(&program, AWA_ARITH_DIV);
        } break;
        case AWA_COUNT: {
            if (program.bubble_first) {
                int count = 0;
                for (awa_bubble_t* b = program.bubble_first->first; b; b = b->next) {
                    count++;
                }
                awa_bubble_t* b = calloc(sizeof(*b), 1);
                b->val = count;
                llist_stack_push(program.bubble_first, b);
            }
        } break;
        case AWA_LABEL: {
            // nothing
        } break;
        case AWA_JUMP: {
            if (program.awalabel_table[current_awatism->u8]) {
                current_awatism = program.awalabel_table[current_awatism->u8];
                goto rerun_current_awatism;
            } else {
                awa_debug_printf("interpreter: unable to find label %d\n", current_awatism->u8);
            }
        } break;
        case AWA_EQUAL_TO: {
            awa_bubble_cmp(&program, AWA_CMP_EQ);
        } break;
        case AWA_LESS_THAN: {
            awa_bubble_cmp(&program, AWA_CMP_LESS);
        } break;
        case AWA_GREATER_THAN: {
            awa_bubble_cmp(&program, AWA_CMP_GRTR);
        } break;
        case AWA_SYSCALL: {
#ifndef awa_syscall
            awa_debug_printf("syscalls are not supported on this platform, terminating\n");
            goto program_end;
#endif
            if (!enable_syscalls) {
                awa_debug_printf("syscalls are intentionally disabled!");
                goto program_end;
            }
            if (program.bubble_first) {
                int count = 0;
                for (awa_bubble_t* b = program.bubble_first->first; b; b = b->next)
                    count++;
                if (count > 0 && count <= 7) {
                    size_t args[6] = {0};
                    bool is_ptr[6] = {0};
                    {
                        awa_bubble_t* b = program.bubble_first->first;
                        for (int i = 0; i < count; i++, b = b->next)
                            args[i] = awa_get_syscall_param(&is_ptr[i], b);
                    }

                    long val = -1;
                    switch(count) {
                    case 1: val = awa_syscall(current_awatism->u16, args[0]); break;
                    case 2: val = awa_syscall(current_awatism->u16, args[0], args[1]); break;
                    case 3: val = awa_syscall(current_awatism->u16, args[0], args[1], args[2]); break;
                    case 4: val = awa_syscall(current_awatism->u16, args[0], args[1], args[2], args[3]); break;
                    case 5: val = awa_syscall(current_awatism->u16, args[0], args[1], args[2], args[3], args[4]); break;
                    case 6: val = awa_syscall(current_awatism->u16, args[0], args[1], args[2], args[3], args[4], args[5]); break;
                    }

                    for (int i = 0; i < count; i++)
                        if (is_ptr[i]) free((void*)args[i]);
                    awa_bubble_t* n = calloc(sizeof(*n), 1);
                    n->val = val;
                    llist_stack_push(program.bubble_first, n);
                } else {
                    awa_debug_printf("syscall(%d) had an invalid number of arguments %d\n", current_awatism->u16, count);
                    goto program_end;
                }
            }
        } break;
        case AWA_DOUBLE_POP: {
            if (program.bubble_first)
                bubble_pop(&program);
        } break;
        default: {
            awa_debug_printf("interpreter: invalid opcode %d\n", current_awatism->opcode);
        } break;
        }
    }
program_end:;
    while(program.bubble_first)
        bubble_pop(&program);
}


#endif // AWA_INTERPRETER_H_
