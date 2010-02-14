#ifndef event_hpp
#define event_hpp

#pragma warning(disable:4786) // �e���v���[�g�x���폜
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

// �N���X�Ńf���Q�[�g�o�^���s���ۂ̃w���p
#define EVENT_USER_CLASS(c) typedef c THIS_CLASS; ssc::event_register EVENT_REGISTER;
#define EVENT_CALL_CLASS(d, m) EVENT_REGISTER.callon(d,this, &THIS_CLASS::m, L#d L", " L#m, __FILE__, __LINE__)
#define EVENT_CALL_EVENT(d, call) EVENT_REGISTER.callon(d, call, L#d L", "  L#call, __FILE__, __LINE__)
#define EVENT_NOTCALL_CLASS(d, m) EVENT_REGISTER.notcall(d,this)
#define EVENT_NOTCALL_EVENT(d, call) EVENT_REGISTER.notcall(d, call)

// ������������̃C�x���g��`���s���w���p
// �g�p��
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

// �C�x���g��M�N���X
class event_listener
{
protected:
	class event* m_pEvent; // �o�^��C�x���g
	std::wstring m_name; // �o�^�֐���

#ifdef _DEBUG
public:
	const char* file;
	int   line;
#endif

public:
	// �C�x���g�A�C�e����{����
	event_listener(const wchar_t* name) { m_pEvent=0; m_name=name; } // �R���X�g���N�^
	virtual ~event_listener() { } // �f�X�g���N�^
	virtual void regist(class event* pEvent) { assert(m_pEvent==0); m_pEvent = pEvent; } // �C�x���g�o�^�����ۂɌĂяo�����B �x�[�X�@�\�Ƃ��ẮA1�����o�^�拖���܂���B
	virtual void unregist(class event* pEvent) { m_pEvent = 0; } // �C�x���g�o�^�����ۂɌĂяo�����B �x�[�X�@�\�Ƃ��ẮA1�����o�^�拖���܂���B

	// �I�[�o�[���C�h
	virtual void call(void*)=0;					// �o�^�����C�x���g�����������Ƃ��ɌĂяo�����B
	virtual void event_delete(class event*)=0;	// �o�^�����C�x���g���폜����鎞�ɌĂяo�����B
	virtual void dbg_trace_event(){};			// �C�x���g�̓o�^�ӏ����g���[�X�o�͂���B

	std::wstring& name(){ return m_name; };
	#ifdef _DEBUG
	virtual void reg_loc()=0;					// �o�^���ꂽ���P�[�V������ OutputDebugString ����B
	#endif
};

// �C�x���g�N���X
class event
{
protected:
	std::vector<event_listener*> m_item;
	ssc::dbg_timer tim;

public:
	// �ǉ�
	operator+=(event_listener* p)
	{
		m_item.push_back(p);
		p->regist(this);
	}
	// �폜
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

	// �I��
	~event()
	{
		for(int i=0; i<m_item.size(); i++) {
			m_item[i]->event_delete(this);
		}
	}

	// �Ăяo������
	virtual void operator() ()
	{
		(*this)(0);
	}

	// �Ăяo������
	virtual void operator() (void* pParam, const char* file=__FILE__, int line = __LINE__)
	{
		//char buf[1024];sprintf(buf, "%s(%d) : �o�^�ӏ�\n", file, line); OutputDebugStringA(buf);
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

	// �C�x���g�̓o�^�ӏ����g���[�X�o�͂���B
	virtual void dbg_trace_event()
	{
	}
};

// �C�x���g�N���X(�t�H�[�}�b�g����)
class eventf : public event
{
// �t�H�[�}�b�g
public:
	virtual void operator() (const wchar_t* fmt, ...)
	{
		if(m_item.empty()) return;

		wchar_t buf[4096];
		va_snwprintf(buf,4096,&fmt);
		(*this)((void*)buf);
	}

private: // �C���^�[�t�F�C�X�B��
	// �Ăяo������
	virtual void operator() (void* pParam) { this->event::operator()(pParam); }
};

// �x���C�x���g
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

// �����Œ�C�x���g����
class eventarg : public event
{
// �C���^�[�t�F�C�X�̉B��
protected:
	virtual void operator() (void* pParam, const char* file=__FILE__, int line = __LINE__)
	{
		this->event::operator()(pParam,file,line);
	}
};
// �����Œ�C�x���g �߂�l�Ȃ� �����Q
template<typename ARG, typename A1, typename A2>
class eventarg_va2 : public eventarg {
public: virtual void operator() (A1 a1, A2 a2) { ARG a={a1,a2}; this->event::operator()(&a); }
};

// �����Œ�C�x���g �߂�l�Ȃ� �����R
template<typename ARG, typename A1, typename A2, typename A3>
class eventarg_va3 : public eventarg {
public: virtual void operator() (A1 a1, A2 a2, A3 a3) { ARG a={a1,a2,a3}; this->event::operator()(&a); }
};

// �����Œ�C�x���g �߂�l���� �����T
template<typename ARG, typename A1, typename A2, typename A3, typename A4, typename A5, typename R, R DEF>
class eventarg_ra5 : public eventarg {
public: virtual R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) { ARG a={DEF, a1,a2,a3,a4,a5}; this->event::operator()(&a); return a.ret; }
};

// �����Œ�C�x���g �߂�l���� �����U
template<typename ARG, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename R, R DEF>
class eventarg_ra6 : public eventarg {
public: virtual R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) { ARG a={DEF, a1,a2,a3,a4,a5,a6}; this->event::operator()(&a); return a.ret; }
};

// �N���X�֐��̌Ăяo��
// �e���v���[�g���s�v�̓��e�͂�����ɋL�q
class event_listener_for_register : public event_listener
{
public:
	class event_register* m_pEventRegister; // �o�^��C�x���g�폜�̃C�x���g

	// �R���X�g���N�^
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
		// �폜���ɂ͓o�^��C�x���g����폜����B
		if( m_pEvent != 0 ) {
			(*m_pEvent) -= this;
		}
	}

	// �o�^�����C�x���g���폜����鎞�ɌĂяo�����B
	virtual void event_delete(class event* pEvent);
	
	// �C�x���g�̓o�^�ӏ����g���[�X�o�͂���B
	virtual void dbg_trace_event()
	{
		#ifdef _DEBUG
			char buf[1024];sprintf(buf, "%s(%d) : �o�^�ӏ�\n", file, line); OutputDebugStringA(buf);
			m_pEvent->dbg_trace_event();
		#endif
	}

	// �o�^���ꂽ���P�[�V������ OutputDebugString ����B
	#ifdef _DEBUG
	virtual void reg_loc()
	{
		char buf[1024];sprintf(buf, "%s(%d) : �o�^�ӏ�\n", file, line); OutputDebugStringA(buf);
	}
	#endif
};

// �N���X�֐��̌Ăяo��
template<typename T, typename A>
class event_listener_for_class_a1 : public event_listener_for_register
{
	T* m_p; // �Ăяo����N���X
	void (T::*m_pFunc)(A p); // �Ăяo����֐�
public:
	event_listener_for_class_a1(T* p, void (T::*pFunc)(A a), const wchar_t* name, class event_register* pEventRegister=0) : event_listener_for_register(name)
	{
		m_p = p;
		m_pFunc = pFunc;
		m_pEventRegister = pEventRegister;
	}

	// �o�^�����C�x���g�����������Ƃ��ɌĂяo�����B
	virtual void call(void* p)
	{
		(m_p->*m_pFunc)((A)p);
	}
};

// �N���X�֐��̌Ăяo��
template<typename T>
class event_listener_for_class : public event_listener_for_register
{
	T* m_p; // �Ăяo����N���X
	void (T::*m_pFunc)(); // �Ăяo����֐�
public:
	event_listener_for_class(T* p, void (T::*pFunc)(), const wchar_t* name,  class event_register* pEventRegister=0)  : event_listener_for_register(name)
	{
		m_p = p;
		m_pFunc = pFunc;
		m_pEventRegister = pEventRegister;
	}

	// �o�^�����C�x���g�����������Ƃ��ɌĂяo�����B
	virtual void call(void* p)
	{
		(m_p->*m_pFunc)();
	}
};

// �N���X�֐��̌Ăяo��
template<typename T, typename A>
class event_listener_for_class_with_callclass : public event_listener_for_register
{
	T* m_p; // �Ăяo����N���X
	void (T::*m_pFunc)(A a, event_listener* p_caller); // �Ăяo����֐�
public:
	event_listener_for_class_with_callclass(T* p, void (T::*pFunc)(A a, event_listener* p_caller), const wchar_t* name,  class event_register* pEventRegister=0)  : event_listener_for_register(name)
	{
		m_p = p;
		m_pFunc = pFunc;
		m_pEventRegister = pEventRegister;
	}

	// �o�^�����C�x���g�����������Ƃ��ɌĂяo�����B
	virtual void call(void* p)
	{
		(m_p->*m_pFunc)((A)p, this);
	}
};

// �C�x���g�̌Ăяo��
class event_listener_fop_event : public event_listener_for_register
{
public:
	event* m_pEvent; // �Ăяo����C�x���g
	event_listener_fop_event(event* pEvent, const wchar_t* name,  class event_register* pEventRegister=0)  : event_listener_for_register(name)
	{
		m_pEvent = pEvent;
		m_pEventRegister = pEventRegister;
	}

	// �o�^�����C�x���g�����������Ƃ��ɌĂяo�����B
	virtual void call(void* p)
	{
		(*m_pEvent)(p);
	}
};

// �C�x���g�o�^�E�Ǘ��N���X
// �Ǘ����̃N���X�́A�{�N���X�̍폜���ɓo�^��������B
class event_register
{
	std::list<event_listener_for_register*> m_owner_item;
	std::list<event_listener_fop_event*> m_forevent_item;

public:
	// �o�^����
	void callon(event& e, event_listener_for_register* p, const char* file, int line)
	{
		#ifdef _DEBUG
			p->file = file;
			p->line = line;
		#endif

		e += p; // �C�x���g�o�^
		m_owner_item.push_back(p); // �Ǘ����̃A�C�e���Ƃ��Ēǉ�
	}

	// �C�x���g�o�^����
	template<typename T, typename A>
	void callon(event& e, T* pThis, void (T::*pFunc)(A a, event_listener* p_caller), const wchar_t* name, const char* file, int line)
	{
		event_listener_for_register* p = new event_listener_for_class_with_callclass<T, A>(pThis, pFunc, name, this); // �Ăяo���A�C�e���̐���
		callon(e, p, file, line);
	}

	// �C�x���g�o�^����
	template<typename T, typename A>
	void callon(event& e, T* pThis, void (T::*pFunc)(A a), const wchar_t* name, const char* file, int line)
	{
		event_listener_for_register* p = new event_listener_for_class_a1<T, A>(pThis, pFunc, name, this); // �Ăяo���A�C�e���̐���
		callon(e, p, file, line);
	}

	// �C�x���g�o�^����
	template<typename T>
	void callon(event& e, T* pThis, void (T::*pFunc)(), const wchar_t* name, const char* file, int line)
	{
		event_listener_for_register* p = new event_listener_for_class<T>(pThis, pFunc, name, this); // �Ăяo���A�C�e���̐���
		callon(e, p, file, line);
	}

	void callon(event& e, event& call, const wchar_t* name, const char* file, int line)
	{
		event_listener_fop_event* p = new event_listener_fop_event(&call, name, this); // �Ăяo���A�C�e���̐���
		callon(e, p, file, line);
	}

	void notcall(event& e, event& call)
	{
		bool bFind = false; // �Ăяo��
		// �Ăяo���폜
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

	// �C�x���g�폜�̃R�[���o�b�N
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

	// ���̃N���X�폜���̏���
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
		m_pEventRegister->event_delete(this); // �C�x���g�Ǘ����Ăяo��
	}
}

}

#endif // #ifndef event_hpp
