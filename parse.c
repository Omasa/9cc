#include"9cc.h"

char *user_input;
Token *token;

// エラーを報告するための関数
// printfと同じ引数をとる
void error(char *fmt, ...){
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	exit(1);
}

// エラー箇所の報告
void error_at(char *loc,char *fmt, ...){
	va_list ap;
	va_start(ap,fmt);

	int pos = loc - user_input;
	fprintf(stderr,"%s\n",user_input);
	fprintf(stderr,"%*s",pos,""); //print pos spaces
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

//次のトークンが期待している記号の時には、トークンを一つ読み進めて
//真を返す。それ以外の場合には偽を返す。
bool consume(char *op){
	if(token->kind != TK_RESERVED ||
			strlen(op) != token->len ||
			memcmp(token->str,op,token->len))
		return false;
	token = token->next;
	return true;
}

Token *consume_ident(){
	if(token->kind != TK_IDENT)
		return NULL;
	Token *t =token;
	token = token->next;
	return t;
}

void expect(char *op){
	if(token->kind != TK_RESERVED ||
			strlen(op) != token->len ||
			memcmp(token->str,op,token->len))
		error_at(token->str, "expected '%c'",op);
	token = token->next;
}

//次のトークンが数値の場合、トークンを一つ読み進めてその数値を返す。
//それ以外の場合にはエラーを報告する。
int expect_number(){
	if (token->kind != TK_NUM)
		error_at(token->str,"expected a number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof(){
	return token->kind == TK_EOF;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str,int len){
	Token *tok = calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p,char *q){
	return memcmp(p,q,strlen(q))== 0;
}

//変数を名前で検索する。見つからなかった場合はNULLを返す
LVar *find_lvar(Token *tok){
	for (LVar *var = locals; var; var = var->next)
		if(var->len == tok->len && !memcmp(tok->str,var->name, var->len))
			return var;
	return NULL;
}

// 入力文字列user_inputをトークナイズしてそれを返す
Token *tokenize(){
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p){
		if(isspace(*p)){
			p++;
			continue;
		}
		//複数文字
		if(startswith(p, "==") || startswith(p, "!=") ||
				startswith(p,"<=") || startswith(p,">=")){
			cur = new_token(TK_RESERVED,cur,p,2);
			p+= 2;
			continue;
		}

		//単記号
		if(strchr("+-*/()<>;",*p)){
			cur = new_token(TK_RESERVED,cur,p++,1);
			continue;
		}

		//単文字
		if('a' <= *p && *p <= 'z'){
			cur = new_token(TK_IDENT,cur,p++,1);
			cur->len=1;
			continue;
		}
		if(isdigit(*p)){
			cur = new_token(TK_NUM,cur,p,0);
			char *q =p;
			cur->val = strtol(p,&p,10);
			cur->len = p - q;
			continue; 
		}
		error_at(p, "invalid token");
	}
	new_token(TK_EOF,cur,p,0);
	return head.next;
}
