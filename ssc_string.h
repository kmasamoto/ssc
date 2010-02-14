#ifndef string_h
#define string_h

#include "macros.h"
#include <stdio.h>

namespace ssc {

int va_snwprintf(wchar_t* buf, size_t size, const wchar_t* (*fmt) ); // 可変引数から printf 処理を実施
int va_sntprintf(TCHAR* buf, size_t size, const TCHAR* (*fmt) ); // 可変引数から printf 処理を実施
std::wstring va_wstrprintf(const wchar_t* (*fmt)); // バッファ1024の wstrprintf 
std::wstring wstrprintf(const wchar_t* fmt, ...); // バッファ1024の wstrprintf 
std::wstring nwstrprintf(size_t size, const wchar_t* fmt, ...); // バッファ数指定での wstrprintf 

// 文字列返すprintf
std::string strprintf(const char* fmt ... );

// バッファ指定の wstrprintf 
template<int bufsize>
std::wstring va_nwstrprintf(const wchar_t* (*fmt))
{
	wchar_t buf[bufsize];
	va_snwprintf(&buf[0], arrayof(buf), fmt);

	return std::wstring(buf);
}

std::wstring strreplace(const wchar_t* src, const wchar_t* find, const wchar_t* replace);

std::string wstrtostr(const std::wstring& s);

} // namespace ssc {

#endif // #ifndef string_hpp
