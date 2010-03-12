#pragma once

// �N���X�̃C���N���[�h�ˑ��֌W�폜
// ����N���X���ق��̃N���X�̎Q�Ƃ����ꍇ�A
// ���̂�ێ�����ɂ̓C���N���[�h���K�v�ɂȂ�B
// 
// �C���X�^���X�Ǘ��N���X�Ȃǂł͑�ʂ̃C���N���[�h���s���A
// ���̃N���X���C���N���[�h����t�@�C���͂��ׂăR���p�C�����Ȃ����K�v������B
// 
// ���̃N���X�𗘗p���邱�ƂŁA���̂������o�ŁA�C���N���[�h���s�v�ɂȂ�B
// 
// �g�p���@�F
// class A {
//		scope_ptr<class B> m_pB; // class �������K�v����B
// };
template<class T>
class scope_ptr{
public:
	T* p;
	scope_ptr() : p(new T)	{	}
	~scope_ptr()			{ delete p;	}

	T* operator->()	const	{ return p; }
    T& operator*()  const	{ return *p;}
    T* get()		const	{ return p;}
};
