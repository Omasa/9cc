#include"9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1,sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

void program();
Node *stmt();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *code[100];

Node *assign(){
	Node *node = equality();
	if(consume("="))
		node =new_node(ND_ASSIGN,node,assign());
	return node;
}

Node *expr(){
	return assign();
}

Node *stmt(){
	Node *node = expr();
	expect(";");
	return node;
}

void program(){
	int i = 0;
	while(!at_eof())
		code[i++] = stmt();
	code[i]= NULL;
}

Node *equality(){
	Node *node = relational();

	for(;;){
		if(consume("=="))
			node =new_node(ND_EQ,node,relational());
		else if(consume("!="))
			node = new_node(ND_NE,node,relational());
		else 
			return node;
	}
}

Node *relational(){
	Node *node = add();

	for(;;){
		if(consume("<"))
			node = new_node(ND_LT,node,add());
		else if(consume("<="))
			node = new_node(ND_LE,node,add());
		else if(consume(">"))
			node = new_node(ND_LT,add(),node);
		else if(consume(">="))
			node = new_node(ND_LE,add(),node);
		else 
			return node;
	}
}

Node *add(){
	Node *node = mul();

	for(;;){
		if(consume("+"))
			node = new_node(ND_ADD,node,mul());
		else if (consume("-"))
			node = new_node(ND_SUB,node,mul());
		else
			return node;
	}
}


Node *mul(){
	Node *node = unary();

	for(;;){
		if(consume("*"))
			node = new_node(ND_MUL,node,unary());
		else if (consume("/"))
			node = new_node(ND_DIV,node,unary());
		else
			return node;
	}
}

Node *unary(){
	if(consume("+"))
		return unary();
	if(consume("-"))
		return new_node(ND_SUB, new_node_num(0),unary());
	return primary();
}

Node *primary(){
	Token *tok = consume_ident();
	if(tok){
		Node *node = calloc(1,sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if(lvar){
			node->offset = lvar->offset;
		}
		else {
			lvar = calloc(1,sizeof(LVar));
			lvar->next = locals;
			lvar->name=tok->str;
			lvar->len=tok->len;
			lvar->offset = locals->offset+8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return node;
	}
	
	//次のトークンが"("なら、"(" expr ")"のはず
	if(consume("(")){
		Node *node = expr();
		expect(")");
		return node;
	}
	//そうでなければ数値のはず
	return new_node_num(expect_number());
}

void gen_lval(Node *node){
	if(node->kind != ND_LVAR)
		error("代入の左辺値が変数ではありません。");
	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n",node->offset);
	printf("  push rax\n");
}

void gen(Node *node){
	switch (node->kind){
		case ND_NUM:
			printf("  push %d\n",node->val);
			return;
		case ND_LVAR:
			gen_lval(node);
			printf("  pop rax\n");
			printf("  mov rax, [rax]\n");
			printf("  push rax\n");
			return;
		case ND_ASSIGN:
			gen_lval(node->lhs);
			gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind){
		case ND_ADD:
			printf("  add rax, rdi\n");
			break;
		case ND_SUB:
			printf("  sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("  imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("  cqo\n");
			printf("  idiv rdi\n");
			break;
		case ND_EQ:
			printf("  cmp rax, rdi\n");
			printf("  sete al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_NE:
			printf("  cmp rax, rdi\n");
			printf("  setne al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LT:
			printf("  cmp rax, rdi\n");
			printf("  setl al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LE:
			printf("  cmp rax, rdi\n");
			printf("  setle al\n");
			printf("  movzb rax, al\n");
			break;
	}
	printf("  push rax\n");
}

void codegen(){

	// Print out the first half of assembly.
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");	

	//プロローグ
	//変数26個分の領域を確保する
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, 208\n");

	//抽象構文木を下りながらコード生成
	for(int i=0; code[i]; i++){
	gen(code[i]);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf("  pop rax\n");
}
	//エピローグ
	//最後の四季の結果がRAXに残っているのでそれが帰り値になる
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
}
