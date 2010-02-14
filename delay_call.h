// 遅延呼び出し機構
#pragma once

#include "event.h"
#include "dbg_timer.h"

namespace ssc {

#define DELAYCALL_USER_CLASS(c) typedef c THIS_CLASS_DELAYCALL;
#define DELAYCALL_CLASS_A0(delay_call, f) delay_call.add(this, &THIS_CLASS_DELAYCALL::f, L#f, __FILE__, __LINE__);
#define DELAYCALL_CLASS_A1(delay_call, f, a) delay_call.add(this, &THIS_CLASS_DELAYCALL::f, a, L#f, __FILE__, __LINE__);

// 遅延呼び出し機構に使用される呼び出し仮想クラス
class delay_call_item
{
public:
	enum { CNONE, CA0, CA1 } type;
	std::wstring name;
	std::string file;
	int line;

	virtual void operator()()=0;
	virtual ~delay_call_item(){}
};

// 遅延呼び出し機構のメインクラス
class delay_call
{
// 型宣言
public:
	enum CallStyle {
		CS_ONECALL, // 未呼び出しの関数、引数が同じものに関しては、1度しか呼び出さない。
		CS_ALLCALL, // 未呼び出しの関数、引数が同じものに関しても全て呼び出す。
	};

// イベント一覧
public:
	event _on_add;

// 関数
public:
	template<class T, class R>
	void add(T* t, R (T::*f)(), const wchar_t* name, const char* file, int line )
	{
		// 同じ呼び出しは無視する。
		add( new delay_call_item_class_a0<T,R>(t,f), name, file, line );
	}

	template<class T, class R, class A1>
	void add(T* t, R (T::*f)(A1), A1 a1, const wchar_t* name, const char* file, int line )
	{
		// 同じ呼び出しは無視する。
		add( new delay_call_item_class_a1<T,R,A1>(t,f,a1), name, file, line );
	}

	template<class T>
	void add(T* add_item, const wchar_t* name, const char* file, int line)
	{
		add_item->name = name;
		add_item->file = file;
		add_item->line = line;

		if( m_CallStyle == CS_ONECALL ) {
			bool find=false;
			for(int i=0; i<m_item.size(); i++) {
				if(m_item[i]->type == add_item->type) {
					T* d = (T*)m_item[i];
					if( *d == *add_item ) {
						find=true;
						break;
					}
				}
			}
			if(!find) {
				m_item.push_back(add_item);
				_on_add();
			}
			else {
				delete add_item;
			}
		}
		else {
			m_item.push_back(add_item);
			_on_add();
		}

	}

	// 呼び出しの実行
	void call()
	{
		dbg_timer.start("----- delay_call use time -----");
		for(int i=0; i<m_item.size(); i++) {
			(*m_item[i])();
			dbg_timer.lap_timediff(strprintf("%S", m_item[i]->name.c_str()).c_str(), __FILE__, __LINE__ );
			delete m_item[i];
		}
		m_item.clear();
		dbg_timer.report_at_timediff();
	}
	
// メンバ変数 ------------------------------------------------------------------
protected:
	std::vector<delay_call_item*> m_item;
	CallStyle m_CallStyle;
	ssc::dbg_timer dbg_timer;

public:
	// コンストラクタ	
	delay_call()
	{
		m_CallStyle = CS_ONECALL;
	}

};

template<class T>
class delay_call_item_class : public delay_call_item
{
protected:
	T* t;
	typedef void (T::*VOIDFUNC)();
	VOIDFUNC f;

	delay_call_item_class( T* t_a, VOIDFUNC f_a ) : t(t_a), f(f_a) { type = CNONE; }
	virtual ~delay_call_item_class(){}

public:
	bool operator == (delay_call_item_class& ref)
	{
		return ref.t == t && ref.f == f;
	}
};

template<class T, class R>
struct delay_call_item_class_a0 : public delay_call_item_class<T>
{
	typedef R (T::*THISFUNC)();
	delay_call_item_class_a0( T* t, THISFUNC f )
		: delay_call_item_class<T>(t, (VOIDFUNC)f)
	{
		type = CA0;
	}

	virtual void operator()()
	{
		(t->*(THISFUNC)f ) ();
	}
	virtual ~delay_call_item_class_a0(){}
};

template<class T, class R, class A1>
struct delay_call_item_class_a1 : public delay_call_item_class<T>
{
	typedef R (T::*THISFUNC)(A1);
	A1 a1;

	delay_call_item_class_a1( T* t, THISFUNC f, A1 a1_)
		: delay_call_item_class<T>(t, (VOIDFUNC)f)
		, a1(a1_)
	{
		 type = CA1;
	}

	virtual void operator()()
	{
		(t->*(THISFUNC)f ) (a1);
	}
	virtual ~delay_call_item_class_a1(){}

public:
	bool operator == (delay_call_item_class_a1& ref)
	{
		return ref.t == t && ref.f == f && ref.a1 == a1;
	}
};

} // namespace ssc
