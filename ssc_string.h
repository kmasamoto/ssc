#ifndef string_h
#define string_h

#include "macros.h"
#include <stdio.h>

namespace ssc {

int va_snwprintf(wchar_t* buf, size_t size, const wchar_t* (*fmt) ); // �ψ������� printf ���������{
int va_sntprintf(TCHAR* buf, size_t size, const TCHAR* (*fmt) ); // �ψ������� printf ���������{
std::wstring va_wstrprintf(const wchar_t* (*fmt)); // �o�b�t�@1024�� wstrprintf 
std::wstring wstrprintf(const wchar_t* fmt, ...); // �o�b�t�@1024�� wstrprintf 
std::wstring nwstrprintf(size_t size, const wchar_t* fmt, ...); // �o�b�t�@���w��ł� wstrprintf 

// ������Ԃ�printf
std::string strprintf(const char* fmt ... );

// �o�b�t�@�w��� wstrprintf 
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
