#include "cc_sakura.h"
#define Ok		 (0)
#define Return	 (-1)
#define Break	 (-2)
#define Continue (-3)

int control = Ok;

int expand_next(Node *node){
    int ret;
	while(node){
		ret = exec(node);
		node = node->next;
	}

    return ret;
}

int expand_block_code(Node *node){
    int ret;
	while(node){
		ret = exec(node);
        if (control == Return) return ret;
        if (control == Break) return Ok;
        if (control == Continue) return Ok;

		node = node->block_code;
	}

    return ret;
}


void exec_gvar_label(GVar *gvar, Node *init){
	Type *type = get_pointer_type(gvar->type);
	if(init->kind == ND_STR){
		if(gvar->type->ty == PTR){
			printf("	.quad	.LC%d\n", init->val);
		}else if(gvar->type->ty == ARRAY){
			printf("	.string \"%.*s\"\n", init->len, init->str);
			if(init->offset) printf("        .zero	%d\n", init->offset);
		}
	}else{
		if(type->ty < INT){
			printf("	.byte	%d\n", init->val);
		}else{
			printf("	.long	%d\n", init->val);
		}
	}
}

int exec_address(Node *node){
	/**/ if(node->kind == ND_DEREF)   exec_expr(node->rhs);
	else if(node->kind == ND_DOT)     exec_struc(node);
	else if(node->kind == ND_ARROW)   exec_struc(node);
	else if(node->kind == ND_GVAR)    exec_gvar(node);
	else if(node->kind == ND_LVAR)    exec_lvar(node);
	else error_at(token->str, "can not assign");
}


int exec_gvar(Node *node){
	if(node->type->is_thread_local){
		printf("	mov rax, fs:0\n");
		printf("	add rax, fs:%.*s@tpoff\n", node->len, node->str);
	}else{
		printf("	lea rax,%.*s[rip]\n", node->len, node->str);
	}
	printf("	push rax\n");
}

int exec_lvar(Node *node){
	if(node->kind != ND_LVAR && node->kind != ND_CALL_FUNC){
		error_at(token->str,"not a variable");
	}

	printf("	mov rax,rbp\n");
	printf("	sub rax,%d\n", node->offset);
	printf("	push rax\n");
}

int exec_struc(Node *node){
	if(node->kind != ND_DOT && node->kind != ND_ARROW){
		error_at(token->str, "not a struct");
	}

	exec_expr(node->lhs);
	exec_expr(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");
	printf("	add rax,rdi\n");
	printf("	push rax\n");
}

int exec_args(Node *args){
	int reg_num;
	int arg_count = 0;

	while(args){
		exec_expr(args);
		arg_count++;
		args=args->block_code;
	}

	for(reg_num = arg_count;reg_num > 0;reg_num--){
		printf("	pop rax\n");
		//printf("	mov %s,rax\n", reg[reg_num-1]);
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
			return exec_gvar(node);
		case ND_LVAR:
			return exec_lvar(node);
		case ND_PREID:
			// ++p -> p += 1
			exec(node->lhs);
			return Ok;
		case ND_POSTID:
			/*
			// push
			exec_address(node->lhs);			// push lhs
			exec_expr(node->rhs->rhs->rhs);	// push rhs

			// calc
			printf("	pop rdi\n");		// rhs
			printf("	pop rax\n");		// lhs
			printf("	push [rax]\n");		// Evacuation lhs data
			printf("	push rax\n");		// Evacuation lhs address
			printf("	mov rax,[rax]\n");	// deref lhs

			exec_calc(node->rhs->rhs);
			printf("	push rax\n");  // rhs op lhs

			// assign
			printf("	pop rdi\n");  // src
			printf("	pop rax\n");  // dst
			if(node->lhs->type->ty == BOOL) {
				printf("	mov R8B,dil\n");
				printf("	cmp R8B,0\n");
				printf("	setne dl\n");
				printf("	movzb rdi,dl\n");
			}
			printf("	mov [rax],%s\n", reg_di[reg_lty]);

			// already evacuated
			// printf("	push rax\n");
			*/
			return Ok;
		case ND_STR:
			/*
			printf("	lea rax, .LC%d[rip]\n", node->val);
			printf("	push rax\n");
			*/
			return Ok;
		case ND_ASSIGN:
			exec_address(node->lhs);
			exec_expr(node->rhs);
			return Ok;
		case ND_COMPOUND:
			// push
			exec_address(node->lhs);	   // push lhs
			exec_expr(node->rhs->rhs);  // push rhs

			// calc
			printf("	pop rdi\n");		// rhs
			printf("	pop rax\n");		// lhs
			printf("	push rax\n");		// Evacuation lhs
			printf("	mov rax,[rax]\n");	// deref lhs

			exec_calc(node->rhs);
			printf("	push rax\n");  // rhs op lhs

			// assign
			printf("	pop rdi\n");  // src
			printf("	pop rax\n");  // dst
			if(node->lhs->type->ty <= CHAR) {
				if(node->lhs->type->ty == BOOL) {
					printf("	mov R8B,dil\n");
					printf("	cmp R8B,0\n");
					printf("	setne dl\n");
					printf("	movzb rdi,dl\n");
				}
				printf("	mov [rax],dil\n");
			} else if(node->lhs->type->ty == INT) {
				printf("	mov [rax],edi\n");
			} else {
				printf("	mov [rax],rdi\n");
			}

			printf("	push rdi\n");
			return Ok;
		case ND_DOT:
		case ND_ARROW:
			return exec_struc(node);
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
		/*
		case ND_ADDRESS:
			exec_address(node->rhs);	 // printf("	push rax\n");
			return Ok;
		case ND_DEREF:
			exec(node->rhs);
			if(node->type->ty != ARRAY && node->type->ty != STRUCT) {
				if(node->type->ty <= CHAR) {
					printf("        mov al,BYTE PTR [rax]\n");
				} else {
					printf("	mov %s,[rax]\n", reg_ax[reg_ty]);
				}
			}
			printf("	push rax\n");
			return Ok;
		*/
		case ND_CALL_FUNC:
			exec_args(node->rhs);
			return Ok;
		default:
			// check left hand side
			//exec_expr(node->lhs);
			// check right hand side
			//exec_expr(node->rhs);
			// calculation lhs and rhs
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
			/*
			// gen cases condtion
			cases = node->next;
			while(cases) {
				exec(cases);

				printf("	cmp %s,0\n", reg_ax[cases->type->ty]);
				printf("	jne .LcaseBegin%03d\n", cases->val);
				cases = cases->next;
			}

			// gen default condtion
			if(node->lhs) {
				printf("	jmp .LcaseBegin%03d\n", node->lhs->val);
			}
			printf("	jmp .LloopEnd%03d\n", node->val);

			// gen code block
			exec(node->rhs);

			printf(".LloopEnd%03d:\n", node->val);
			*/
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
			/*
			printf(".LcaseBegin%03d:\n", node->val);
			exec(node->rhs);
			*/
			return Ok;
		case ND_ARG:
			/*
			while(node) {
				if(node->rhs && node->rhs->type)
					reg_rty = (int)node->rhs->type->ty;
				// push register argument saved
				printf("        push %s\n", reg[node->val]);
				exec_lvar(node->rhs);
				printf("	pop rax\n");
				printf("	pop rdi\n");
				printf("	mov [rax],%s\n", reg_di[reg_rty]);
				node = node->next;
			}
			*/
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

void exec_main(void) {
	int i;
	int j;
    int ret;

	for(i = 0; func_list[i]; i++) {
		if(strncmp(func_list[i]->name, "main", 4) == 0) {
			for(j = 0; func_list[i]->code[j] != NULL; j++) {
                control = Ok;
				ret = exec(func_list[i]->code[j]);
                if (control == Return) break;
			}

            printf("%d\n", ret);
            return;
		}
	}
}
