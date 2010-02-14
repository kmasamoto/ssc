// CRequestThread.h: CRequestThread �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREQUESTTHREAD_H__241872B0_22BC_4745_A39D_75275CEC163D__INCLUDED_)
#define AFX_CREQUESTTHREAD_H__241872B0_22BC_4745_A39D_75275CEC163D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>
#include <afxmt.h>
#include <assert.h>
#include "delegate.hpp"

template <class T>
class CRequestThread {
// �X���b�h�g�p�҂ւ̊O���C���^�[�t�F�C�X
public:
	// ���N�G�X�g���X�g�^�C�v�̐錾
	class RequestList : public std::list<T>{};

	// ���s�O���f���Q�[�g
	Delegate _OnExecute; // T* ���킽��܂��B

	// �X���b�h���쐬���A���s������
	virtual void Create() {
		m_pWinThread = AfxBeginThread(CRequestThread<T>::StaticThread, this, THREAD_PRIORITY_IDLE, 0,0,0);
		m_pWinThread->m_bAutoDelete = TRUE;
		m_bClose = false;
	}

	// ���N�G�X�g��ǉ�����B
	// �ő僊�N�G�X�g���𒴂�����A���s����B
	virtual bool Request(T val) {
		m_ThreadCriticalSection.Lock();
		// �ő僊�N�G�X�g���͒����ĂȂ����ȁH
		if(m_nRequestCountLimit != 0 && m_nRequestCountLimit <= m_ThreadRequest.size() ) {
			m_ThreadCriticalSection.Unlock();
			return false;
		}
		m_AllRequestEndEvent.ResetEvent();
		m_ThreadRequest.push_back(val);
		m_ThreadCriticalSection.Unlock();

		return true;
	}

	// ���s������B
	virtual void Kick() {
		assert(m_pWinThread != 0);
		m_Event.SetEvent();
	}

	// �X���b�h�I��
	virtual void Close() {
		// ����
		m_bClose = true;
		// �X���b�h�����s������
		Kick();
		// �X���b�h�I���܂ő҂����킹
		::WaitForSingleObject( m_pWinThread->m_hThread, INFINITE);
	}

	// �X���b�h�����I��
	virtual void CloseForce() {
		// �X���b�h�������I��������
		if(m_pWinThread->SuspendThread() != 0xFFFFFFFF){
			delete m_pWinThread;
			m_pWinThread = 0;
		}
	}

	// �ő僊�N�G�X�g��������������
	virtual void SetRequestCountLimit(int nLimit)
	{
		m_nRequestCountLimit = nLimit;
	}

	// �S�Ẵ��N�G�X�g�������I������܂ő҂�
	virtual void WaitForAllRequest()
	{
		// �X���b�h�����s������
		Kick();
		WaitForSingleObject(m_AllRequestEndEvent, INFINITE);
	}

	// �X���b�h�I���̊m�F
	virtual bool IsExitThread(RequestList& requestList) {
		return true;
	}

// �X���b�h�p���g�p�҂ւ̃C���^�[�t�F�C�X
protected:
	bool IsCloseing(){ return m_bClose; } // �p����ł̊m�F�B�I�������ɓ������������͏I�����ł��邩��Ԃ��B

	// �p����C���^�[�t�F�C�X
	void WaitForEvent();

	// �e�P�����n����ď�������B
	// �P�Â̏������L�q����݂̂ŁA
	// ��̏����̓f�t�H���g�ł悢�ꍇ�͂�����B
	virtual void Execute(T& request) {
		_OnExecute(&request);
	}

	// �S�Ẵ��X�g��n����ď�������B
	// ���X�g�S�̂ɑ΂��āA�f�t�H���g����ȊO���s���ꍇ�ɃI�[�o�[���C�h
	virtual void Execute(RequestList& requestList) {
		RequestList::iterator it;

		for(it = requestList.begin(); it != requestList.end(); it++) {
			if( m_bClose ) {
				return;
			}
			Execute(*it);
		}

		// ���N�G�X�g���N���A����B
		requestList.clear();
		return;
	}

// ���[�J�[�X���b�h�{��
private:
	static UINT StaticThread(LPVOID lpParameter) {
		CRequestThread<T>* t = static_cast<CRequestThread<T>*>(lpParameter);
		return t->Thread();
	}

// ���J�C���^�[�t�F�C�X
public:
	CRequestThread() :
		m_AllRequestEndEvent(FALSE, TRUE)
	{
		m_pWinThread = 0;
		m_bClose = false;
		m_nRequestCountLimit = 0; // ���N�G�X�g����������
		m_AllRequestEndEvent.SetEvent();
	}
	virtual ~CRequestThread() {
		// �K���I���֐����Ăяo���悤�A�T�[�g���o���B
		ASSERT(m_bClose);
	}

	void EnterCriticalSection()
	{
		m_ThreadCriticalSection.Lock();
	}
	void LeveCriticalSection()
	{
		m_ThreadCriticalSection.Unlock();
	}

protected:
	// �X���b�h�Ăяo��
	virtual UINT Thread() {
		localRequest.clear();
		while(1) {
			WaitForSingleObject(m_Event, INFINITE);
			if( m_bClose ) {
				break;
			}

			// �X���b�h�O����ݒ肳�ꂽ���N�G�X�g���m�F
			m_ThreadCriticalSection.Lock();
			localRequest.swap(m_ThreadRequest);
			m_ThreadCriticalSection.Unlock();

			// �I���ݒ肪���āA���N�G�X�g���܂��c���ĂĂ��I����Ă����H
			if( m_bClose ) {
				if(IsExitThread(localRequest)) {
					break;
				}
			}

			Execute(localRequest);

			// �I����������X���b�h���I���B
			if( m_bClose ) {
				break;
			}
			// ���N�G�X�g���Ȃ��Ȃ��Ă���ΏI���҂����킹�I���
			m_ThreadCriticalSection.Lock();
			// ���N�G�X�g����H
			if(m_ThreadRequest.empty()) {
				m_AllRequestEndEvent.SetEvent();
			}
			m_ThreadCriticalSection.Unlock();
		}
		return 0;
	};

	
protected:
	RequestList m_ThreadRequest;	// ���N�G�X�g���X�g
	RequestList localRequest;
	CCriticalSection m_ThreadCriticalSection;	// ���N�G�X�g���X�g�ւ̃A�N�Z�X�𐧌�
	CEvent m_Event;					// �X���b�h���s�C�x���g
	CWinThread* m_pWinThread;		// �X���b�h�n���h���ۗL�N���X
	bool m_bClose;					// �X���b�h�I���t���O
	int m_nRequestCountLimit;		// �ő僊�N�G�X�g��
	CEvent	m_AllRequestEndEvent;	// ���N�G�X�g�I���҂����킹
};

#endif // !defined(AFX_CREQUESTTHREAD_H__241872B0_22BC_4745_A39D_75275CEC163D__INCLUDED_)
