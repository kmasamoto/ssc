#include "replace_string.h"

namespace ssc {

std::wstring replace_string::replace(const wchar_t* psrc)
{
	std::wstring src = psrc;

	for(int i=0; i<m_vecItem.size(); i++) {
		if(src.find( m_vecItem[i].find.c_str()) != std::wstring::npos ) {
			src = strreplace(src.c_str(), m_vecItem[i].find.c_str(), m_vecItem[i].replace.c_str());
		}
	}

	return src;
}

// 置換文字列設定
void replace_string::set(const wchar_t* find, const __int64 replace)
{
	wchar_t buf[512];
	set(find, _itow(replace,buf,10));
}

// 置換文字列設定
void replace_string::set(const wchar_t* find, const wchar_t* replace)
{
	item one;
	one.find = find;
	one.replace = replace;

	bool bFind=false;
	for(int i=0; i<m_vecItem.size(); i++) {
		if( wcscmp(m_vecItem[i].find.c_str(), find) == 0) {
			bFind = true;
			
			// 文字列変更?
			if( wcscmp(m_vecItem[i].replace.c_str(), replace) != 0) {
				_on_set_arg arg = {find, replace};
				_on_set(&arg);
				m_vecItem[i] = one;
			}
		}
	}

	if(!bFind) {
		m_vecItem.push_back(one);

		// 文字列変更
		_on_set_arg arg = {find, replace};
		_on_set(&arg);
	}
}

std::wstring replace_string::get(const wchar_t* find)
{
	for(int i=0; i<m_vecItem.size(); i++) {
		if( wcscmp(m_vecItem[i].find.c_str(), find) == 0) {
			return m_vecItem[i].replace;
		}
	}

	return L"";
}

std::vector<std::wstring> replace_string::match_str_list(const wchar_t* src)
{
	std::vector<std::wstring> ret;

	for(int i=0; i<m_vecItem.size(); i++ ) {
		if( wcsstr(src, m_vecItem[i].find.c_str()) != 0 ) {
			ret.push_back(m_vecItem[i].find);
		}
	}

	return ret;
}

} // namespace ssc