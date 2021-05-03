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

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr(){
	return equality();
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
	//次のトークンが"("なら、"(" expr ")"のはず
	if(consume("(")){
		Node *node = expr();
		expect(")");
		return node;
	}
	//そうでなければ数値のはず
	return new_node_num(expect_number());
}

void gen(Node *node){
	if(node->kind == ND_NUM){
		printf("  push %d\n",node->val);
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
void codegen(Node *node){

	// Print out the first half of assembly.
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");	

	//抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf("  pop rax\n");
	printf("  ret\n");
}
