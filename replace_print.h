#ifndef replace_print_h
#define replace_print_h

#include "event.h"
#include "replace_string.h"

namespace ssc {

// 別設定文字列置換printf
std::wstring	replace_print(const wchar_t* s);

// 置換文字列設定
void			replace_print_set(const wchar_t* find, const wchar_t* replace);
void			replace_print_set(const wchar_t* find, const __int64 replace);

// 現在の置換文字列取得
std::wstring	replace_print_get(const wchar_t* find);

// 文字列置換の実施
inline std::wstring	replace_printf(const wchar_t* fmt, ...){ return replace_print( ssc::va_wstrprintf(&fmt).c_str() ); };

// 文字列内の置換対象文字列リストの取得
std::vector<std::wstring>	replace_print_match_str_list(const wchar_t* s);

// 別設定文字列置換printf に関するイベント
extern event	_on_replace_print_set;
typedef replace_string::_on_set_arg _on_replace_print_set_arg;

} // namespace ssc {

#endif // #ifndef replace_printf_h
