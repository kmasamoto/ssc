#ifndef replace_string_h
#define replace_string_h

#include "event.h"

namespace ssc {

// 置換文字列生成
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
	// 置換文字列設定
	void set(const wchar_t* find, const wchar_t* replace);
	// 置換文字列設定
	void set(const wchar_t* find, const __int64 replace);
	// 置換文字列取得
	std::wstring get(const wchar_t* find);

	// 置換文字列取得
	int getsize();
	// 置換文字列取得
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
