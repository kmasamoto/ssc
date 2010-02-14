#ifndef config_hpp
#define config_hpp

#include "delegate.hpp"
#include "DB_SQLite.hpp"
#include <map>

class Config
{
private:
	// �A�v���P�[�V�����Ŏg�p����f�[�^�x�[�X�t�@�C�������w�肷��B
	static const wchar_t* s_SqliteDBFileName;
	// �V���O���g��
	static Config s_Instance;
	// SQLite �f�[�^�x�[�X
	db_sqlite m_db;
	
private:	// �V���O���g��
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

public: // �C�x���g�o�^
	std::map<std::wstring, Delegate> OnChangeConfig;
};

#endif // #ifndef config_hpp
