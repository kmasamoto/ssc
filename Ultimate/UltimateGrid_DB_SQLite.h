#ifndef UltimateGrid_DB_SQLite_hpp
#define UltimateGrid_DB_SQLite_hpp

#include "../DB_SQLite.h"
#include "UltimateGrid_SSCEx.h"
#include "../replace_print.h"
#include "../delay_call.h"

//cell types
#include "CellTypes/ugctexpand.h"
#include "UltimateGrid_CellType_Checkbox.h"

#include <vector>
#include <string>
#include <set>

enum {
	UGCT_ICON = UGCT_ARROW+1,
};

namespace ssc {

// SQLite にアクセスするグリッド
// １列目にはキーが取得されるようにされていることを前提に動いています。
// デフォルトは2列目から表示します。
// SQLの条件で必要な列がある場合、先頭に追加して、オフセットを設定して使用します。
class UltimateGrid_DB_SQLite : public UltimateGrid_SSCEx
{
	EVENT_USER_CLASS(UltimateGrid_DB_SQLite);
	DELAYCALL_USER_CLASS(UltimateGrid_DB_SQLite);

// デリゲート
public:
	// エラーイベント
	ssc::eventf _OnError;

	// 挿入の値取得デリゲート
	// ここの newval を書き換えることでSQLへ渡す文字列を変更可能
	struct _OnSQL_arg{int col; const wchar_t* colName; CString* newval; };

	// インサートかアップデートが呼び出される時のイベント
	// ここの newval を書き換えることでSQLへ渡す文字列を変更可能
	ssc::event _OnSQLInsertOrUpdate; struct _OnSQLInsertOrUpdate_arg{int col; const wchar_t* colName; CString* newval; };

	// インサートが呼び出される時のイベント
	// ここの newval を書き換えることでSQLへ渡す文字列を変更可能
	ssc::event _OnSQLInsert; struct _OnSQLInsert_arg{int col; const wchar_t* colName; CString* newval; };

	// アップデートが呼び出される時のイベント
	// ここの newval を書き換えることでSQLへ渡す文字列を変更可能
	ssc::event _OnSQLUpdate; struct _OnSQLUpdate_arg{int col; const wchar_t* colName; CString* newval; };

	// 削除イベント
	ssc::event _OnDelete; struct _OnDelete_arg{int ret;}; // ret に１が入っていれば削除処理を行います。

	// 表示イベント
	ssc::event _OnShowWindow; struct _OnShowWindow_arg{BOOL bShow; UINT nStatus;}; // ret に１が入っていれば削除処理を行います。

// 外部インターフェイス
public:
	UltimateGrid_DB_SQLite();
	//void SetUpdateView(db_sqlite* p_DB_SQLite, const wchar_t* szUpdateView);	// 更新用ビューの設定
	void SetDB_SQLite(ssc::db_sqlite* p_DB_SQLite);
	void SetAutoUpdate(bool bIsAutoUpdate, const wchar_t* szUpdateView);	// 自動アップデート
	void SetAutoInsert(bool bIsAutoInsert, const wchar_t* szInsertView, int nDefaults=0, ...);	// 自動インサート
	void SetAutoDelete(bool bIsAutoDelete, const wchar_t* szDeleteView);	// 自動削除
	bool SetDataset(ssc::db_sqlite_dataset_autoptr pDs);	// SQLデータセットから値を設定
	bool SetDataset_Table(ssc::db_sqlite_dataset_autoptr pDs);
	bool SetDataset_OneRecord(ssc::db_sqlite_dataset_autoptr pDs);

	bool SetSelect(const wchar_t* s);	// SQLデータセットから値を設定
	bool SetSelectReplace(const wchar_t* s);	// 置き換えを行うSQL。置き換え文字列がさし変わった場合に自動で更新する。( ssc::replace_string )
	bool SetSelectf(const wchar_t* fmt, ... ){ return SetSelect(ssc::va_wstrprintf(&fmt).c_str()); }	// SQLデータセットから値を設定
	bool SetSelectWhere(const wchar_t* s);	// SQLデータセットから値を設定
	bool SetSelectWheref(const wchar_t* fmt,...){ return SetSelectWhere(ssc::va_wstrprintf(&fmt).c_str()); };

	void SetEdit(bool bIsEdit){ m_bIsEdit = bIsEdit; }
	void BuildSelectSQL();

	bool ResetSelect();
	void SetHideCol(int hideColNum);	// 先頭からの非表示列数とデフォルト値を設定
	void SetTitleRow(int titlerow){m_nTitleRow=titlerow;};	// 先頭からのヘッダ行数を指定。

	void SetColDroplist(int col, ssc::db_sqlite_dataset_autoptr p);
	void SetColBoolIcon(int col, HICON hTrue, HICON hFalse, size_t iconsize);
	void SetColCheckbox(int col, const wchar_t* szTrue, const wchar_t* szFalse);

	bool IsRowData(int keyval, int* pRow=0); // キー値が管理下にあるか確認
	void UpdateRowData(int keyval); // キーの値を指定して、データ表示の更新
	void UpdateRow(int row, ssc::db_sqlite_dataset_autoptr& pDs);
	void SetCellProperty(int row);

	// 描画の開始停止
	void StartDraw();
	void StopDraw();

	// 行単位の２トンカラーの設定
	void SetRow2Color(COLORREF col1, COLORREF col2);
	void EnableRow2Color(BOOL bEnable);

	__int64 GetSelectRowKeyval(); // 現在選択中の行のキーIDを取得する。
	std::vector<__int64> GetSelectRangeKeyval(); // 現在選択中の行のキーIDを取得する。
	__int64 GetKeyval(int row); // 行のキーIDを取得する

	void SetHedderFont(CFont* pFont);

	void SetTree(bool isTree, __int64 nTreeKeyCol, __int64 nParentKeyCol); // ツリー表示する？する場合は親キーの列と子キーの列を指定
	void Tree_CollapseAll();

	bool IsVisible();
	void SetColTitleInvisible(int col);

	void FixColumn();
	void CollectColInt64(const wchar_t* sz);
	__int64 GetCollectColInt64(const wchar_t* sz, int row);

	bool IsSetDataset(){ return m_bIsSetDataset; } // データセット設定中？

	void SetAutoFixColumn(bool bAutoFixColumn){ m_bIsAutoFixColumn=bAutoFixColumn; }

	int GetDataCol(int col);

	// １レコード表示グリッド用処理
	struct GridData {
		const wchar_t* text;
		const wchar_t* label;
		int type;
		HICON hIcon;
		int align;
	};
	#define GDALIGNRIGHT(t) {t,0,0,0,UG_ALIGNRIGHT}
	#define GDCHECK(t,l,i) {t,l,m_RadioCheckCellIndex,GetIconRes(i,16),0}
	//GridData GD_Align(const wchar_t* text, int align){ GridData g={0}; g.text=text; g.align=align; return g; };

	void SetGridData(const GridData* ar, size_t row, size_t col);
	void SetIsOneRecordGrid(bool bIsOneRecordGrid){ m_bIsOneRecordGrid = bIsOneRecordGrid; };
	void SetOneRecordGridKey(const wchar_t* keycol, const wchar_t* key); // １レコードの設定対象キー

	void SetReplace(ssc::replace_string* pReplaceString);

	bool IsMultiSelected();

public:
	void SetCurrentRow(int nKeyval); // キーを指定して行の移動

// 内部デリゲート呼び出し
private:
	void OnDClicked(_OnDClicked_arg* arg);	// 更新開始（ダブルクリック）
	//void OnCharDown(_OnCharDown_arg* arg);	// 更新開始（キーボード入力）
	void OnKeyDown(_OnKeyDown_arg* arg);		// 更新開始（キーボード入力）
	void OnKeyUp(_OnKeyUp_arg* arg);			// 更新開始（キーボード入力）
	void OnRowChange(_OnRowChange_arg* arg);	// 行変更
	void OnPreEditFinish(_OnPreEditFinish_arg* arg);	// 更新終了（テキスト編集）
	void OnEditFinish(_OnEditFinish_arg* arg);	// 更新終了（テキスト編集）
	void OnEditStart(_OnEditStart_arg* arg);	// 更新終了（テキスト編集）
	void OnEditContinue(_OnEditContinue_arg* arg);	// 更新終了（テキスト編集）
	void OnCellTypeNotify(_OnCellTypeNotify_arg* arg);	// 各セルタイプによる入力イベント対応
	void OnCanMove(_OnCanMove_arg* arg); // セル移動前確認
	void OnTH_LClicked(_OnTH_LClicked_arg* arg); // 

	BOOL OnDataChange(const wchar_t* newval, int row, int col);

	void OnDelete(ssc::db_sqlite::_on_delete_arg* arg);
	void OnUpdate(ssc::db_sqlite::_on_update_arg* arg);

	//
	int OnRowChangeImplement(int oldrow, int newrow); // 共通の行変更処理
	void OnEditStartImplement(int row);	// 共通のテキスト編集開始
	bool IsInsertEditing(); // 挿入入力中か？
	void CancelInsert(); // 挿入キャンセル処理

	// 
	void on_replace_print_set(_on_replace_print_set_arg* arg);

	void OnAddDelayCall();
public:
	// CUGCtrl 仮想関数
	virtual void OnSetup();

// 実装
private:
	int Paste();
	int Paste(CString &string);
	int PasteOne(int col, int row, CString string);

	// ツリーの深さを調査
	int TreeDepth(int nBegin, int nTreeKey, int nDepth);


	//{{AFX_DATA(UltimateGrid_DB_SQLite)
	//}}AFX_DATA

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(UltimateGrid_DB_SQLite)
	public:
	protected:
	//}}AFX_VIRTUAL

public:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(UltimateGrid_DB_SQLite)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	//}}AFX_MSG
	LONG OnPaste(UINT wParam, LONG lParam);
	LONG OnCopy(UINT wParam, LONG lParam);
	LONG OnCut(UINT wParam, LONG lParam);
	DECLARE_MESSAGE_MAP()

// メンバ変数
protected:
	ssc::event_delay _delayResetSelect;
	void _DelayResetSelect(){ ResetSelect(); }

	ssc::event_delay _delayUpdateRowData;
	void _DelayUpdateRowData(void* p){ UpdateRowData((int)p); }

	ssc::event_delay _delayBuildSelectSQL;
	void _DelayBuildSelectSQL(){ BuildSelectSQL(); }

protected:
	// ツリー化用のセルタイプ
	CUGExpandType		m_expand;
	int					m_nExpandIndex;

	// ツリー関連の保持値
	bool m_bIsTree;
	int m_nParentKeyCol;
	int m_nTreeKeyCol;
	std::vector<__int64> m_vecTreeParentKey;	// 親キーの列
	std::vector<__int64> m_vecTreeKey;		// 自費目キーの列
	std::vector<__int64> m_vecTreeDepth;		// 深さ
	int m_nTreeDepthMax;

	std::vector<__int64> m_vecKeys; // 1列目のキー
	std::vector<__int64> m_vecKeysTitle; // 1列目のキー
	int m_nColOffset; // 表示を開始する列のオフセット
	std::vector<std::wstring> m_vecColDefault; // デフォルト値
	std::vector<std::wstring> m_vecColType; // 列のタイプ


	std::wstring	m_strSelectSQLNotReplace;	// 表示対象のビュー(Replace)
	std::set<std::wstring> m_setReplace;		// 置き換え文字列リスト

	std::wstring	m_strSelectSQLBase;			// 表示対象のビュー
	std::wstring	m_strWhere;					// 検索条件
	std::wstring	m_strSelectSQL;				// 表示対象のビュー

	std::wstring	m_strUpdateView; // 更新対象のビュー
	std::wstring	m_strInsertView; // 更新対象のビュー
	std::wstring	m_strDeleteView; // 削除対象のビュー
	std::wstring	m_strKeyCol; // 更新対象のビュー
	ssc::db_sqlite*		m_pDB_SQLite; // 更新対象のデータベース

	//std::map<int, db_sqlite_dataset_autoptr > m_mapColDroplistDataset; // 列のドロップダウンリスト
	std::map<int, std::wstring > m_mapColDroplist; // 列のドロップダウンリスト
	std::map<int, HICON > m_mapTrueIconList; // 列のboolアイコンリスト
	std::map<int, HICON > m_mapFalseIconList; // 列のboolアイコンリスト
	size_t						 m_nIconsize;		// アイコンサイズ

	// 更新用SQL
	std::vector<std::wstring> m_vecColnames; // 1列目のキー
	std::vector<std::wstring> m_vecOffsetColnames; // 1列目のキー

	// グリッド状況
	enum GridState {
		GS_NOT_INIT,
		GS_INITING,
		GS_NORMAL,
	};
	GridState m_eGridState;

	// 設定
	bool m_bIsAutoUpdate;
	bool m_bIsAutoInsert;
	bool m_bIsAutoDelete;
	bool m_bIsEdit;

	// 挿入状態
	// ①変更開始（新規行IDの確定、保持）新規追加用行の表示
	// ②変更中（updateをかけない）
	// ③変更確定（Insert　行移動 失敗しなければ確定して挿入します。）
	// ④失敗なら行移動させない。オレの好きな動作じゃないけど…
	enum EditState {
		ES_NONE,
		ES_EDITING,
		ES_INSERT_EDITING_EDITCONTINUE,
		ES_INSERT_EDITING_NOCHANGE,
		ES_INSERT_EDITING_CHANGED,
	};
	EditState m_eEditState;
	int m_nInsertRow;		// 挿入行
	int m_nInsertEditRow;	// 挿入行の編集中行

	int m_nStopDrawCnt; // 描画停止回数
	int m_nTitleRow; // タイトル行
	int m_nDBDataCount;

	BOOL m_bEnableRow2Color;
	COLORREF m_2Color1;
	COLORREF m_2Color2;

	CFont* m_pHedderFont;  // ヘッダ用のフォント

	struct CheckboxCol {
		int nCol;
		std::wstring strTrue;
		std::wstring strFalse;
	};
	std::map<int, CheckboxCol> m_mapCheckboxCol;

	bool m_bResetSelectCalled; // リセットSelect 関数が呼び出されて、処理されなかったことがある？
	bool m_bShowWindowProcess; // 表示処理中？
	bool m_bNeedReplace; // 文字列置換必要？

	std::set<int> m_setTitleInvisibleCol;
	
	std::map<std::wstring, std::vector<__int64> > m_mapvecCollectColInt64;	// intキー値として収集した値。
	std::set<std::wstring> m_setCollectCol;

	bool m_bIsSetDataset; // データセット設定中？
	bool m_bIsAutoFixColumn; // 自動的に列をFixする。

	std::vector<std::wstring> m_updateevent;
	std::vector<std::wstring> m_insertevent;

	// 加工データ
	struct Datatype {
		bool isDatasetColumn;
		std::wstring value;

		Datatype(){ isDatasetColumn=false; }
		void create(const std::wstring& s)
		{
			// '%'が先頭なら、データベースの列データを表示対象とする。
			int pos = s.find(L'%');
			if(pos != -1) {
				isDatasetColumn=true;
				value = s.substr(pos+1);
			}
			else {
				isDatasetColumn=false;
				value = s;
			}
		}
		const wchar_t* get(ssc::db_sqlite_dataset* p)
		{
			const wchar_t* ret=0;
			if(isDatasetColumn) {
				if( p != 0 ) {
					ret = p->column_wchar(value.c_str());
				}
			}
			else {
				ret = value.c_str();
			}

			if(ret != 0) {
				return ret;
			}

			return L"";
		}
	};
	struct CellData : public GridData {
		Datatype text;
		Datatype label;
		
		CellData& create(const GridData& v)
		{
			*(GridData*)this = v;
			// '|'を境にしてラベルテキストを分ける。
			if(v.text != 0) {
				text.create(v.text);
			}
			if(v.label != 0) {
				label.create(v.label);
			}

			return *this;
		}
	};
	// SetGridData 処理用データ
	const GridData* m_pGridData; // 生データ
	std::vector< std::vector< CellData > > m_vecCellData; // 加工データ保持
	bool m_bIsOneRecordGrid; // 1レコード用グリッド？

	ssc::replace_string* m_pReplaceString; // ローカルの置換用クラス
	std::wstring m_strOneRecordGridKeycol; // キー
	std::wstring m_strOneRecordGridKeyval; // キー
	__int64 m_nOneRecordGridKeyval; // キー

	UltimateGrid_CellType_Checkbox m_RadioCheckCell; // １セル１ラジオのセル描画
	int m_RadioCheckCellIndex; // １セル１ラジオのセル描画

	delay_call m_DelayCall; // 遅延呼び出し処理
};

} // namespace ssc {
#endif // #ifndef UltimateGrid_DB_SQLite_hpp

