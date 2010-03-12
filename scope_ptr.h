#pragma once

// クラスのインクルード依存関係削除
// あるクラスがほかのクラスの参照を持つ場合、
// 実体を保持するにはインクルードが必要になる。
// 
// インスタンス管理クラスなどでは大量のインクルードを行い、
// そのクラスをインクルードするファイルはすべてコンパイルしなおす必要がある。
// 
// このクラスを利用することで、実体を持つ感覚で、インクルードが不要になる。
// 
// 使用方法：
// class A {
//		scope_ptr<class B> m_pB; // class を書く必要あり。
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
