// rb_cons.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "ruby.h"

int main(int argc, char* argv[])
{
    ruby_init();
    rb_eval_string("puts 'Hello'");

	printf("Hello World!\n");
	return 0;
}

