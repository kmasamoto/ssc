#ifndef dbg_timer_hpp
#define dbg_timer_hpp

// 実行時間計測
// 2005/12/09 K.Masamoto(Media+)
#include <mmsystem.h>
#include <string>
#include <sstream>
#include <map>
#pragma comment(lib,"winmm.lib")

#ifdef _DEBUG
	#define DBG_TIMER_TRACE 1
#else
	#define DBG_TIMER_TRACE 0
#endif

namespace ssc{

#define DLAP(s) 

//eventf g_on_lap;

//#define LAP1(s) g_on_lap(L"%s(%d):" L#s, __FILE__, __LINE__)

#define dbg_timer(val) ssc::dbg_timer val

#define LAP(val, s) val.lap(s, -1, __FILE__, __LINE__)
#define LAPF(val) val.lap_timediff("", __FILE__, __LINE__)
#define LAPS(val, s) val.lap_stack(s, __FILE__, __LINE__)
#define LAPSR(val, s) val.lap_stack_report(s, __FILE__, __LINE__)
#define LAPSRA(val) val.lap_stack_report_all(__FILE__, __LINE__)

#define LAP_FANCTIME(val, f) val.lap_timediff(#f " befour", __FILE__, __LINE__); f; val.lap_timediff(#f " after", __FILE__, __LINE__)
#define LAPFX(val, f) val.lap_stack(#f, __FILE__, __LINE__)

class dbg_timer {
public:
	std::string s;
	DWORD begin, last;

	struct LapStacData {
		DWORD dwTime;
		std::string file;
		int line;
	};
	std::map<std::string, LapStacData> m_mapLapStack;

	void start( const char* fmt, ... )
	{
		char buf[4096];

		va_list arg;

		va_start(arg, fmt);

		vsprintf(buf, fmt, arg);

		va_end(arg);

		begin = ::timeGetTime();
		last = begin;
		s = buf;
		s += "\n";
	}

	void lap(std::string ss="", long lap=-1, const char* pFile="", int nLine=0)
	{
		DWORD now = ::timeGetTime();
		char buf[512];
		if(lap==-1) {
			sprintf(buf, "%s(%d): %.3fsec[%.3f] : %s\n", pFile, nLine, (double)(now - begin)/1000, (double)(now - last) / 1000, ss.c_str());
		}
		else {
			sprintf(buf, "%s(%d): %.3fsec[%.3f] : %s\n", pFile, nLine, (double)(now - begin)/1000, (double)lap / 1000, ss.c_str());
		}
		s+=buf;
		last = now;
	}

	void lap_timediff(std::string ss="", const char* pFile="", int nLine=0)
	{
		DWORD now = ::timeGetTime();
		if(last != now) {
			if(now-last > 300) {
				int n=0;
			}
			lap(ss, -1, pFile, nLine);
		}
	}

	// 複数回呼び出される関数の処理時間を計測する。
	void lap_stack(std::string ss="", const char* pFile="", int nLine=0)
	{
		DWORD now = ::timeGetTime();

		std::map<std::string, LapStacData>::iterator it = m_mapLapStack.find(ss);

		if(it == m_mapLapStack.end()) {
			LapStacData dt;
			dt.dwTime = now - last;
			dt.file = pFile;
			dt.line = nLine;

			m_mapLapStack[ss] = dt;
		}
		else {
			it->second.dwTime += now - last;
		}
		last = now;
	}

	void lap_stack_report(std::string ss="", const char* pFile="", int nLine=0)
	{
		char buf[512];
		sprintf(buf, "%s", ss.c_str());
		lap(buf, m_mapLapStack[ss].dwTime, m_mapLapStack[ss].file.c_str(), m_mapLapStack[ss].line);
		m_mapLapStack[ss].dwTime=0;
	}

	void lap_stack_report_all(const char* pFile="", int nLine=0)
	{
		std::map<std::string, LapStacData>::iterator it = m_mapLapStack.begin();
		for(; it!=m_mapLapStack.end(); it++) {
			if(it->second.dwTime != 0) {
				lap_stack_report(it->first, pFile, nLine);
			}
		}
	}

	void report(){
		#if DBG_TIMER_TRACE
			char buf[1024];
			sprintf(buf, "%s(%d):", __FILE__, __LINE__);

			std::stringstream ss(s);
			std::string bufstr;
			while(std::getline(ss,bufstr) ){
				OutputDebugStringA(bufstr.c_str());
				OutputDebugStringA("\n");
			}
		#endif
	}


	void report_at_timediff()
	{
		if(begin != timeGetTime()) {
			report();
		}
	}
};

} // namespace ssc{

#endif // #ifndef dbg_timer_hpp
