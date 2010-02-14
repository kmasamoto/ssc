#ifndef DB_SQLite_hpp
#define DB_SQLite_hpp

#pragma warning(disable:4786) // STL の警告削除

#include "sqlite3.h"
#include <map>
#include <string>
#include <memory> // auto_ptr
#include <assert.h>
#include "ssc_string.h"
#include "event.h"

namespace ssc {

// データセット
class db_sqlite_dataset
{
public: // 基本操作
	db_sqlite_dataset()		{		m_stm = 0;	}
	~db_sqlite_dataset()	{		if(m_stm != 0) sqlite3_finalize(m_stm);	}

	// データ取得
	int column_count() { return sqlite3_column_count(m_stm); }
	const wchar_t* column_name(int n){ return (const wchar_t*)sqlite3_column_name16(m_stm, n); }
	bool step()	{ return sqlite3_step(m_stm) == SQLITE_ROW; }
	bool reset()	{ return sqlite3_reset(m_stm) != 0; }

	const wchar_t* column_decltype(int col){ return (const wchar_t*)sqlite3_column_decltype16(m_stm, col); }

	// カラムのインデックスは０ベース。
	int column_int(int col)				{ return sqlite3_column_int(m_stm, col); }
	__int64 column_int64(int col) { return sqlite3_column_int64(m_stm, col); }

	// カラムのインデックスは０ベース。
	std::string column_utf8(int col)	{ return std::string ((char const*) ::sqlite3_column_text (m_stm, col), ::sqlite3_column_bytes (m_stm, col)); }
	std::wstring column_wstr(int col)	{ return std::wstring((wchar_t const*) ::sqlite3_column_text16(m_stm, col)); }
	const wchar_t* column_wchar(int col)	{ return (const wchar_t *) ::sqlite3_column_text16(m_stm, col); }

public: // 拡張操作

	// カラムのインデックスは０ベース。
	// カラム名文字列による取得
	int column_int(const wchar_t* col)				{ map(); return column_int(m_mapColnameIndex[col]); }
	int column_int64(const wchar_t* col)				{ map(); return column_int64(m_mapColnameIndex[col]); }
	std::string column_utf8(const wchar_t* col)		{ map(); return column_utf8(m_mapColnameIndex[col]); }
	std::wstring column_wstr(const wchar_t* col)		{ map(); return column_wstr(m_mapColnameIndex[col]); }
	const wchar_t* column_wchar(const wchar_t* col)	{ map(); return column_wchar(m_mapColnameIndex[col]); }

	// 列名マップ
	void map()
	{
		// まだマップされていない場合だけ実行
		if(!m_mapColnameIndex.empty()) return;

		int colCount = sqlite3_column_count(m_stm);
		for (int col = 0; col < colCount; ++col) {
			m_mapColnameIndex[(const wchar_t*)sqlite3_column_name16(m_stm, col)] = col;
		}
	}

private:
	// フレンドクラス
	friend class db_sqlite;

	// データアクセス状態
	sqlite3_stmt* m_stm;
	int prepare(sqlite3* db, const wchar_t* sql) { return sqlite3_prepare16(db, sql, -1, &m_stm, NULL); }

	// 列名とカラムの対応マップ。
	std::map<std::wstring, int> m_mapColnameIndex;

};

// 通常扱う用のデータセット
class db_sqlite_dataset_autoptr : public std::auto_ptr<db_sqlite_dataset>
{
// null チェック
public:
	db_sqlite_dataset_autoptr(db_sqlite_dataset* p) :
		std::auto_ptr<db_sqlite_dataset>(p)
	{
	}

	bool isnull() { return this->get() == 0; };
};

// データベースクラス
class db_sqlite  {
public: // コンストラクタ
	db_sqlite()	{ m_db = 0;	m_autobegin = true; }
	~db_sqlite() {	sqlite3_close(m_db); }

public: // イベント
	ssc::eventf _on_import_error; // インポート処理でエラーが発生
	ssc::eventf _on_error;		// エラー発生

	// Update 呼び出し発生時のイベントデータ
	struct _on_update_arg {
		const wchar_t* table;
		const wchar_t* key;
		std::vector<__int64>* keyvalvec;
		const wchar_t* sql;
	};
	ssc::event _on_pre_update;		// Update 呼び出し発生前
	ssc::event _on_post_update;		// Update 呼び出し発生後

	// Delete 呼び出し発生時のイベントデータ
	struct _on_delete_arg {
		const wchar_t* table;
		const wchar_t* key;
		std::vector<__int64>* keyvalvec;
		const wchar_t* sql;
	};
	ssc::event _on_pre_delete;		// Delete 呼び出し発生前
	ssc::event _on_post_delete;		// Delete 呼び出し発生後

	// insert 呼び出し発生時のイベントデータ
	struct _on_insert_arg {
		const wchar_t* table;
		const wchar_t* sql;
	};
	ssc::event _on_pre_insert;		// insert 呼び出し発生前
	ssc::event _on_post_insert;		// insert 呼び出し発生後

	ssc::eventf _on_exec;	// exec 呼び出し発生

	ssc::event _on_begin;	// select 呼び出し発生
	ssc::event _on_commit;	// select 呼び出し発生
	ssc::event _on_rollback;// select 呼び出し発生

public: // 基本操作
	sqlite3* sqlite3(){ return m_db; }

	// オープン
	int open(const wchar_t* filename){ return sqlite3_open16(filename, &m_db);	}
	int close(){ if(m_db != 0) return sqlite3_close(m_db); return 0;}

	// 最後に挿入した行IDを取得
	int last_insert_rowid(){ return sqlite3_last_insert_rowid(m_db); }

	// トランザクションに入っているか確認
	bool is_transaction(){ return sqlite3_get_autocommit(m_db) == 0; }

	// 実行基本形
	// SQLを実行します。select 等、戻り値でデータセットがある場合はこちら
	db_sqlite_dataset_autoptr select(const wchar_t* sql);
	// SQLを実行します。update, delete, insert 等、戻り値でデータセットがない場合はこちら
	bool exec(const wchar_t* sql);
	// select文で取得できる最初のカラムをint64で取得(失敗時は-1を返します。マイナスのIDはないものとしての動作です。SQLで取得できる1行目、1列目のデータをintで取得して返します。)
	__int64 front_int64(const wchar_t* sql);
	// select文で取得できる最初のカラムを wchar_t* で取得(失敗時は-1を返します。マイナスのIDはないものとしての動作です。SQLで取得できる1行目、1列目のデータをintで取得して返します。)
	std::wstring front_wstr(const wchar_t* sql);
	std::wstring front_wstrf(const wchar_t* sql, ...);

	// format 形
	// SQLをフォーマット文で実行します。
	bool execf(const wchar_t* fmt, ...);
	// SQLを実行します。select 等、戻り値でデータセットがある場合はこちら
	db_sqlite_dataset_autoptr selectf(const wchar_t* fmt, ...);
	// ID値取得形(失敗時は-1を返します。マイナスのIDはないものとしての動作です。SQLで取得できる1行目、1列目のデータをintで取得して返します。)
	__int64 front_int64f(const wchar_t* fmt,...);

	// sqlite には mprintf16 がないので、シミュレート
	std::wstring vmprintf();
	std::wstring mprintf(const wchar_t* fmt,...);

	const wchar_t* error_msg(){ return (const wchar_t*)sqlite3_errmsg16(m_db); }

public: // 拡張操作
	// CSV や TSV ファイルからのインポート
	bool import(const wchar_t* zFile, const wchar_t* zTable, const wchar_t* zSeparator);

	// update,delete,insert時に自動でトランザクションを開始する。
	void set_autobegin(bool autobegin){ m_autobegin = autobegin; }

	// 実行内容指定のexec
	bool exec_begin()		{ _on_begin(0); return exec(L"BEGIN"); };
	bool exec_commit()		{ _on_commit(0); return exec(L"COMMIT"); };
	bool exec_rollback()	{ _on_rollback(0); return exec(L"ROLLBACK"); };

	bool exec_update(const wchar_t* table, const wchar_t* key, __int64 keyval, const wchar_t* setsqlfmt, ...);
	bool exec_update(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* setsqlfmt, ...);
	bool exec_updatev(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* (*setsqlfmt));

	bool exec_delete(const wchar_t* table, const wchar_t* key, __int64 keyval);
	bool exec_delete(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval);

	bool exec_insert(const wchar_t* table, const wchar_t* insert_sql_after_table); // insert に関してはあまり乗り気ではない。
	bool exec_insertf(const wchar_t* table, const wchar_t* insert_sql_after_table, ...); // insert に関してはあまり乗り気ではない。

	bool exec_insert_sql(const wchar_t* table, const wchar_t* sql); // insert に関してはあまり乗り気ではない。

	//bool exec_insert(const wchar_t* table, const wchar_t* key, __int64 keyval, const wchar_t* setsql);

private: // メンバ変数等
	// データベースクラス
	::sqlite3* m_db;

	bool m_autobegin; // update,delete,insert時に自動でトランザクションを開始する。

	// デバッグ用に最後のSQL実行を保持
	#ifdef _DEBUG
	std::wstring m_lastsql;
	#endif

};

// update 実行のSQL
inline bool db_sqlite::exec_update(const wchar_t* table, const wchar_t* key, __int64 keyval, const wchar_t* setsqlfmt, ...)
{
	std::vector<__int64> v;
	v.push_back(keyval);
	return exec_updatev(table, key, v, &setsqlfmt);
}

// update 実行のSQL
inline bool db_sqlite::exec_update(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* setsqlfmt, ...)
{
	return exec_updatev(table, key, keyval, &setsqlfmt);
}

// update 実行のSQL
inline bool db_sqlite::exec_updatev(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval, const wchar_t* (*setsqlfmt))
{
	// 自動トランザクション
	if( m_autobegin && !is_transaction() ) {
		exec_begin();
	}

	// set 文の生成
	wchar_t buf_set[1024];
	va_snwprintf(buf_set, 1024, setsqlfmt);

	// SQL 文の生成
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

// update 実行のSQL
inline bool db_sqlite::exec_delete(const wchar_t* table, const wchar_t* key, std::vector<__int64>& keyval)
{
	// 自動トランザクション
	if( m_autobegin && !is_transaction() ) {
		exec_begin();
	}

	// SQL 文の生成
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

// insert に関してはあまり乗り気ではない。
// insert 実行
inline bool db_sqlite::exec_insert(const wchar_t* table, const wchar_t* insert_sql_after_table)
{
	// 自動トランザクション
	if( m_autobegin && !is_transaction() ) {
		exec_begin();
	}

	// SQL 文の生成
	std::wstring sql;
	sql = ssc::wstrprintf(L"insert into %s %s", table, insert_sql_after_table);

	return exec_insert_sql(table, sql.c_str());
}

// insert に関してはあまり乗り気ではない。
// insert 実行
inline bool db_sqlite::exec_insertf(const wchar_t* table, const wchar_t* insert_sql_after_table, ...)
{
	return exec_insert( table, ssc::va_wstrprintf(&insert_sql_after_table).c_str() );
}

// 実行
inline bool db_sqlite::exec(const wchar_t* sql)
{
	// デバッグ用に最後のSQL実行を保持
	#ifdef _DEBUG
	m_lastsql = sql;
	#endif

	sqlite3_stmt* stm; // ステートメント実行
	const wchar_t* sql_one = sql;	// 実行したSQLの残り
	bool err=false; // エラー有無


	// 全ステップを実行する
	while( *sql_one != L'\0' && !err ) {
		int ret = sqlite3_prepare16(m_db, sql_one, -1, &stm, (const void**)&sql_one);
		// SQL文法解釈
		if( ret == SQLITE_OK ) {
			if( stm != NULL ) {
				// SQLコマンド実行
				int nRet = sqlite3_step(stm);
				bool ret = nRet == SQLITE_DONE;

				// 行のあるやつはシカト。
				if(!ret) {
					if(nRet == SQLITE_ROW ) {
						ret=true;
					}
				}
				// ステートメント削除
				sqlite3_finalize(stm);

				// エラー発生
				if(!ret) {
					err = true;
				}

				// 空白分を進める
				while(isspace(*sql_one))sql_one++;
			}
		}
		// 文法解釈に失敗
		else {
			err = true;
		}
	}

	// エラーイベント発生
	if( err ) {
		_on_error(L"db_sqlite::exec() \nerr:%s\nSQL:%s\nAll SQL:%s", sqlite3_errmsg16(m_db), sql_one, sql);
	}
	else {
		_on_exec(sql); // 実行処理イベント
	}

	return (!err); // 失敗していなければ成功。
}

// SQLを実行します。
inline db_sqlite_dataset_autoptr db_sqlite::select(const wchar_t* sql)
{
	// デバッグ用に最後のSQL実行を保持
	#ifdef _DEBUG
	m_lastsql = sql;
	#endif

	db_sqlite_dataset* pDataset = new db_sqlite_dataset;

	// new 失敗
	if(pDataset == 0 ) {
		_on_error(L"db_sqlite::select() / db_sqlite_dataset のnewに失敗しました。\n%s", sql);
		return 0;
	}

	// SQL実行成功時
	if( pDataset->prepare(m_db, sql) == SQLITE_OK && pDataset->m_stm != NULL ) {
		// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
		return db_sqlite_dataset_autoptr(pDataset);
	}

	_on_error(L"db_sqlite::select() / 有効なステートメントが存在しませんでした。[%s]\n%s", sqlite3_errmsg16(m_db), sql);
	// 失敗したり、有効なステートメントが存在しなかった場合。
	delete pDataset;
	return db_sqlite_dataset_autoptr(NULL);
}

// select文で取得できる最初のカラムを wchar_t* で取得(失敗時は-1を返します。マイナスのIDはないものとしての動作です。SQLで取得できる1行目、1列目のデータをintで取得して返します。)
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
// select文で取得できる最初のカラムを wchar_t* で取得(失敗時は-1を返します。マイナスのIDはないものとしての動作です。SQLで取得できる1行目、1列目のデータをintで取得して返します。)
inline std::wstring db_sqlite::front_wstrf(const wchar_t* sql, ...)
{
	return front_wstr( va_wstrprintf(&sql).c_str() );
}

// ID値取得形(失敗時は-1を返します。マイナスのIDはないものとしての動作です。SQLで取得できる1行目、1列目のデータをintで取得して返します。)
inline __int64 db_sqlite::front_int64(const wchar_t* sql)
{
	db_sqlite_dataset_autoptr p = select(sql);

	if(p.get() != 0 && p->step()) {
		return p->column_int64(0);
	}

	// _on_error(L"db_sqlite::front_int64() / step の呼び出しに失敗しました。"); -1 は想定の動作の可能性が高い
	return -1;
}

// SQLをフォーマット文で実行します。
inline bool db_sqlite::execf(const wchar_t* fmt, ...)
{
	wchar_t buf[4096];
	va_snwprintf(buf,4096,&fmt);

	return exec(buf);
}

// SQLを実行します。select 等、戻り値でデータセットがある場合はこちら
inline db_sqlite_dataset_autoptr db_sqlite::selectf(const wchar_t* fmt, ...)
{
	wchar_t buf[4096];
	va_snwprintf(buf,4096,&fmt);

	return select(buf);
}

// SQLを実行します。select 等、戻り値でデータセットがある場合はこちら
inline __int64 db_sqlite::front_int64f(const wchar_t* fmt,...)
{
	wchar_t buf[4096];
	va_snwprintf(buf,4096,&fmt);

	return front_int64(buf);
}

inline std::wstring db_sqlite::mprintf(const wchar_t* fmt,...)
{
	std::wstring strFmt=fmt;
	// %qを%sに置き換え
	int pos=0;

	// 置き換え
	while( (pos = strFmt.find(L"%q", pos)) != std::wstring::npos ) {
		strFmt.replace(pos, wcslen(L"%s"), L"%s");
	}

	wchar_t buf[1024];
	va_snwprintf(buf,1024,&fmt);

	return std::wstring(buf);
}

// CSV や TSV ファイルからのインポート
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
