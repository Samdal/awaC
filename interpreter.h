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
#define bubble_pop(prog) do {						\
		awa_bubble_t* tmp = (prog)->bubble_first;	\
		llist_stack_pop((prog)->bubble_first);		\
		awa_recursive_bubble_free(tmp);				\
	} while (0);


static void awa_recursive_print_bubble(awa_bubble_t* bubble) {
	if (bubble->first) {
		for (awa_bubble_t* b = bubble->first; b; b = b->next) {
			awa_recursive_print_bubble(b);
		}
	} else {
		putchar(awascii_table[bubble->val & 63]);
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
static int awa_bubble_arithmetic_val(awa_bubble_t* a, awa_bubble_t* b, enum awa_arithmetic_op op) {
	switch(op) {
	case AWA_ARITH_ADD: return a->val + b->val;
	case AWA_ARITH_SUB: return a->val - b->val;
	case AWA_ARITH_MUL: return a->val * b->val;
	case AWA_ARITH_DIV: return a->val / b->val;
	}
	return 0;
}

static awa_bubble_t* awa_recursive_bubble_arithmetic(awa_bubble_t* a, awa_bubble_t* b, enum awa_arithmetic_op op) {
	awa_bubble_t* new = calloc(sizeof(*new), 1);
	if (!a->first && !b->first) {
		new->val = awa_bubble_arithmetic_val(a, b, op);
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

static void awa_program_run(awa_program_t program, bool print_debug) {
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
			printf(">>>\n");
		}

		if (program.cmp_skip_next_flag) {
			program.cmp_skip_next_flag = false;
			continue;
		}
		switch((enum awatisms)current_awatism->opcode) {
		case AWA_NOP: {
			// nothing
		} break;
		case AWA_PRINT: {
			awa_recursive_print_bubble(program.bubble_first);
			program.last_print_was_number_flag = false;
			bubble_pop(&program);
		} break;
		case AWA_PRINT_NUM: {
			awa_recursive_print_num_bubble(program.bubble_first, !program.last_print_was_number_flag);
			program.last_print_was_number_flag = true;
			bubble_pop(&program);
		} break;
		case AWA_READ: {
			printf(">>>AWA INTERPRETER IS REQUESTING INPUT(text): ");
			char buf[1024] = {0};
			fgets(buf, sizeof(buf), stdin);
			for (int i = 0; buf[i]; i++) {
				uint8_t c = awascii_table_inverse[(uint8_t)buf[i]];
				if (c && buf[i] != 'A') {
					awa_bubble_t* b = calloc(sizeof(*b), 1);
					b->val = c;
					llist_stack_push(program.bubble_first, b);
				}
			}
			printf(">>>AWA INTERPRETER IS READ INPUT [%s]\n", buf);
		} break;
		case AWA_READ_NUM: {
			printf(">>>AWA INTERPRETER IS REQUESTING INPUT(numb): ");
			char buf[1024] = {0};
			fgets(buf, sizeof(buf), stdin);
			awa_bubble_t* b = calloc(sizeof(*b), 1);
			sscanf(buf, "%d", &b->val);
			llist_stack_push(program.bubble_first, b);
			printf(">>>AWA INTERPRETER IS READ INPUT [%d]\n", b->val);
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
			if (program.bubble_first) {
				awa_bubble_t* sub = program.bubble_first;
				int depth = current_awatism->u8;
				program.bubble_first = program.bubble_first->next;

				awa_bubble_t* last = program.bubble_first;
				int i = 1;
				for (awa_bubble_t* b = program.bubble_first->next; b && i != depth; b = b->next, i++) {
					last = b;
				}
				sub->next = last->next;
				last->next = sub;
			}
		} break;
		case AWA_POP: {
			bubble_pop(&program);
		} break;
		case AWA_DUPLICATE: {
			if (program.bubble_first) {
				awa_bubble_t* b = calloc(sizeof(*b), 1);
				awa_recursive_bubble_deep_copy(b, program.bubble_first);
				llist_stack_push(program.bubble_first, b);
			}
		} break;
		case AWA_SURROUND: {
			awa_bubble_t* b = calloc(sizeof(*b), 1);
			for (int i = 0; i < current_awatism->u8; i++) {
				awa_bubble_t* b_inside = program.bubble_first;
				if (!b_inside) break;
				llist_stack_pop(program.bubble_first);
				llist_queue_push(b->first, b->last, b_inside);
			}
			llist_stack_push(program.bubble_first, b);
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
				for (awa_bubble_t* b = program.bubble_first; b; b = b->next) {
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
