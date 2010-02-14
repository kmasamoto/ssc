#ifndef replace_print_h
#define replace_print_h

#include "event.h"
#include "replace_string.h"

namespace ssc {

// �ʐݒ蕶����u��printf
std::wstring	replace_print(const wchar_t* s);

// �u��������ݒ�
void			replace_print_set(const wchar_t* find, const wchar_t* replace);
void			replace_print_set(const wchar_t* find, const __int64 replace);

// ���݂̒u��������擾
std::wstring	replace_print_get(const wchar_t* find);

// ������u���̎��{
inline std::wstring	replace_printf(const wchar_t* fmt, ...){ return replace_print( ssc::va_wstrprintf(&fmt).c_str() ); };

// ��������̒u���Ώە����񃊃X�g�̎擾
std::vector<std::wstring>	replace_print_match_str_list(const wchar_t* s);

// �ʐݒ蕶����u��printf �Ɋւ���C�x���g
extern event	_on_replace_print_set;
typedef replace_string::_on_set_arg _on_replace_print_set_arg;

} // namespace ssc {

#endif // #ifndef replace_printf_h
