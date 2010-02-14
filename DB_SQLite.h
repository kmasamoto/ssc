#ifndef DB_SQLite_hpp
#define DB_SQLite_hpp

#pragma warning(disable:4786) // STL �̌x���폜

#include "sqlite3.h"
#include <map>
#include <string>
#include <memory> // auto_ptr
#include <assert.h>
#include "ssc_string.h"
#include "event.h"

namespace ssc {

// �f�[�^�Z�b�g
class db_sqlite_dataset
{
public: // ��{����
	db_sqlite_dataset()		{		m_stm = 0;	}
	~db_sqlite_dataset()	{		if(m_stm != 0) sqlite3_finalize(m_stm);	}

	// �f�[�^�擾
	int column_count() { return sqlite3_column_count(m_stm); }
	const wchar_t* column_name(int n){ return (const wchar_t*)sqlite3_column_name16(m_stm, n); }
	bool step()	{ return sqlite3_step(m_stm) == SQLITE_ROW; }
	bool reset()	{ return sqlite3_reset(m_stm) != 0; }

	const wchar_t* column_decltype(int col){ return (const wchar_t*)sqlite3_column_decltype16(m_stm, col); }

	// �J�����̃C���f�b�N�X�͂O�x�[�X�B
	int column_int(int col)				{ return sqlite3_column_int(m_stm, col); }
	__int64 column_int64(int col) { return sqlite3_column_int64(m_stm, col); }

	// �J�����̃C���f�b�N�X�͂O�x�[�X�B
	std::string column_utf8(int col)	{ return std::string ((char const*) ::sqlite3_column_text (m_stm, col), ::sqlite3_column_bytes (m_stm, col)); }
	std::wstring column_wstr(int col)	{ return std::wstring((wchar_t const*) ::sqlite3_column_text16(m_stm, col)); }
	const wchar_t* column_wchar(int col)	{ return (const wchar_t *) ::sqlite3_column_text16(m_stm, col); }

public: // �g������

	// �J�����̃C���f�b�N�X�͂O�x�[�X�B
	// �J������������ɂ��擾
	int column_int(const wchar_t* col)				{ map(); return column_int(m_mapColnameIndex[col]); }
	int column_int64(const wchar_t* col)				{ map(); return column_int64(m_mapColnameIndex[col]); }
	std::string column_utf8(const wchar_t* col)		{ map(); return column_utf8(m_mapColnameIndex[col]); }
	std::wstring column_wstr(const wchar_t* col)		{ map(); return column_wstr(m_mapColnameIndex[col]); }
	const wchar_t* column_wchar(const wchar_t* col)	{ map(); return column_wchar(m_mapColnameIndex[col]); }

	// �񖼃}�b�v
	void map()
	{
		// �܂��}�b�v����Ă��Ȃ��ꍇ�������s
		if(!m_mapColnameIndex.empty()) return;

		int colCount = sqlite3_column_count(m_stm);
		for (int col = 0; col < colCount; ++col) {
			m_mapColnameIndex[(const wchar_t*)sqlite3_column_name16(m_stm, col)] = col;
		}
	}

private:
	// �t�����h�N���X
	friend class db_sqlite;

	// �f�[�^�A�N�Z�X���
	sqlite3_stmt* m_stm;
	int prepare(sqlite3* db, const wchar_t* sql) { return sqlite3_prepare16(db, sql, -1, &m_stm, NULL); }

	// �񖼂ƃJ�����̑Ή��}�b�v�B
	std::map<std::wstring, int> m_mapColnameIndex;

};

// �ʏ툵���p�̃f�[�^�Z�b�g
class db_sqlite_dataset_autoptr : public std::auto_ptr<db_sqlite_dataset>
{
// null �`�F�b�N
public:
	db_sqlite_dataset_autoptr(db_sqlite_dataset* p) :
		std::auto_ptr<db_sqlite_dataset>(p)
	{
	}

	bool isnull() { return this->get() == 0; };
};

// �f�[�^�x�[�X�N���X
class db_sqlite  {
public: // �R���X�g���N�^
	db_sqlite()	{ m_db = 0;	m_autobegin = true; }
	~db_sqlite() {	sqlite3_close(m_db); }

public: // �C�x���g
	ssc::eventf _on_import_error; // �C���|�[�g�����ŃG���[������
	ssc::eventf _on_error;		// �G���[����

	// Update �Ăяo���������̃C�x���g�f�[�^
	struct _on_update_arg {
		const wchar_t* table;
		const wchar_t* key;
		std::vector<__int64>* keyvalvec;
		const wchar_t* sql;
	};
	ssc::event _on_pre_update;		// Update �Ăяo�������O
	ssc::event _on_post_update;		// Update �Ăяo��������

	// Delete �Ăяo���������̃C�x���g�f�[�^
	struct _on_delete_arg {
		const wchar_t* table;
		const wchar_t* key;
		std::vector<__int64>* keyvalvec;
		const wchar_t* sql;
	};
	ssc::event _on_pre_delete;		// Delete �Ăяo�������O
	ssc::event _on_post_delete;		// Delete �Ăяo��������

	// insert �Ăяo���������̃C�x���g�f�[�^
	struct _on_insert_arg {
		const wchar_t* table;
		const wchar_t* sql;
	};
	ssc::event _on_pre_insert;		// insert �Ăяo�������O
	ssc::event _on_post_insert;		// insert �Ăяo��������

	ssc::eventf _on_exec;	// exec �Ăяo������

	ssc::event _on_begin;	// select �Ăяo������
	ssc::event _on_commit;	// select �Ăяo������
	ssc::event _on_rollback;// select �Ăяo������

public: // ��{����
	sqlite3* sqlite3(){ return m_db; }

	// �I�[�v��
	int open(const wchar_t* filename){ return sqlite3_open16(filename, &m_db);	}
	int close(){ if(m_db != 0) return sqlite3_close(m_db); return 0;}

	// �Ō�ɑ}�������sID���擾
	int last_insert_rowid(){ return sqlite3_last_insert_rowid(m_db); }

	// �g�����U�N�V�����ɓ����Ă��邩�m�F
	bool is_transaction(){ return sqlite3_get_autocommit(m_db) == 0; }

	// ���s��{�`
	// SQL�����s���܂��Bselect ���A�߂�l�Ńf�[�^�Z�b�g������ꍇ�͂�����
	db_sqlite_dataset_autoptr select(const wchar_t* sql);
	// SQL�����s���܂��Bupdate, delete, insert ���A�߂�l�Ńf�[�^�Z�b�g���Ȃ��ꍇ�͂�����
	bool exec(const wchar_t* sql);
	// select���Ŏ擾�ł���ŏ��̃J������int64�Ŏ擾(���s����-1��Ԃ��܂��B�}�C�i�X��ID�͂Ȃ����̂Ƃ��Ă̓���ł��BSQL�Ŏ擾�ł���1�s�ځA1��ڂ̃f�[�^��int�Ŏ擾���ĕԂ��܂��B)
	__int64 front_int64(const wchar_t* sql);
	// select���Ŏ擾�ł���ŏ��̃J������ wchar_t* �Ŏ擾(���s����-1��Ԃ��܂��B�}�C�i�X��ID�͂Ȃ����̂Ƃ��Ă̓���ł��BSQL�Ŏ擾�ł���1�s�ځA1��ڂ̃f�[�^��int�Ŏ擾���ĕԂ��܂��B)
	std::wstring front_wstr(const wchar_t* sql);
	std::wstring front_wstrf(const wchar_t* sql, ...);

	// format �`
	// SQL���t�H�[�}�b�g���Ŏ��s���܂��B
	bool execf(const wchar_t* fmt, ...);
	// SQL�����s���܂��Bselect ���A�߂�l�Ńf�[�^�Z�b�g������ꍇ�͂�����
	db_sqlite_dataset_autoptr selectf(const wchar_t* fmt, ...);
	// ID�l�擾�`(���s����-1��Ԃ��܂��B�}�C�i�X��ID�͂Ȃ����̂Ƃ��Ă̓���ł��BSQL�Ŏ擾�ł���1�s�ځA1��ڂ̃f�[�^��int�Ŏ擾���ĕԂ��܂��B)
	__int64 front_int64f(const wchar_t* fmt,...);

	// sqlite �ɂ� mprintf16 ���Ȃ��̂ŁA�V�~�����[�g
	std::wstring vmprintf();
	std::wstring mprintf(const wchar_t* fmt,...);

	const wchar_t* error_msg(){ return (const wchar_t*)sqlite3_errmsg16(m_db); }

public: // �g������
	// CSV �� TSV �t�@�C������̃C���|�[�g
	bool import(const wchar_t* zFile, const wchar_t* zTable, const wchar_t* zSeparator);

	// update,delete,insert���Ɏ����Ńg�����U�N�V�������J�n����B
	void set_autobegin(bool autobegin){ m_autobegin = autobegin; }

	// ���s���e�w���exec
	bool exec_begin()		{ _on_begin(0); return exec(L"BEGIN"); };
	bool exec_commit()		{ _on_commit(0); return exec(L"COMMIT"); };
	bool exec_rollback()	{ _on_rollback(0); return exec(L"ROLLBACK"); };

	bool exec_update(const wchar_t* table, const wchar_t* key, __int64 keyval, const wchar_t* setsqlfmt, ...);
	bool exec_update(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* setsqlfmt, ...);
	bool exec_updatev(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* (*setsqlfmt));

	bool exec_delete(const wchar_t* table, const wchar_t* key, __int64 keyval);
	bool exec_delete(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval);

	bool exec_insert(const wchar_t* table, const wchar_t* insert_sql_after_table); // insert �Ɋւ��Ă͂��܂���C�ł͂Ȃ��B
	bool exec_insertf(const wchar_t* table, const wchar_t* insert_sql_after_table, ...); // insert �Ɋւ��Ă͂��܂���C�ł͂Ȃ��B

	bool exec_insert_sql(const wchar_t* table, const wchar_t* sql); // insert �Ɋւ��Ă͂��܂���C�ł͂Ȃ��B

	//bool exec_insert(const wchar_t* table, const wchar_t* key, __int64 keyval, const wchar_t* setsql);

private: // �����o�ϐ���
	// �f�[�^�x�[�X�N���X
	::sqlite3* m_db;

	bool m_autobegin; // update,delete,insert���Ɏ����Ńg�����U�N�V�������J�n����B

	// �f�o�b�O�p�ɍŌ��SQL���s��ێ�
	#ifdef _DEBUG
	std::wstring m_lastsql;
	#endif

};

// update ���s��SQL
inline bool db_sqlite::exec_update(const wchar_t* table, const wchar_t* key, __int64 keyval, const wchar_t* setsqlfmt, ...)
{
	std::vector<__int64> v;
	v.push_back(keyval);
	return exec_updatev(table, key, v, &setsqlfmt);
}

// update ���s��SQL
inline bool db_sqlite::exec_update(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* setsqlfmt, ...)
{
	return exec_updatev(table, key, keyval, &setsqlfmt);
}

// update ���s��SQL
inline bool db_sqlite::exec_updatev(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* (*setsqlfmt))
{
	// �����g�����U�N�V����
	if( m_autobegin && !is_transaction() ) {
		exec_begin();
	}

	// set ���̐���
	wchar_t buf_set[1024];
	va_snwprintf(buf_set, 1024, setsqlfmt);

	// SQL ���̐���
	std::wstring sql;
	sql = ssc::wstrprintf(L"update [%s] set %s where [%s] in (", table, buf_set, key);
	for(int i=0; i<keyval.size(); i++) {
		if(i!=0) sql += L",";
		sql += ssc::wstrprintf(L"%I64d", keyval[i]);
	}
	sql += L")";

	_on_update_arg arg = {table, key, &keyval, sql.c_str()};

	_on_pre_update(&arg);

	bool b = exec(sql.c_str());

	_on_post_update(&arg);

	return b;
}

inline bool db_sqlite::exec_delete(const wchar_t* table, const wchar_t* key, __int64 keyval)
{
	std::vector<__int64> v;
	v.push_back(keyval);
	return exec_delete(table, key, v);
}

// update ���s��SQL
inline bool db_sqlite::exec_delete(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval)
{
	// �����g�����U�N�V����
	if( m_autobegin && !is_transaction() ) {
		exec_begin();
	}

	// SQL ���̐���
	std::wstring sql;
	sql = ssc::wstrprintf(L"delete from [%s] where [%s] in (", table, key);
	for(int i=0; i<keyval.size(); i++) {
		if(i!=0) sql += L",";
		sql += ssc::wstrprintf(L"%I64d", keyval[i]);
	}
	sql += L")";

	_on_delete_arg arg = {table, key, &keyval, sql.c_str()};

	_on_pre_delete(&arg);

	bool b = exec(sql.c_str());

	_on_post_delete(&arg);

	return b;
}

inline bool db_sqlite::exec_insert_sql(const wchar_t* table, const wchar_t* sql)
{
	_on_insert_arg arg = {table, sql};

	_on_pre_insert(&arg);

	bool b = exec(sql);

	_on_post_insert(&arg);

	return b;
}

// insert �Ɋւ��Ă͂��܂���C�ł͂Ȃ��B
// insert ���s
inline bool db_sqlite::exec_insert(const wchar_t* table, const wchar_t* insert_sql_after_table)
{
	// �����g�����U�N�V����
	if( m_autobegin && !is_transaction() ) {
		exec_begin();
	}

	// SQL ���̐���
	std::wstring sql;
	sql = ssc::wstrprintf(L"insert into %s %s", table, insert_sql_after_table);

	return exec_insert_sql(table, sql.c_str());
}

// insert �Ɋւ��Ă͂��܂���C�ł͂Ȃ��B
// insert ���s
inline bool db_sqlite::exec_insertf(const wchar_t* table, const wchar_t* insert_sql_after_table, ...)
{
	return exec_insert( table, ssc::va_wstrprintf(&insert_sql_after_table).c_str() );
}

// ���s
inline bool db_sqlite::exec(const wchar_t* sql)
{
	// �f�o�b�O�p�ɍŌ��SQL���s��ێ�
	#ifdef _DEBUG
	m_lastsql = sql;
	#endif

	sqlite3_stmt* stm; // �X�e�[�g�����g���s
	const wchar_t* sql_one = sql;	// ���s����SQL�̎c��
	bool err=false; // �G���[�L��


	// �S�X�e�b�v�����s����
	while( *sql_one != L'\0' && !err ) {
		int ret = sqlite3_prepare16(m_db, sql_one, -1, &stm, (const void**)&sql_one);
		// SQL���@����
		if( ret == SQLITE_OK ) {
			if( stm != NULL ) {
				// SQL�R�}���h���s
				int nRet = sqlite3_step(stm);
				bool ret = nRet == SQLITE_DONE;

				// �s�̂����̓V�J�g�B
				if(!ret) {
					if(nRet == SQLITE_ROW ) {
						ret=true;
					}
				}
				// �X�e�[�g�����g�폜
				sqlite3_finalize(stm);

				// �G���[����
				if(!ret) {
					err = true;
				}

				// �󔒕���i�߂�
				while(isspace(*sql_one))sql_one++;
			}
		}
		// ���@���߂Ɏ��s
		else {
			err = true;
		}
	}

	// �G���[�C�x���g����
	if( err ) {
		_on_error(L"db_sqlite::exec() \nerr:%s\nSQL:%s\nAll SQL:%s", sqlite3_errmsg16(m_db), sql_one, sql);
	}
	else {
		_on_exec(sql); // ���s�����C�x���g
	}

	return (!err); // ���s���Ă��Ȃ���ΐ����B
}

// SQL�����s���܂��B
inline db_sqlite_dataset_autoptr db_sqlite::select(const wchar_t* sql)
{
	// �f�o�b�O�p�ɍŌ��SQL���s��ێ�
	#ifdef _DEBUG
	m_lastsql = sql;
	#endif

	db_sqlite_dataset* pDataset = new db_sqlite_dataset;

	// new ���s
	if(pDataset == 0 ) {
		_on_error(L"db_sqlite::select() / db_sqlite_dataset ��new�Ɏ��s���܂����B\n%s", sql);
		return 0;
	}

	// SQL���s������
	if( pDataset->prepare(m_db, sql) == SQLITE_OK && pDataset->m_stm != NULL ) {
		// �R�����g���A�L����SQL�X�e�[�g�����g�łȂ��ƁA�߂�l��OK����stm��NULL�ɂȂ�B
		return db_sqlite_dataset_autoptr(pDataset);
	}

	_on_error(L"db_sqlite::select() / �L���ȃX�e�[�g�����g�����݂��܂���ł����B[%s]\n%s", sqlite3_errmsg16(m_db), sql);
	// ���s������A�L���ȃX�e�[�g�����g�����݂��Ȃ������ꍇ�B
	delete pDataset;
	return db_sqlite_dataset_autoptr(NULL);
}

// select���Ŏ擾�ł���ŏ��̃J������ wchar_t* �Ŏ擾(���s����-1��Ԃ��܂��B�}�C�i�X��ID�͂Ȃ����̂Ƃ��Ă̓���ł��BSQL�Ŏ擾�ł���1�s�ځA1��ڂ̃f�[�^��int�Ŏ擾���ĕԂ��܂��B)
inline std::wstring db_sqlite::front_wstr(const wchar_t* sql)
{
	db_sqlite_dataset_autoptr p = select(sql);

	if(p.get() != 0 && p->step()) {
		const wchar_t* s = p->column_wchar(0);
		if(s!=0) {
			return s;
		}
	}

	return L"";
}
// select���Ŏ擾�ł���ŏ��̃J������ wchar_t* �Ŏ擾(���s����-1��Ԃ��܂��B�}�C�i�X��ID�͂Ȃ����̂Ƃ��Ă̓���ł��BSQL�Ŏ擾�ł���1�s�ځA1��ڂ̃f�[�^��int�Ŏ擾���ĕԂ��܂��B)
inline std::wstring db_sqlite::front_wstrf(const wchar_t* sql, ...)
{
	return front_wstr( va_wstrprintf(&sql).c_str() );
}

// ID�l�擾�`(���s����-1��Ԃ��܂��B�}�C�i�X��ID�͂Ȃ����̂Ƃ��Ă̓���ł��BSQL�Ŏ擾�ł���1�s�ځA1��ڂ̃f�[�^��int�Ŏ擾���ĕԂ��܂��B)
inline __int64 db_sqlite::front_int64(const wchar_t* sql)
{
	db_sqlite_dataset_autoptr p = select(sql);

	if(p.get() != 0 && p->step()) {
		return p->column_int64(0);
	}

	// _on_error(L"db_sqlite::front_int64() / step �̌Ăяo���Ɏ��s���܂����B"); -1 �͑z��̓���̉\��������
	return -1;
}

// SQL���t�H�[�}�b�g���Ŏ��s���܂��B
inline bool db_sqlite::execf(const wchar_t* fmt, ...)
{
	wchar_t buf[4096];
	va_snwprintf(buf,4096,&fmt);

	return exec(buf);
}

// SQL�����s���܂��Bselect ���A�߂�l�Ńf�[�^�Z�b�g������ꍇ�͂�����
inline db_sqlite_dataset_autoptr db_sqlite::selectf(const wchar_t* fmt, ...)
{
	wchar_t buf[4096];
	va_snwprintf(buf,4096,&fmt);

	return select(buf);
}

// SQL�����s���܂��Bselect ���A�߂�l�Ńf�[�^�Z�b�g������ꍇ�͂�����
inline __int64 db_sqlite::front_int64f(const wchar_t* fmt,...)
{
	wchar_t buf[4096];
	va_snwprintf(buf,4096,&fmt);

	return front_int64(buf);
}

inline std::wstring db_sqlite::mprintf(const wchar_t* fmt,...)
{
	std::wstring strFmt=fmt;
	// %q��%s�ɒu������
	int pos=0;

	// �u������
	while( (pos = strFmt.find(L"%q", pos)) != std::wstring::npos ) {
		strFmt.replace(pos, wcslen(L"%s"), L"%s");
	}

	wchar_t buf[1024];
	va_snwprintf(buf,1024,&fmt);

	return std::wstring(buf);
}

// CSV �� TSV �t�@�C������̃C���|�[�g
inline bool db_sqlite::import(const wchar_t* zFile, const wchar_t* zTable, const wchar_t* zSeparator)
{
	sqlite3_stmt *pStmt;        /* A statement */
	int rc;                     /* Result code */
	int nCol;                   /* Number of columns in the table */
	int nByte;                  /* Number of bytes in an SQL string */
	int i, j;                   /* Loop counters */
	int nSep;                   /* Number of bytes in p->separator[] */
	//char *zSql;                 /* An SQL statement */
	wchar_t *zLine;                /* A single line of input from the file */
	wchar_t **azCol;               /* zLine[] broken up into columns */
	wchar_t *zCommit;              /* How to commit changes */
	FILE *in;                   /* The input file */
	int lineno = 0;             /* Line number of input file */

	nSep = wcslen(zSeparator);
	if( nSep==0 ){
		_on_import_error(L"non-null separator required for import\n");
		return false;
	}
	std::wstring zSql = mprintf(L"SELECT * FROM '%q'", zTable);
	nByte = zSql.size() * sizeof(wchar_t);

	rc = sqlite3_prepare16(m_db, zSql.c_str(), -1, &pStmt, 0);

	if( rc ){
	  _on_import_error(L"Error: %s\n", sqlite3_errmsg16(m_db));
	  nCol = 0;
	  rc = 1;
	}else{
	  nCol = sqlite3_column_count(pStmt);
	}
	sqlite3_finalize(pStmt);
	if( nCol==0 ) {
		return true;
	}

	zSql = mprintf(L"INSERT INTO '%q' VALUES(?", zTable);
	j = zSql.size();
	for(i=1; i<nCol; i++){
	  zSql += L", ?";
	}
	zSql += ')';

	rc = sqlite3_prepare16(m_db, zSql.c_str(), -1, &pStmt, 0);

	if( rc ){
		_on_import_error(L"Error: %s\n", sqlite3_errmsg16(m_db));
		sqlite3_finalize(pStmt);
		return false;
	}
	in = _wfopen(zFile, L"rb");
	if( in==0 ){
		_on_import_error(L"cannot open file: %s\n", zFile);
		sqlite3_finalize(pStmt);
		return false;
	}

	azCol = (wchar_t**)malloc( sizeof(azCol[0])*(nCol+1) );
	if( azCol==0 ){
		fclose(in);
		return false;
	}
	this->exec(L"BEGIN");
	zCommit = L"COMMIT";

	wchar_t buf[2048];

	while( (zLine = fgetws(buf,2048,in))!=0 ){
		wchar_t *z;
		i = 0;
		lineno++;
		azCol[0] = zLine;
		for(i=0, z=zLine; *z && *z!='\n' && *z!='\r'; z++){
			if( *z==zSeparator[0] && wcsncmp(z, zSeparator, nSep)==0 ){
				*z = 0;
				i++;
				if( i<nCol ){
					azCol[i] = &z[nSep];
					z += nSep-1;
				}
			}
		}
		*z = 0;
		if( i+1!=nCol ){
			_on_import_error(L"%s line %d: expected %d columns of data but found %d\n",zFile, lineno, nCol, i+1);
			zCommit = L"ROLLBACK";
			break;
		}
		for(i=0; i<nCol; i++){
			sqlite3_bind_text16(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
		}
		sqlite3_step(pStmt);
		rc = sqlite3_reset(pStmt);
		if( rc!=SQLITE_OK ){
			_on_import_error(L"Error: %s\n", sqlite3_errmsg16(m_db));
			zCommit = L"ROLLBACK";
			rc = 1;
			break;
		}
	}
	free(azCol);
	fclose(in);
	sqlite3_finalize(pStmt);

	this->exec(zCommit);

	return true;
}

}

#endif // #ifndef DatabaseSQLite_h
