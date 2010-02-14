#ifndef event_hpp
#define event_hpp

#pragma warning(disable:4786) // テンプレート警告削除
#include <vector>
#include <list>
#include <assert.h>
#include "ssc_string.h"
#include <set>
#include "dbg_timer.h"

#ifdef _DEBUG
	#define EVENT_TRACE 0
#else
	#define EVENT_TRACE 0
#endif

namespace ssc {

// クラスでデリゲート登録を行う際のヘルパ
#define EVENT_USER_CLASS(c) typedef c THIS_CLASS; ssc::event_register EVENT_REGISTER;
#define EVENT_CALL_CLASS(d, m) EVENT_REGISTER.callon(d,this, &THIS_CLASS::m, L#d L", " L#m, __FILE__, __LINE__)
#define EVENT_CALL_EVENT(d, call) EVENT_REGISTER.callon(d, call, L#d L", "  L#call, __FILE__, __LINE__)
#define EVENT_NOTCALL_CLASS(d, m) EVENT_REGISTER.notcall(d,this)
#define EVENT_NOTCALL_EVENT(d, call) EVENT_REGISTER.notcall(d, call)

// 引数制限ありのイベント定義を行うヘルパ
// 使用例
// EVENT_VA2(void, _OnClick, int, x, int, y); _OnClick_t _OnClick;
// EVENT_VA2(void, _OnClick, int, x, int, y); _OnClick_t _OnClick;
#define EVENT_VA2(R, F, A1T, A1V, A2T, A2V) \
	struct F##_arg { A1T A1V; A2T A2V; }; \
	typedef ssc::eventarg_va2<F##_arg, A1T, A2T> F##_t

#define EVENT_VA3(R, F, A1T, A1V, A2T, A2V, A3T, A3V) \
	struct F##_arg { A1T A1V; A2T A2V; A3T A3V; }; \
	typedef ssc::eventarg_va3< F##_arg, A1T, A2T, A3T> F##_t

#define EVENT_RA5(R, D, F, A1T, A1V, A2T, A2V, A3T, A3V, A4T, A4V, A5T, A5V) \
	struct F##_arg { R ret; A1T A1V; A2T A2V; A3T A3V; A4T A4V; A5T A5V;}; \
	typedef ssc::eventarg_ra5< F##_arg, A1T, A2T, A3T, A4T, A5T, R,D> F##_t

#define EVENT_RA6(R, D, F, A1T, A1V, A2T, A2V, A3T, A3V, A4T, A4V, A5T, A5V, A6T, A6V) \
	struct F##_arg { R ret; A1T A1V; A2T A2V; A3T A3V; A4T A4V; A5T A5V; A6T A6V;}; \
	typedef ssc::eventarg_ra6< F##_arg, A1T, A2T, A3T, A4T, A5T, A6T, R,D> F##_t

// イベント受信クラス
class event_listener
{
protected:
	class event* m_pEvent; // 登録先イベント
	std::wstring m_name; // 登録関数名

#ifdef _DEBUG
public:
	const char* file;
	int   line;
#endif

public:
	// イベントアイテム基本処理
	event_listener(const wchar_t* name) { m_pEvent=0; m_name=name; } // コンストラクタ
	virtual ~event_listener() { } // デストラクタ
	virtual void regist(class event* pEvent) { assert(m_pEvent==0); m_pEvent = pEvent; } // イベント登録した際に呼び出される。 ベース機能としては、1つしか登録先許可しません。
	virtual void unregist(class event* pEvent) { m_pEvent = 0; } // イベント登録した際に呼び出される。 ベース機能としては、1つしか登録先許可しません。

	// オーバーライド
	virtual void call(void*)=0;					// 登録したイベントが発生したときに呼び出される。
	virtual void event_delete(class event*)=0;	// 登録したイベントが削除される時に呼び出される。
	virtual void dbg_trace_event(){};			// イベントの登録箇所をトレース出力する。

	std::wstring& name(){ return m_name; };
	#ifdef _DEBUG
	virtual void reg_loc()=0;					// 登録されたロケーションを OutputDebugString する。
	#endif
};

// イベントクラス
class event
{
protected:
	std::vector<event_listener*> m_item;
	ssc::dbg_timer tim;

public:
	// 追加
	operator+=(event_listener* p)
	{
		m_item.push_back(p);
		p->regist(this);
	}
	// 削除
	operator-=(event_listener* p)
	{
		for(int i=0;i<m_item.size();i++) {
			if(m_item[i] == p) {
				m_item.erase(&m_item[i]);
				p->unregist(this);
				return 1;
			}
		}
		p->unregist(this);
		return 0;
	}

	// 終了
	~event()
	{
		for(int i=0; i<m_item.size(); i++) {
			m_item[i]->event_delete(this);
		}
	}

	// 呼び出し処理
	virtual void operator() ()
	{
		(*this)(0);
	}

	// 呼び出し処理
	virtual void operator() (void* pParam, const char* file=__FILE__, int line = __LINE__)
	{
		//char buf[1024];sprintf(buf, "%s(%d) : 登録箇所\n", file, line); OutputDebugStringA(buf);
		if(m_item.empty()) return;

		tim.start("----- event call use time -----");
		for(int i=0; i<m_item.size(); i++) {
			#if EVENT_TRACE
				m_item[i]->reg_loc();
			#endif
			m_item[i]->call(pParam);
			#ifdef _DEBUG
				tim.lap_timediff(strprintf("%s\n%s(%d):", wstrtostr(m_item[i]->name()).c_str(), m_item[i]->file, m_item[i]->line ).c_str(), __FILE__, __LINE__ );
			#endif
		}
		tim.report_at_timediff();
	}

	// イベントの登録箇所をトレース出力する。
	virtual void dbg_trace_event()
	{
	}
};

// イベントクラス(フォーマット書式)
class eventf : public event
{
// フォーマット
public:
	virtual void operator() (const wchar_t* fmt, ...)
	{
		if(m_item.empty()) return;

		wchar_t buf[4096];
		va_snwprintf(buf,4096,&fmt);
		(*this)((void*)buf);
	}

private: // インターフェイス隠匿
	// 呼び出し処理
	virtual void operator() (void* pParam) { this->event::operator()(pParam); }
};

// 遅延イベント
class event_delay : public event
{
private:
	std::set<void*> m_setParam;

public:
	virtual void operator() (void* pParam)
	{
		m_setParam.insert(pParam);
	}

	void exec()
	{
		std::set<void*>::iterator it = m_setParam.begin();
		for(; it != m_setParam.end(); it++ )
		{
			this->event::operator()(*it);
		}
	}
};

// 引数固定イベント向け
class eventarg : public event
{
// インターフェイスの隠匿
protected:
	virtual void operator() (void* pParam, const char* file=__FILE__, int line = __LINE__)
	{
		this->event::operator()(pParam,file,line);
	}
};
// 引数固定イベント 戻り値なし 引数２
template<typename ARG, typename A1, typename A2>
class eventarg_va2 : public eventarg {
public: virtual void operator() (A1 a1, A2 a2) { ARG a={a1,a2}; this->event::operator()(&a); }
};

// 引数固定イベント 戻り値なし 引数３
template<typename ARG, typename A1, typename A2, typename A3>
class eventarg_va3 : public eventarg {
public: virtual void operator() (A1 a1, A2 a2, A3 a3) { ARG a={a1,a2,a3}; this->event::operator()(&a); }
};

// 引数固定イベント 戻り値あり 引数５
template<typename ARG, typename A1, typename A2, typename A3, typename A4, typename A5, typename R, R DEF>
class eventarg_ra5 : public eventarg {
public: virtual R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) { ARG a={DEF, a1,a2,a3,a4,a5}; this->event::operator()(&a); return a.ret; }
};

// 引数固定イベント 戻り値あり 引数６
template<typename ARG, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R, R DEF>
class eventarg_ra6 : public eventarg {
public: virtual R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) { ARG a={DEF, a1,a2,a3,a4,a5,a6}; this->event::operator()(&a); return a.ret; }
};

// クラス関数の呼び出し
// テンプレート化不要の内容はこちらに記述
class event_listener_for_register : public event_listener
{
public:
	class event_register* m_pEventRegister; // 登録先イベント削除のイベント

	// コンストラクタ
	event_listener_for_register(const wchar_t* name) : event_listener(name)
	{
		#ifdef _DEBUG
			m_pEventRegister=0;
			file=0;
			line=0;
		#endif
	}

	virtual ~event_listener_for_register()
	{
		// 削除時には登録先イベントから削除する。
		if( m_pEvent != 0 ) {
			(*m_pEvent) -= this;
		}
	}

	// 登録したイベントが削除される時に呼び出される。
	virtual void event_delete(class event* pEvent);
	
	// イベントの登録箇所をトレース出力する。
	virtual void dbg_trace_event()
	{
		#ifdef _DEBUG
			char buf[1024];sprintf(buf, "%s(%d) : 登録箇所\n", file, line); OutputDebugStringA(buf);
			m_pEvent->dbg_trace_event();
		#endif
	}

	// 登録されたロケーションを OutputDebugString する。
	#ifdef _DEBUG
	virtual void reg_loc()
	{
		char buf[1024];sprintf(buf, "%s(%d) : 登録箇所\n", file, line); OutputDebugStringA(buf);
	}
	#endif
};

// クラス関数の呼び出し
template<typename T, typename A>
class event_listener_for_class_a1 : public event_listener_for_register
{
	T* m_p; // 呼び出し先クラス
	void (T::*m_pFunc)(A p); // 呼び出し先関数
public:
	event_listener_for_class_a1(T* p, void (T::*pFunc)(A a), const wchar_t* name, class event_register* pEventRegister=0) : event_listener_for_register(name)
	{
		m_p = p;
		m_pFunc = pFunc;
		m_pEventRegister = pEventRegister;
	}

	// 登録したイベントが発生したときに呼び出される。
	virtual void call(void* p)
	{
		(m_p->*m_pFunc)((A)p);
	}
};

// クラス関数の呼び出し
template<typename T>
class event_listener_for_class : public event_listener_for_register
{
	T* m_p; // 呼び出し先クラス
	void (T::*m_pFunc)(); // 呼び出し先関数
public:
	event_listener_for_class(T* p, void (T::*pFunc)(), const wchar_t* name,  class event_register* pEventRegister=0)  : event_listener_for_register(name)
	{
		m_p = p;
		m_pFunc = pFunc;
		m_pEventRegister = pEventRegister;
	}

	// 登録したイベントが発生したときに呼び出される。
	virtual void call(void* p)
	{
		(m_p->*m_pFunc)();
	}
};

// クラス関数の呼び出し
template<typename T, typename A>
class event_listener_for_class_with_callclass : public event_listener_for_register
{
	T* m_p; // 呼び出し先クラス
	void (T::*m_pFunc)(A a, event_listener* p_caller); // 呼び出し先関数
public:
	event_listener_for_class_with_callclass(T* p, void (T::*pFunc)(A a, event_listener* p_caller), const wchar_t* name,  class event_register* pEventRegister=0)  : event_listener_for_register(name)
	{
		m_p = p;
		m_pFunc = pFunc;
		m_pEventRegister = pEventRegister;
	}

	// 登録したイベントが発生したときに呼び出される。
	virtual void call(void* p)
	{
		(m_p->*m_pFunc)((A)p, this);
	}
};

// イベントの呼び出し
class event_listener_fop_event : public event_listener_for_register
{
public:
	event* m_pEvent; // 呼び出し先イベント
	event_listener_fop_event(event* pEvent, const wchar_t* name,  class event_register* pEventRegister=0)  : event_listener_for_register(name)
	{
		m_pEvent = pEvent;
		m_pEventRegister = pEventRegister;
	}

	// 登録したイベントが発生したときに呼び出される。
	virtual void call(void* p)
	{
		(*m_pEvent)(p);
	}
};

// イベント登録・管理クラス
// 管理下のクラスは、本クラスの削除時に登録解除する。
class event_register
{
	std::list<event_listener_for_register*> m_owner_item;
	std::list<event_listener_fop_event*> m_forevent_item;

public:
	// 登録処理
	void callon(event& e, event_listener_for_register* p, const char* file, int line)
	{
		#ifdef _DEBUG
			p->file = file;
			p->line = line;
		#endif

		e += p; // イベント登録
		m_owner_item.push_back(p); // 管理下のアイテムとして追加
	}

	// イベント登録処理
	template<typename T, typename A>
	void callon(event& e, T* pThis, void (T::*pFunc)(A a, event_listener* p_caller), const wchar_t* name, const char* file, int line)
	{
		event_listener_for_register* p = new event_listener_for_class_with_callclass<T, A>(pThis, pFunc, name, this); // 呼び出しアイテムの生成
		callon(e, p, file, line);
	}

	// イベント登録処理
	template<typename T, typename A>
	void callon(event& e, T* pThis, void (T::*pFunc)(A a), const wchar_t* name, const char* file, int line)
	{
		event_listener_for_register* p = new event_listener_for_class_a1<T, A>(pThis, pFunc, name, this); // 呼び出しアイテムの生成
		callon(e, p, file, line);
	}

	// イベント登録処理
	template<typename T>
	void callon(event& e, T* pThis, void (T::*pFunc)(), const wchar_t* name, const char* file, int line)
	{
		event_listener_for_register* p = new event_listener_for_class<T>(pThis, pFunc, name, this); // 呼び出しアイテムの生成
		callon(e, p, file, line);
	}

	void callon(event& e, event& call, const wchar_t* name, const char* file, int line)
	{
		event_listener_fop_event* p = new event_listener_fop_event(&call, name, this); // 呼び出しアイテムの生成
		callon(e, p, file, line);
	}

	void notcall(event& e, event& call)
	{
		bool bFind = false; // 呼び出し
		// 呼び出し削除
		std::list<event_listener_fop_event*>::iterator it = m_forevent_item.begin();
		for(; it != m_forevent_item.end(); it++) {
			if( (*it)->m_pEvent == &call) {
				bFind = true;
				break;
			}
		}

		if ( bFind ) {
			e -= (*it);
		}
	}

	// イベント削除のコールバック
	void event_delete(event_listener_for_register* pEventItem)
	{
		m_owner_item.remove(pEventItem);
		m_forevent_item.remove((event_listener_fop_event*)pEventItem);
		/*
		for(int i=0;i<m_owner_item.size();i++) {
			if(m_owner_item[i] == pEventItem) {
				m_owner_item.erase(&m_owner_item[i]);
				break;
			}
		}
		*/

		delete pEventItem;
	}

	// このクラス削除時の処理
	~event_register()
	{
		std::list<event_listener_for_register*>::iterator it =m_owner_item.begin();
		for(; it != m_owner_item.end(); it++) {
			delete (*it);
		}
	}
};

inline void event_listener_for_register::event_delete(class event* pEvent)
{
	if(m_pEventRegister != 0 ) {
		m_pEventRegister->event_delete(this); // イベント管理を呼び出し
	}
}

}

#endif // #ifndef event_hpp
