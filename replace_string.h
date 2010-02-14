#ifndef replace_string_h
#define replace_string_h

#include "event.h"

namespace ssc {

// �u�������񐶐�
class replace_string
{
public:
	std::wstring replace(const wchar_t* src);
	std::vector<std::wstring> match_str_list(const wchar_t* src);

	event _on_set;
	struct _on_set_arg{
		const wchar_t* find;
		const wchar_t* replace;
	};

public:
	// �u��������ݒ�
	void set(const wchar_t* find, const wchar_t* replace);
	// �u��������ݒ�
	void set(const wchar_t* find, const __int64 replace);
	// �u��������擾
	std::wstring get(const wchar_t* find);

	// �u��������擾
	int getsize();
	// �u��������擾
	std::wstring get(int n);

private:
	struct item {
		std::wstring find;
		std::wstring replace;
	};
	std::vector<item> m_vecItem;
};

} // namespace ssc {

#endif // #ifndef replace_printf_h
