#include "replace_print.h"
#include "ssc_string.h"
#include "replace_string.h"

namespace ssc {

event	_on_replace_print_set;
static replace_string s_replace_string;

class ___startup_first {
public:
	EVENT_USER_CLASS(___startup_first);
	___startup_first(){
		EVENT_CALL_EVENT(s_replace_string._on_set, _on_replace_print_set);
	}
};
static ___startup_first s;


// ï ê›íËï∂éöóÒíuä∑printf
std::wstring replace_print(const wchar_t* s)
{
	return s_replace_string.replace( s );
}

void replace_print_set(const wchar_t* find, const wchar_t* replace)
{
	s_replace_string.set(find, replace);

	_on_replace_print_set_arg a={find, replace};
	_on_replace_print_set(&a);
}
void replace_print_set(const wchar_t* find, const __int64 replace)
{
	std::wstring s = ssc::wstrprintf(L"%I64d", replace);
	s_replace_string.set(find, s.c_str() );

	_on_replace_print_set_arg a={find, s.c_str()};
	_on_replace_print_set(&a);
}

std::wstring	replace_print_get(const wchar_t* find)
{
	return s_replace_string.get(find);
}

std::vector<std::wstring>	replace_print_match_str_list(const wchar_t* s)
{
	return s_replace_string.match_str_list(s);
}

} //namespace ssc {
