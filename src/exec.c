#include "cc_sakura.h"
#define Ok		 (0)
#define Return	 (-1)
#define Break	 (-2)
#define Continue (-3)

#define SEGMENT_SIZE 3000

int control				 = Ok;
int sp					 = 0;
int fp					 = 0;
char stack[SEGMENT_SIZE] = {0};
Node *args				 = NULL;

void not_implemented(void) {
	printf("not implemented\n");
	exit(1);
}

int expand_next(Node *node) {
	int ret;
	while(node) {
		ret	 = exec(node);
		node = node->next;
	}

	return ret;
}

int expand_block_code(Node *node) {
	int ret;
	while(node) {
		ret = exec(node);
		if(control == Return) {
			control = Ok;
			return ret;
		}
		if(control == Break || control == Continue) {
			control = Ok;
			return Ok;
		}

		node = node->block_code;
	}

	return ret;
}

void *exec_address(Node *node) {
	if(node->kind == ND_DEREF)
        not_implemented();
	else if(node->kind == ND_DOT)
        not_implemented();
	else if(node->kind == ND_ARROW)
        not_implemented();
	else if(node->kind == ND_GVAR)
        not_implemented();
	else if(node->kind == ND_LVAR)
		return lookup_lvar(node);
	else
		error_at(token->str, "can not assign");
}

void *lookup_lvar(Node *node) {
	if((fp + node->offset + node->len) > SEGMENT_SIZE) error_at(token->str, "stack size exceed");
	if(node->kind != ND_LVAR && node->kind != ND_CALL_FUNC) {
		error_at(token->str, "not a variable");
	}

	sp += node->len;

	return (void *)(stack + node->offset);
}

int exec_args(Node *args) {
	int reg_num;
	int arg_count = 0;

	while(args) {
		exec_expr(args);
		arg_count++;
		args = args->block_code;
	}

	for(reg_num = arg_count; reg_num > 0; reg_num--) {
		printf("	pop rax\n");
		// printf("	mov %s,rax\n", reg[reg_num-1]);
	}
	printf("	mov rax,%d\n", arg_count);
}

int exec_calc(Node *node) {
	int left  = exec_expr(node->lhs);
	int right = exec_expr(node->rhs);

	switch(node->kind) {
		case ND_ADD:
			return left + right;
		case ND_SUB:
			return left - right;
		case ND_MUL:
			return left * right;
		case ND_DIV:
			return left / right;
		case ND_MOD:
			return left % right;
		case ND_GT:
			return left > right;
		case ND_GE:
			return left >= right;
		case ND_LT:
			return left < right;
		case ND_LE:
			return left <= right;
		case ND_EQ:
			return left == right;
		case ND_NE:
			return left != right;
		case ND_BIT_AND:
			return left & right;
		case ND_BIT_OR:
			return left | right;
		case ND_NOT:
			return !right;
		default:
			error_at(token->str, "cannot code gen");
	}
}

int exec_expr(Node *node) {
	void *lhs_ptr = NULL;
	int reg_ty;
	int reg_rty;

	if(node && node->type) reg_ty = (int)node->type->ty;
	if(node->rhs && node->rhs->type) reg_rty = (int)node->rhs->type->ty;

	switch(node->kind) {
		case ND_NUM:
			return node->val;
		case ND_CAST:
			if(reg_ty > reg_rty) {
				if(reg_rty == BOOL) {
					return (bool)exec_expr(node->rhs);
				} else if(reg_rty == CHAR) {
					return (char)exec_expr(node->rhs);
				} else if(reg_rty == INT) {
					return (int)exec_expr(node->rhs);
				}
			}
			return Ok;
		case ND_GVAR:
            not_implemented();
		case ND_LVAR:
			switch(node->type->ty) {
				case CHAR:
					return *(char *)lookup_lvar(node);
				case INT:
				case ENUM:
					return *(int *)lookup_lvar(node);
				case LONG:
				case PTR:
				case ARRAY:
					return *(long *)lookup_lvar(node);
			}
		case ND_PREID:
			// ++p -> p += 1
			exec(node->lhs);
			return Ok;
		case ND_POSTID:
			lhs_ptr = exec_address(node->lhs);
			switch(node->rhs->type->ty) {
				case CHAR:
					*(char *)lhs_ptr = (char)exec_calc(node->rhs->rhs);
					break;
				case INT:
				case ENUM:
					*(int *)lhs_ptr = (int)exec_calc(node->rhs->rhs);
					break;
				case LONG:
				case PTR:
				case ARRAY:
					*(long *)lhs_ptr = (long *)exec_calc(node->rhs->rhs);
					break;
			}
			return Ok;
		case ND_STR:
			not_implemented();
		case ND_ASSIGN:
			lhs_ptr = exec_address(node->lhs);
			switch(node->rhs->type->ty) {
				case CHAR:
					*(char *)lhs_ptr = (char)exec_expr(node->rhs);
					break;
				case INT:
				case ENUM:
					*(int *)lhs_ptr = (int)exec_expr(node->rhs);
					break;
				case LONG:
				case PTR:
				case ARRAY:
					*(long *)lhs_ptr = (long *)exec_expr(node->rhs);
					break;
			}
			return Ok;
		case ND_COMPOUND:
			lhs_ptr = exec_address(node->lhs);
			switch(node->rhs->type->ty) {
				case CHAR:
					*(char *)lhs_ptr = (char)exec_calc(node->rhs);
					break;
				case INT:
				case ENUM:
					*(int *)lhs_ptr = (int)exec_calc(node->rhs);
					break;
				case LONG:
				case PTR:
				case ARRAY:
					*(long *)lhs_ptr = (long *)exec_calc(node->rhs);
					break;
			}
			return Ok;
		case ND_DOT:
		case ND_ARROW:
			not_implemented();
		case ND_TERNARY:
			// condition
			if(exec_expr(node->lhs)) {
				// true
				exec(node->rhs);
			} else {
				// false
				exec(node->next);
			}
			return Ok;
		case ND_AND:
			return exec_expr(node->lhs) && exec_expr(node->rhs);
		case ND_OR:
			return exec_expr(node->lhs) || exec_expr(node->rhs);
		case ND_NOT:
			return !exec_expr(node->rhs);
		case ND_ADDRESS:
		case ND_DEREF:
			not_implemented();
		case ND_CALL_FUNC:
			int ret = exec_func(node->str, node->len, node->rhs);
			return ret;
		default:
			return exec_calc(node);
	}
}

int exec(Node *node) {
	int ret;
	Node *cases;

	int reg_rty;
	if(node->rhs && node->rhs->type)
		reg_rty = (int)node->rhs->type->ty;

	// generate assembly
	switch(node->kind) {
		case ND_NULL_STMT:
			// NULL statement
			return Ok;
		case ND_IF:
			if(exec(node->lhs)) {
				ret = exec(node->rhs);
			}

			return ret;
		case ND_IFELSE:
			// condition
			if(exec(node->lhs)) {
				// expr in if
				ret = exec(node->rhs->lhs);
			} else {
				// expr in else
				ret = exec(node->rhs->rhs);
			}

			return ret;
		case ND_SWITCH:
			not_implemented();
			return Ok;
		case ND_FOR:
			// init
			exec(node->lhs);

			// condition
			while(exec(node->lhs->next)) {
				// gen block
				exec(node->rhs);

				// gen update expression
				exec(node->lhs->next->next);
			}

			return Ok;
		case ND_WHILE:
			// condition
			while(exec(node->lhs)) {
				// else expression
				exec(node->rhs);
			}
			return Ok;
		case ND_DOWHILE:
			do {
				// codeblock
				exec(node->rhs);

				// condition
			} while(exec(node->lhs));
			return Ok;
		case ND_CONTINUE:
			control = Continue;
			return Ok;
		case ND_BREAK:
			control = Break;
			return Ok;
		case ND_CASE:
			not_implemented();
		case ND_ARG:
			while(node) {
				if(node->rhs && node->rhs->type)
					reg_rty = (int)node->rhs->type->ty;

				void *lhs_ptr = lookup_lvar(node->rhs);
				switch(node->rhs->type->ty) {
					case CHAR:
						*(char *)lhs_ptr = (char)exec_expr(args);
						break;
					case INT:
					case ENUM:
						*(int *)lhs_ptr = (int)exec_expr(args);
						break;
					case LONG:
					case PTR:
					case ARRAY:
						*(long *)lhs_ptr = (long *)exec_expr(args);
						break;
				}
				node = node->next;
				args = args->block_code;
			}
			return Ok;
		case ND_BLOCK:
			return expand_block_code(node->rhs);
		case ND_RETURN:
			control = Return;
			return exec_expr(node->rhs);
		default:
			return exec_expr(node);
	}
}

int exec_func(char *func_name, int len, Node *func_args) {
	int i;
	int j;
	int ret;

	for(i = 0; func_list[i]; i++) {
		if(strncmp(func_list[i]->name, func_name, len) == 0) {
			for(j = 0; func_list[i]->code[j] != NULL; j++) {
				int prev_fp = fp;
				fp			= sp;

				control = Ok;
				if(func_list[i]->args) {
					// set local variable
					args = func_args;
					exec(func_list[i]->args);
				}
				ret = exec(func_list[i]->code[j]);

				sp = fp;
				fp = prev_fp;

				if(control == Return) break;
			}

			control = Ok;
			return ret;
		}
	}
}

void exec_main(void) {
	int ret = exec_func("main", 4, NULL);
	printf("%d\n", ret);
}
