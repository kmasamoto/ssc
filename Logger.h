#ifndef _Logger_hpp
#define _Logger_hpp

#define _CriticalSection_h // これもここで定義してしまいます。

#include <string>

// 1つボタン押してすぐもう1つのボタンを押すと動作異常を起こすため、
// ハンドラの多重呼び出しを防ぐ。
// クリティカルセクション
class CriticalSection {
	CRITICAL_SECTION cs;
public:
	CriticalSection() { InitializeCriticalSection(&cs);	}
	~CriticalSection() {	DeleteCriticalSection( &cs );	}
	void EnterCriticalSection() { ::EnterCriticalSection( &cs ); }
	void LeaveCriticalSection() { ::LeaveCriticalSection( &cs ); }
};

class Logger
{
public:
	Logger(const char* filename, size_t reserveBytes=1*1024*1024, bool bLogEnable=true);
	void Log(const char* fmt...);
	void LogTime();
	void Output();

public:
	std::string m_strFilename;
	std::string m_strLog;
	bool		m_bLogEnable;
	CriticalSection m_criticalSection;
	int			m_nLogTimeCnt;
};

inline Logger::Logger(const char* filename, size_t reserveBytes, bool bLogEnable)
{
	m_strFilename = filename;
	m_strLog.reserve(reserveBytes);
	m_bLogEnable = bLogEnable;
	m_nLogTimeCnt=0;
}
inline void Logger::LogTime()
{
	if(!m_bLogEnable) return;
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	Log("%02d/%02d/%02d %02d:%02d:%02d.%03d : ",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, 
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond, 
			sysTime.wMilliseconds
			);
	#ifdef _XRAYDISP
		Log("00:XRayDisp : %03d :", m_nLogTimeCnt++);
	#else
		Log("01:02XRay   : %03d :", m_nLogTimeCnt++);
	#endif
}
inline void Logger::Log(const char* fmt...)
{
	if(!m_bLogEnable) return;
	va_list arg;

	va_start(arg, fmt);

	char buf[1024];
	vsprintf(buf, fmt, arg);

	va_end(arg);

	m_criticalSection.EnterCriticalSection();
	m_strLog += buf;
	m_criticalSection.LeaveCriticalSection();
}
inline void Logger::Output()
{
	if(!m_bLogEnable) return;
	m_criticalSection.EnterCriticalSection();
	FILE* fp=fopen(m_strFilename.c_str(), "a");
	if(fp != 0) {
		fprintf(fp, m_strLog.c_str());
		fclose(fp);

		m_strLog="";
	}
	m_criticalSection.LeaveCriticalSection();
}

#endif // #ifndef _Logger_hpp
