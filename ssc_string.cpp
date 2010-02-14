#include "ssc_string.h"
#include "replace_print.h"

namespace ssc {

// 可変引数から printf 処理を実施
int va_snwprintf(wchar_t* buf, size_t size, const wchar_t* (*fmt) )
{
	va_list arg;

	va_start(arg, *fmt);

	int n = _vsnwprintf(buf, size, *fmt, arg);
	assert(n!=-1);
	assert(n < size);

	va_end(arg);

	return n;
}

// 可変引数から printf 処理を実施
int va_sntprintf(TCHAR* buf, size_t size, const TCHAR* (*fmt) )
{
	va_list arg;

	va_start(arg, *fmt);

	int n = _vsntprintf(buf, size, *fmt, arg);
	assert(n!=-1);
	assert(n < size);

	va_end(arg);

	return n;
}


// バッファ1024の wstrprintf 
std::wstring va_wstrprintf(const wchar_t* (*fmt))
{
	return va_nwstrprintf<1024>(fmt);
}

// バッファ4096の wstrprintf 
std::wstring wstrprintf(const wchar_t* fmt, ...)
{
	wchar_t buf[4096];
	int n = va_snwprintf(&buf[0], arrayof(buf), &fmt);
	assert(n!=-1);
	assert(n < 4096);

	return buf;
}

// バッファ数指定での wstrprintf 
std::wstring nwstrprintf(size_t size, const wchar_t* fmt, ...)
{
	std::vector<wchar_t> buf;
	buf.resize(size);
	va_snwprintf(&buf[0], size, &fmt);

	return std::wstring(&buf[0]);
}

std::wstring strreplace(const wchar_t* psrc, const wchar_t* find, const wchar_t* replace)
{
	// 置き換えが発生するので作業用バッファに置く。
	std::wstring src = psrc;

	int n=0;
	// 見つかれば処理
	while( (n = src.find(find, n)) != std::wstring::npos) {
		src.erase(n, wcslen(find));
		src.insert(n, replace);
	}

	return src;
}

//ワイド文字列からマルチバイト文字列
//ロケール依存
void narrow(const std::wstring &src, std::string &dest) {
	char *mbs = new char[src.length() * MB_CUR_MAX + 1];
	wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
	dest = mbs;
	delete [] mbs;
}

//マルチバイト文字列からワイド文字列
//ロケール依存
void widen(const std::string &src, std::wstring &dest) {
	wchar_t *wcs = new wchar_t[src.length() + 1];
	mbstowcs(wcs, src.c_str(), src.length() + 1);
	dest = wcs;
	delete [] wcs;
}

std::string wstrtostr(const std::wstring& s)
{
	char *mbs = new char[s.length() * MB_CUR_MAX + 1];
	wcstombs(mbs, s.c_str(), s.length() * MB_CUR_MAX + 1);
	std::string dest = mbs;
	delete [] mbs;

	return dest;
}

std::wstring strtowstr(const std::string& s)
{
	wchar_t *wcs = new wchar_t[s.length() + 1];
	mbstowcs(wcs, s.c_str(), s.length() + 1);
	std::wstring dest = wcs;
	delete [] wcs;

	return dest;
}

template <class T>
std::wstring to_wstring(T x)
{
	std::wstringstream wss;
	wss << x;
	return wss.str();
}
template <class T>
std::wstring to_string(T x)
{
	std::stringstream ss;
	ss << x;
	return ss.str();
}

// 可変引数から printf 処理を実施
int va_snprintf(char* buf, size_t size, const char* (*fmt) )
{
	va_list arg;

	va_start(arg, *fmt);

	int n = _vsnprintf(buf, size, *fmt, arg);
	assert(n!=-1);
	assert(n < size);

	va_end(arg);

	return n;
}

std::string strprintf(const char* fmt ... )
{
	char buf[4096];
	int n = va_snprintf(&buf[0], arrayof(buf), &fmt);
	return buf;
}

} // namespace ssc