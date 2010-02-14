// CRequestThread.h: CRequestThread クラスのインターフェイス
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
// スレッド使用者への外部インターフェイス
public:
	// リクエストリストタイプの宣言
	class RequestList : public std::list<T>{};

	// 実行外部デリゲート
	Delegate _OnExecute; // T* がわたります。

	// スレッドを作成し、実行させる
	virtual void Create() {
		m_pWinThread = AfxBeginThread(CRequestThread<T>::StaticThread, this, THREAD_PRIORITY_IDLE, 0,0,0);
		m_pWinThread->m_bAutoDelete = TRUE;
		m_bClose = false;
	}

	// リクエストを追加する。
	// 最大リクエスト数を超えたら、失敗する。
	virtual bool Request(T val) {
		m_ThreadCriticalSection.Lock();
		// 最大リクエスト数は超えてないかな？
		if(m_nRequestCountLimit != 0 && m_nRequestCountLimit <= m_ThreadRequest.size() ) {
			m_ThreadCriticalSection.Unlock();
			return false;
		}
		m_AllRequestEndEvent.ResetEvent();
		m_ThreadRequest.push_back(val);
		m_ThreadCriticalSection.Unlock();

		return true;
	}

	// 実行させる。
	virtual void Kick() {
		assert(m_pWinThread != 0);
		m_Event.SetEvent();
	}

	// スレッド終了
	virtual void Close() {
		// 閉じる
		m_bClose = true;
		// スレッドを実行させる
		Kick();
		// スレッド終了まで待ち合わせ
		::WaitForSingleObject( m_pWinThread->m_hThread, INFINITE);
	}

	// スレッド強制終了
	virtual void CloseForce() {
		// スレッドを強制終了させる
		if(m_pWinThread->SuspendThread() != 0xFFFFFFFF){
			delete m_pWinThread;
			m_pWinThread = 0;
		}
	}

	// 最大リクエスト数制限をかける
	virtual void SetRequestCountLimit(int nLimit)
	{
		m_nRequestCountLimit = nLimit;
	}

	// 全てのリクエスト処理が終了するまで待つ
	virtual void WaitForAllRequest()
	{
		// スレッドを実行させる
		Kick();
		WaitForSingleObject(m_AllRequestEndEvent, INFINITE);
	}

	// スレッド終了の確認
	virtual bool IsExitThread(RequestList& requestList) {
		return true;
	}

// スレッド継承使用者へのインターフェイス
protected:
	bool IsCloseing(){ return m_bClose; } // 継承先での確認。終了処理に入ったもしくは終了中であるかを返す。

	// 継承先インターフェイス
	void WaitForEvent();

	// 各１つだけ渡されて処理する。
	// １つづつの処理を記述するのみで、
	// 後の処理はデフォルトでよい場合はこちら。
	virtual void Execute(T& request) {
		_OnExecute(&request);
	}

	// 全てのリストを渡されて処理する。
	// リスト全体に対して、デフォルト動作以外を行い場合にオーバーライド
	virtual void Execute(RequestList& requestList) {
		RequestList::iterator it;

		for(it = requestList.begin(); it != requestList.end(); it++) {
			if( m_bClose ) {
				return;
			}
			Execute(*it);
		}

		// リクエストをクリアする。
		requestList.clear();
		return;
	}

// ワーカースレッド本体
private:
	static UINT StaticThread(LPVOID lpParameter) {
		CRequestThread<T>* t = static_cast<CRequestThread<T>*>(lpParameter);
		return t->Thread();
	}

// 公開インターフェイス
public:
	CRequestThread() :
		m_AllRequestEndEvent(FALSE, TRUE)
	{
		m_pWinThread = 0;
		m_bClose = false;
		m_nRequestCountLimit = 0; // リクエスト数制限無し
		m_AllRequestEndEvent.SetEvent();
	}
	virtual ~CRequestThread() {
		// 必ず終了関数を呼び出すようアサートを出す。
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
	// スレッド呼び出し
	virtual UINT Thread() {
		localRequest.clear();
		while(1) {
			WaitForSingleObject(m_Event, INFINITE);
			if( m_bClose ) {
				break;
			}

			// スレッド外から設定されたリクエストを確認
			m_ThreadCriticalSection.Lock();
			localRequest.swap(m_ThreadRequest);
			m_ThreadCriticalSection.Unlock();

			// 終了設定がきて、リクエストがまだ残ってても終わっていい？
			if( m_bClose ) {
				if(IsExitThread(localRequest)) {
					break;
				}
			}

			Execute(localRequest);

			// 終了が来たらスレッドを終わる。
			if( m_bClose ) {
				break;
			}
			// リクエストがなくなっていれば終了待ち合わせ終わり
			m_ThreadCriticalSection.Lock();
			// リクエストが空？
			if(m_ThreadRequest.empty()) {
				m_AllRequestEndEvent.SetEvent();
			}
			m_ThreadCriticalSection.Unlock();
		}
		return 0;
	};

	
protected:
	RequestList m_ThreadRequest;	// リクエストリスト
	RequestList localRequest;
	CCriticalSection m_ThreadCriticalSection;	// リクエストリストへのアクセスを制限
	CEvent m_Event;					// スレッド実行イベント
	CWinThread* m_pWinThread;		// スレッドハンドル保有クラス
	bool m_bClose;					// スレッド終了フラグ
	int m_nRequestCountLimit;		// 最大リクエスト数
	CEvent	m_AllRequestEndEvent;	// リクエスト終了待ち合わせ
};

#endif // !defined(AFX_CREQUESTTHREAD_H__241872B0_22BC_4745_A39D_75275CEC163D__INCLUDED_)
