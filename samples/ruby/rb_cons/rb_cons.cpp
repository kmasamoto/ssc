// rb_cons.cpp : �R���\�[�� �A�v���P�[�V�����p�̃G���g�� �|�C���g�̒�`
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

