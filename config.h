#ifndef config_hpp
#define config_hpp

#include "delegate.hpp"
#include "DB_SQLite.hpp"
#include <map>

class Config
{
private:
	// アプリケーションで使用するデータベースファイル名を指定する。
	static const wchar_t* s_SqliteDBFileName;
	// シングルトン
	static Config s_Instance;
	// SQLite データベース
	db_sqlite m_db;
	
private:	// シングルトン
	Config()
	{
		m_db.open(s_SqliteDBFileName);
	}
public:
	~Config()
	{
		m_db.close();
	}
	Config& GetInstance()
	{
		return s_Instance;
	}

public: // イベント登録
	std::map<std::wstring, Delegate> OnChangeConfig;
};

#endif // #ifndef config_hpp
