#include "9cc.h"

int main (int argc, char **argv){ 
	if(argc !=2){
		error("引数の個数が正しくありません");
		return 1;
	}

	//トークナイズしてパースする
	user_input = argv[1];
	tokenize();
	program();

	codegen();

	return 0;
}
