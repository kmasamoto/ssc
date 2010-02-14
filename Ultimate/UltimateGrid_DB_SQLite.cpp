#include "stdafx.h"
#include "UltimateGrid_DB_SQLite.h"
#include "../mfcutil.h"
#include <stdarg.h>
#include "../dbg_timer.h"
#include "../replace_print.h"

namespace ssc {

BEGIN_MESSAGE_MAP(UltimateGrid_DB_SQLite, UltimateGrid_SSCEx)
	//{{MSG_MAP(UltimateGrid_DB_SQLite)
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PASTE,OnPaste)
	ON_MESSAGE(WM_COPY,OnCopy)
	ON_MESSAGE(WM_CUT,OnCut)
END_MESSAGE_MAP()

UltimateGrid_DB_SQLite::UltimateGrid_DB_SQLite()
{
	//{{AFX_DATA_INIT(UltimateGrid_DB_SQLite)
	//}}AFX_DATA_INIT

	EVENT_CALL_CLASS( _OnEditStart, OnEditStart);
	EVENT_CALL_CLASS( _OnPreEditFinish, OnPreEditFinish);
	EVENT_CALL_CLASS( _OnEditFinish, OnEditFinish);
	EVENT_CALL_CLASS( _OnDClicked, OnDClicked );
	EVENT_CALL_CLASS( _OnKeyDown, OnKeyDown );
	EVENT_CALL_CLASS( _OnKeyUp, OnKeyUp );
	EVENT_CALL_CLASS( _OnRowChange, OnRowChange );
	EVENT_CALL_CLASS( _OnEditContinue, OnEditContinue );
	EVENT_CALL_CLASS( _OnCellTypeNotify, OnCellTypeNotify );
	EVENT_CALL_CLASS( _OnCanMove, OnCanMove );
	EVENT_CALL_CLASS( _OnTH_LClicked, OnTH_LClicked );

	// まとめて呼び出す用のインターフェイス。
	EVENT_CALL_EVENT(_OnSQLInsert, _OnSQLInsertOrUpdate);
	EVENT_CALL_EVENT(_OnSQLUpdate, _OnSQLInsertOrUpdate);

	EVENT_CALL_CLASS(ssc::_on_replace_print_set, on_replace_print_set);

	EVENT_CALL_CLASS(m_DelayCall._on_add, OnAddDelayCall);

	EVENT_CALL_CLASS(_delayResetSelect, _DelayResetSelect);
	EVENT_CALL_CLASS(_delayUpdateRowData, _DelayUpdateRowData);
	EVENT_CALL_CLASS(_delayBuildSelectSQL, _DelayBuildSelectSQL);

	m_nColOffset = 0;
	m_pDB_SQLite=0;
	m_eEditState = ES_NONE;
	m_eGridState = GS_NOT_INIT;

	m_bIsAutoInsert=false;
	m_bIsAutoUpdate=false;
	m_bIsAutoDelete=false;
	m_bIsEdit = true;

	m_bEnableRow2Color = FALSE;

	m_nStopDrawCnt = 0;
	m_nTitleRow = 0; // タイトル行はなし。
	m_pHedderFont = 0;

	m_bIsTree = false;
	m_bResetSelectCalled = false;

	m_bShowWindowProcess = false;
	m_bNeedReplace = false;

	m_bIsAutoFixColumn = false;

	m_bIsOneRecordGrid = false;

	m_pReplaceString = 0;

	m_nTreeDepthMax = 0;

	m_RadioCheckCellIndex = this->AddCellType(&m_RadioCheckCell);
}

LONG UltimateGrid_DB_SQLite::OnPaste(UINT wParam, LONG lParam)
{
	if(!m_bIsAutoInsert && !m_bIsAutoUpdate){
		return UG_ERROR;
	}

	return this->Paste();
}

LONG UltimateGrid_DB_SQLite::OnCopy(UINT wParam, LONG lParam)
{
	return this->CopySelected();
}

LONG UltimateGrid_DB_SQLite::OnCut(UINT wParam, LONG lParam)
{
	if(!m_bIsAutoInsert && !m_bIsAutoUpdate){
		return UG_ERROR;
	}

	return this->CutSelected();
}

/***************************************************
Paste
	Pastes the contents of the clipboard into the
	grid - starting from the current cell
  return
	UG_SUCCESS	success
	UG_ERROR	fail
****************************************************/
int UltimateGrid_DB_SQLite::Paste(){
	CString	string;

	//get the text from the clipboard
	if(CopyFromClipBoard(&string)!= UG_SUCCESS)
		return UG_ERROR;

	if( Paste(string) == UG_ERROR ) {
		ResetSelect();
		return UG_ERROR;
	}

	return UG_SUCCESS;
}
/***************************************************
Paste
	Pastes the contents of the clipboard into the
	grid - starting from the current cell
  return
	UG_SUCCESS	success
	UG_ERROR	fail
****************************************************/
int UltimateGrid_DB_SQLite::Paste(CString &string)
{
	if(!m_bIsAutoInsert && !m_bIsAutoUpdate){
		return UG_ERROR;
	}

	int		col		= m_GI->m_currentCol;
	long	row		= m_GI->m_currentRow;
	int		pos		=0;
	int     endpos;
	int		len;
	CString	sub;
	LPTSTR	buf;
	CUGCell	cell;
	TCHAR	endchar = 0;
	int		maxcol = col;
	long	maxrow = row;

	buf = string.GetBuffer(1);
	len = string.GetLength();

	if( !m_pDB_SQLite->is_transaction() ) {
		m_pDB_SQLite->exec_begin();
	}

	// ペースト動作について、複数選択だった場合は同じ値をどんどんコピーさせる。
	// 複数行選択しての貼り付けの場合は、1行のみ受付する。
	if( IsMultiSelected() ) {
		// 1行データ？
		wchar_t* tab = wcschr(buf, L'\t');
		wchar_t* cr = wcschr(buf, L'\r');
		wchar_t* lf = wcschr(buf, L'\n');


		// 改行orTABがあった？
		bool bMultiData = (tab != 0)||(cr != 0)||(lf != 0);
		if(bMultiData) {
			_OnError(L"複数行のデータを複数の選択範囲に貼り付けることはできません。");
			return UG_ERROR;
		}

		// 貼り付け処理の実行
		int col;
		long row;
		if( EnumFirstSelected(&col, &row) == UG_SUCCESS) {
			do {
				PasteOne(col,row,buf);
			} while( EnumNextSelected(&col,&row) == UG_SUCCESS );
		}
	}
	// 複数選択をしていなければ、クリップボードの内容を確認して処理を行う。
	else {
		while(pos < len){

			//check to see if the field is blank
			if(buf[pos]==_T('\t')|| buf[pos]==_T('\r') || buf[pos]==_T('\n')){
				endchar = buf[pos];
				endpos = pos;
				if( col <= GetNumberCols() - 1 && row <= GetNumberRows() - 1 )
				{
					//set a blank cell
					GetCell(col,row,&cell);
					if ( cell.GetReadOnly() != TRUE )
					{
						if(PasteOne(col,row, _T(""))==UG_ERROR) {
							m_pDB_SQLite->exec_rollback();
							return UG_ERROR;
						}
					}
				}
			}
			//find the end of the item then copy the item to the cell
			else{
				endpos = pos+1;
				while(endpos < len){
					endchar = buf[endpos];
					if(endchar == _T('\n') || endchar == _T('\r') || endchar == _T('\t'))
						break;
					endpos++;
				}


				if( col <= GetNumberCols() - 1 && row <= GetNumberRows() - 1 )
				{
					//copy the item
					GetCell(col,row,&cell);

					if ( cell.GetReadOnly() != TRUE )
					{
						sub = string.Mid(pos,endpos-pos);
						if(PasteOne(col,row, sub)==UG_ERROR) {
							m_pDB_SQLite->exec_rollback();
							return UG_ERROR;
						}
					}
				}

				//store the max col and row
				if(col > maxcol)
					maxcol = col;
				if(row > maxrow)
					maxrow = row;
			}

			//set up the position for the next field
			if(endchar == _T('\t')) {
				OnColChange(col, col+1);
				col++;
			}
			if(endchar == _T('\r') || endchar == _T('\n')){
				if( !this->UltimateGrid_SSCEx::OnCanMove(col,row,col+1,row+1) ) {
					m_pDB_SQLite->exec_rollback();
					return UG_ERROR;
				}
				if(col != m_GI->m_currentCol) {
					this->UltimateGrid_SSCEx::OnColChange(col, col+1);
				}
				this->UltimateGrid_SSCEx::OnRowChange(row, row+1);

				col = m_GI->m_currentCol;
				row++;
				if(buf[endpos] == _T('\r') && buf[endpos+1]== _T('\n'))
					endpos++;
			}
			pos = endpos +1;

		}
	}
	// 終了時処理実行
	this->UltimateGrid_SSCEx::OnRowChange(row, m_GI->m_currentRow);

	string.ReleaseBuffer();

	ClearSelections();
	SelectRange(m_GI->m_currentCol,m_GI->m_currentRow,maxcol,maxrow);

	// 再描画をいったん停止
	SetRedraw(TRUE);
	Invalidate();
	RedrawAll();

	m_pDB_SQLite->exec_commit();

	return UG_SUCCESS;
}

int UltimateGrid_DB_SQLite::PasteOne(int col, int row, CString string)
{
	bool bSuccess=false;
	int ret = this->UltimateGrid_SSCEx::OnEditStart(col, row, 0);

	if(ret) {
		int retFinish = this->UltimateGrid_SSCEx::OnEditFinish(col, row, 0, &string, FALSE);
		if(retFinish) {
			bSuccess=true;
			QuickSetText(col,row,string);
		}
	}
	if(!bSuccess){
		return UG_ERROR;
	}

	return UG_SUCCESS;
}

// db_sqlite を設定
void UltimateGrid_DB_SQLite::SetDB_SQLite(ssc::db_sqlite* p_DB_SQLite)
{
	m_pDB_SQLite = p_DB_SQLite;

	// 削除処理やアップデートによる位置の変更等に対応しないといかん。
	EVENT_CALL_CLASS(m_pDB_SQLite->_on_post_delete, OnDelete);
	EVENT_CALL_CLASS(m_pDB_SQLite->_on_post_update, OnUpdate);
}

// 自動削除
void UltimateGrid_DB_SQLite::SetAutoDelete(bool bIsAutoDelete, const wchar_t* szDeleteView)
{
	m_strDeleteView = szDeleteView;
	m_bIsAutoDelete = bIsAutoDelete;
}

// 自動アップデート
void UltimateGrid_DB_SQLite::SetAutoUpdate(bool bIsAutoUpdate, const wchar_t* szUpdateView)
{
	m_strUpdateView = szUpdateView;
	m_bIsAutoUpdate = bIsAutoUpdate;
}
// 自動インサート
void UltimateGrid_DB_SQLite::SetAutoInsert(bool bIsAutoInsert, const wchar_t* szInsertView, int nDefaults, ...)
{
	m_bIsAutoInsert = bIsAutoInsert;
	m_strInsertView = szInsertView;

	m_vecColDefault.clear();

	va_list	arg;
	va_start(arg, nDefaults);

	for(int i=0; i<nDefaults; i++) {
		const wchar_t* p = va_arg(arg, const wchar_t*);
		if(p == 0) {
			m_vecColDefault.push_back(L"");
		}
		else {
			m_vecColDefault.push_back(p);
		}
	}

	va_end(arg);
}

// 更新開始（ダブルクリック）
void UltimateGrid_DB_SQLite::OnDClicked(_OnDClicked_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	if( m_bIsEdit ) this->StartEdit();
}

bool UltimateGrid_DB_SQLite::IsInsertEditing()
{
	return (m_eEditState == ES_INSERT_EDITING_NOCHANGE || m_eEditState == ES_INSERT_EDITING_CHANGED);
}

void UltimateGrid_DB_SQLite::CancelInsert()
{
	// 挿入中だった行を削除
	this->DeleteRow(m_nInsertEditRow);

	// 挿入行を設定
	m_nInsertRow = m_nInsertEditRow;

	// 編集状態を設定
	m_eEditState = ES_NONE;

	// アイコンを再設定
	// 追加アイコンを設定
	CUGCell		cell;
	this->GetCell(-1,m_nInsertRow,&cell);
	cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
	this->SetCell(-1,m_nInsertRow,&cell);
}

// 更新開始（キーボード入力）
void UltimateGrid_DB_SQLite::OnKeyDown(_OnKeyDown_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	if(!m_bIsAutoInsert && !m_bIsAutoUpdate){
		return;
	}

	// 挿入キャンセル
	if( IsInsertEditing() && *arg->vcKey == VK_ESCAPE )  {
		CancelInsert();
	}
	else if( isupper(*arg->vcKey) || isdigit(*arg->vcKey) || (*arg->vcKey == '-') || (*arg->vcKey == L'-') ) {
		// ショートカットキー？
		if(*arg->vcKey == L'V' && ::GetKeyState(VK_CONTROL)) {
			// ctrl + v が押された場合に WM_PASTE を生成
			this->SendMessage(WM_PASTE);
			return;
		}
		// ショートカットキー？
		else if(*arg->vcKey == L'C' && ::GetKeyState(VK_CONTROL)) {
			// ctrl + v が押された場合に WM_PASTE を生成
			this->SendMessage(WM_COPY);
			return;
		}
		// ショートカットキー？
		else if(*arg->vcKey == L'X' && ::GetKeyState(VK_CONTROL)) {
			// ctrl + v が押された場合に WM_PASTE を生成
			this->SendMessage(WM_CUT);
			return;
		}
		else {
			this->StartEdit(*arg->vcKey);
		}
	}
	// IME開始
	else {
		switch(*arg->vcKey) {
		case VK_PROCESSKEY:
		case VK_F2:
		case VK_RETURN:
			this->StartEdit();
			break;
		}
	}
}

void UltimateGrid_DB_SQLite::OnKeyUp(_OnKeyUp_arg* arg)
{
	if(*arg->vcKey == VK_DELETE) {
		if( !m_bIsAutoDelete ) {
			return;
		}

		// 削除ビューが渡されていないので何もしません。
		if(m_strDeleteView == L"") {
			_OnError(L"自動削除のビューが設定されていません。");
			return;
		}
		if(m_strSelectSQL == L"") {
			_OnError(L"削除後に再表示する為のselect文が設定されていません。");
			return;
		}

		// 削除処理実施。
		_OnDelete_arg a;
		a.ret=1;
		_OnDelete(&a);

		if( a.ret == 1 ) {
			int nRow = GetCurrentRow();
			// 削除実行
			//bool b = m_pDB_SQLite->execf(L"delete from %s where %s = %d", m_strDeleteView.c_str(), m_strKeyCol.c_str(), m_vecKeys[nRow]);
			bool b = m_pDB_SQLite->exec_delete(m_strDeleteView.c_str(), m_strKeyCol.c_str(), m_vecKeys[nRow]);

			// 実行に成功したら、この画面を再構成。
			if(b) {
				int nCol = GetCurrentCol();

				int nTopRow=GetTopRow();

				StopDraw();
				ResetSelect();
				GotoCell(nCol, nRow);
				SetTopRow(nTopRow);
				StartDraw();
			}
			else {
				_OnError(L"削除に失敗しました。権限や削除不可ビューが指定されていないか確認してください。\n[%s]", m_pDB_SQLite->error_msg());
			}
		}
	}
}

void UltimateGrid_DB_SQLite::StopDraw()
{
	m_nStopDrawCnt++;
	SetRedraw(FALSE);
}
void UltimateGrid_DB_SQLite::StartDraw()
{
	m_nStopDrawCnt-=1;
	if(m_nStopDrawCnt<=0)m_nStopDrawCnt=0;

	if(m_nStopDrawCnt==0){
		// 再描画を再開
		SetRedraw(TRUE);
		AdjustComponentSizes();
		m_CUGVScroll->Invalidate();
		m_CUGHScroll->Invalidate();
		Invalidate();
		RedrawAll();
	}
}

// 行変更
void UltimateGrid_DB_SQLite::OnRowChange(_OnRowChange_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;

	OnRowChangeImplement(arg->oldrow, arg->newrow);
}

 // セル移動前確認
void UltimateGrid_DB_SQLite::OnCanMove(_OnCanMove_arg* arg)
{
	if( arg->oldrow != arg->newrow ) {
		arg->ret = OnRowChangeImplement(arg->oldrow, arg->newrow);
	}
}

// 更新開始（ダブルクリック）
void UltimateGrid_DB_SQLite::OnEditContinue(_OnEditContinue_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	arg->ret = 0;
	this->GotoCell(*arg->newcol,*arg->newrow);
}

int UltimateGrid_DB_SQLite::OnRowChangeImplement(int oldrow, int newrow)
{
	if(m_eGridState != GS_NORMAL) return 1; // 移動OK

	int nAutoNumberCol=-1;

	// エディット中なら更新実行
	if(m_eEditState == ES_EDITING) {
		m_eEditState = ES_NONE; // 編集状態はなし

		// 追加アイコンを未設定にする
		CUGCell		cell;
		this->GetCell(-1,oldrow,&cell);
		cell.SetIcon(0, 16 );
		this->SetCell(-1,oldrow,&cell);

		Invalidate();
	}
	// 挿入編集中なら挿入実行
	else if( IsInsertEditing() ) {
		// 実際に挿入処理を行う
		// 非表示の列はデフォルト値を設定でSQLを作成する。
		std::wstring s = L"(";

		// データ列設定
		for(int i=0; i<m_vecOffsetColnames.size(); i++) {
			s += m_vecOffsetColnames[i];
			s += L",";
		}
		for(int i=0; i<m_vecColnames.size(); i++) {
			s += m_vecColnames[i];
			s += L",";
		}
		s = s.substr(0, s.length()-1);

		s += L") values(";

		for(int i=0; i<m_vecOffsetColnames.size(); i++) {
			if( m_vecColDefault.size() > i ) {
				if( m_vecColDefault[i] != L"NULL") {
					s += L"'";
					s += m_vecColDefault[i];
					s += L"'";
				}
				else {
					s += m_vecColDefault[i];
				}
			}
			else {
				assert(0); // 非表示列はデフォルト値を入れる必要があります。
			}
			s += L", ";
		}
		for(int i=0; i<m_vecColnames.size(); i++) {
			const wchar_t* p = this->QuickGetText(i, m_nInsertEditRow);
			if(p==0) {
				p=L"";
			}

			// 更新セル取得
			CString strSetValue= p;
			_OnSQLInsert_arg a = {i, m_vecColnames[i].c_str(), &strSetValue };
			_OnSQLInsert(&a);

			if( strSetValue == L"(オートナンバー)" ) {
				nAutoNumberCol = i;
				s += L"NULL";
			}
			else {
				s += L"'";
				s += strSetValue;
				s += L"'";
			}
			if(i+1 < m_vecColnames.size() ) {
				s += L", ";
			}
		}

		s += L")";
		bool b = m_pDB_SQLite->exec_insert(m_strInsertView.c_str(), s.c_str());

		if(!b) {
			_OnError(L"追加に失敗しました。不正なデータが入っていないか確認してください。%s", m_pDB_SQLite->error_msg());
			return 0;
		}
		else {
			__int64 n = m_pDB_SQLite->front_int64f(L"select max(%s) from %s", m_strKeyCol.c_str(), m_strInsertView.c_str());
			m_vecKeys.push_back( n );

			// 編集状態を設定
			m_eEditState = ES_NONE;

			// オートナンバーの位置には新規のＩＤを設定する。
			if(nAutoNumberCol != -1) {
				CString s;
				s.Format(L"%d",n);
				this->QuickSetText(nAutoNumberCol, m_nInsertEditRow, s);
			}

			// 追加アイコンを未設定にする
			CUGCell		cell;
			this->GetCell(-1,oldrow,&cell);
			cell.SetIcon(0, 16 );
			this->SetCell(-1,oldrow,&cell);

			// 追加アイコンを設定
			this->GetCell(-1,m_nInsertRow,&cell);
			cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
			this->SetCell(-1,m_nInsertRow,&cell);
		}

		Invalidate();
	}

	return 1;
}

// 非表示列のデフォルト値
void UltimateGrid_DB_SQLite::SetHideCol(int hideColNum)
{
	m_nColOffset = hideColNum;
}

// 更新終了（テキスト編集）
void UltimateGrid_DB_SQLite::OnEditStart(_OnEditStart_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	OnEditStartImplement(arg->row);
}

// 共通のテキスト編集開始
void UltimateGrid_DB_SQLite::OnEditStartImplement(int row)
{
	if(m_eGridState != GS_NORMAL) return;
	// 最終行なら追加を行う。
	if( row == m_nInsertRow ) {
		if( IsInsertEditing() ) {
			// まだ登録されていないはず。
			int n=0;
		}

		m_eEditState = ES_INSERT_EDITING_NOCHANGE;
		m_nInsertEditRow = m_nInsertRow;

		// 行追加
		m_nInsertRow++;
		//InsertRow(m_nInsertRow);
		AppendRow();

		// デフォルト値を設定する。
		for(int i=0; m_nColOffset + i<m_vecColDefault.size(); i++) {
			if( m_vecColDefault[m_nColOffset+i] != L"NULL" ) {
				this->QuickSetText(i,m_nInsertEditRow, m_vecColDefault[m_nColOffset + i].c_str() );
			}
		}
		SetCellProperty(m_nInsertEditRow);

		// 追加アイコンを設定
		CUGCell		cell;
		this->GetCell(-1,m_nInsertRow,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
		this->SetCell(-1,m_nInsertRow,&cell);

		// 編集アイコンを設定
		this->GetCell(-1,m_nInsertEditRow,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_ADDGO,16), 16 );
		this->SetCell(-1,m_nInsertEditRow,&cell);

		// 表示を最後に設定する。
		this->SetTopRow(m_nInsertEditRow+1);
		//this->MoveCurrentRow(0);
		//this->(m_nInsertEditRow);
		Invalidate();
	}
	else if( IsInsertEditing() && m_nInsertEditRow == row) {
		// 編集続行
		int n=0;
	}
	else {
		// 編集アイコンを設定
		CUGCell		cell;
		this->GetCell(-1,row,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_EDIT,16), 16 );
		this->SetCell(-1,row,&cell);

		m_eEditState = ES_EDITING;
		Invalidate();
	}
}

// 各セルタイプによる入力イベント対応
void UltimateGrid_DB_SQLite::OnCellTypeNotify(_OnCellTypeNotify_arg* arg)
{
	// コンボボックス選択
	if(arg->ID == UGCT_DROPLIST ){
		// 更新確認
		if( arg->msg == UGCT_DROPLISTSELECT ) {
			// 変更が発生していれば更新処理。
			// 変更が発生していなければ更新処理なし。
			if(this->QuickGetText(arg->col,arg->row) != 0) {
				wchar_t* p = *(wchar_t**)arg->param;
				if( wcscmp(this->QuickGetText(arg->col,arg->row), p) == 0 ) {
					arg->ret = FALSE;
					return;
				}
			}
			else {
				//arg->ret = FALSE;
				//_OnError(L"データが取得できませんでした。row[%d]col[%d]", arg->row, arg->col);
				//return;
			}
		}

		// 更新確定
		if( arg->msg == UGCT_DROPLISTPOSTSELECT ) {
			// コンボボックスの選択時処理
			// 変更が発生していたので登録処理を実行する。
			OnDataChange(*(wchar_t**)arg->param, arg->row, arg->col);
		}
	}

	// チェックボックス
	if(arg->ID == UGCT_CHECKBOX ){
		const wchar_t* t = QuickGetText(arg->col,arg->row);
		OnDataChange(t, arg->row, arg->col);
	}
}

BOOL UltimateGrid_DB_SQLite::OnDataChange(const wchar_t* newval, int row, int col)
{
	// 最終行に対する変更なら、更新処理は行わない。
	if(m_eEditState == ES_INSERT_EDITING_NOCHANGE && m_nInsertEditRow == row) {
		// 変更あり。
		m_eEditState = ES_INSERT_EDITING_CHANGED;
		return TRUE;
	}
	// 最終行に対する変更が継続
	else if( m_eEditState == ES_INSERT_EDITING_CHANGED && m_nInsertEditRow == row) {
		// ここでもなにもしない
		return TRUE;
	}

	// 自動更新フラグが落とされていなければ、こちらで自動で処理します。
	if(m_bIsAutoUpdate) {
		if(m_strUpdateView == L"") {
			assert(m_strUpdateView != L""); // ここでアサートが出る場合は更新ビューが設定されていないのに自動更新しようとしました。
			_OnError(L"更新ビューが設定されていません。");
			return FALSE; // 更新キャンセル。
		}
		if(m_pDB_SQLite == 0) {
			assert(0);
			_OnError(L"SQLite オブジェクトが未設定です。");
			return FALSE; // 更新キャンセル。
		}

		// 更新セル取得
		CString strSetValue=newval;

		bool bUpdateSuccess = false;

		// １レコード処理？
		if( m_bIsOneRecordGrid && m_vecCellData[row][col].text.isDatasetColumn) {
			
			_OnSQLUpdate_arg a = {col, m_vecCellData[row][col].text.value.c_str(), &strSetValue };
			_OnSQLUpdate(&a);

			bUpdateSuccess = m_pDB_SQLite->exec_update(
												m_strUpdateView.c_str(), 
												m_strOneRecordGridKeycol.c_str(),
												m_nOneRecordGridKeyval,
												L"[%s]='%s'",
												m_vecCellData[row][col].text.value.c_str(), strSetValue);
		}
		else {
			_OnSQLUpdate_arg a = {col, m_vecColnames[col].c_str(), &strSetValue };
			_OnSQLUpdate(&a);

			bUpdateSuccess = m_pDB_SQLite->exec_update(m_strUpdateView.c_str(),  m_strKeyCol.c_str(), m_vecKeys[row], L"[%s]='%s'", m_vecColnames[col].c_str(), strSetValue);
		}

		// 失敗
		if(!bUpdateSuccess) {
			_OnError( L"更新に失敗しました。 \n[%s]", (wchar_t*)sqlite3_errmsg16(m_pDB_SQLite->sqlite3()) );
			return FALSE; // 更新キャンセル。
		}
	}

	return TRUE;
}

// 更新終了（テキスト編集）
void UltimateGrid_DB_SQLite::OnPreEditFinish(_OnPreEditFinish_arg* arg)
{
	// キャンセルなら常にOKを出す。
	if(arg->cancelFlag) {
		return;
	}

	// １レコードの場合は自動整形なし。
	if(m_bIsOneRecordGrid) {
		return;
	}

	// 日付設定だった場合は自動で整形
	if( m_vecColType[arg->col] == L"DATETIME" ) {
		COleDateTime date;
		arg->ret = date.ParseDateTime(*arg->string,VAR_DATEVALUEONLY);
		if( arg->ret != 0 ) {
			*arg->string = date.Format(L"%Y-%m-%d");
		}
	}
}

// 更新終了（テキスト編集）
void UltimateGrid_DB_SQLite::OnEditFinish(_OnEditFinish_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	// キャンセルなら常にOKを出す。
	if(arg->cancelFlag) {
		// 挿入編集中なら挿入キャンセル
		if(m_eEditState == ES_INSERT_EDITING_NOCHANGE) {
			CancelInsert();
		}
		return;
	}

	// 変更が発生していれば更新処理。
	// 変更が発生していなければ更新処理なし。
	if(this->QuickGetText(arg->col,arg->row) != 0) {
		if( wcscmp(this->QuickGetText(arg->col,arg->row), *arg->string) == 0 ) {
			return;
		}
	}

	// 新規挿入中は更新しない。
	if( IsInsertEditing() ) {
		return;
	}

	arg->ret = OnDataChange(*arg->string, arg->row, arg->col);
}

static std::vector<std::wstring> GetSSCValue(const wchar_t* s, const wchar_t* name)
{
	std::vector<std::wstring> ret;

	// イベント登録
	const wchar_t* szFind=s;
	while( (szFind = wcsstr(szFind,name)) != 0 ) {
		// 値を確認
		const wchar_t* szValue = szFind + wcslen(name);
		// 空白を除去
		while(isspace(*szValue)) szValue++;

		// 改行を検索
		const wchar_t* szEnd=szValue;
		while(*szEnd!=L'\0' && *szEnd!=L'\n' && *szEnd!=L'\r') szEnd++;

		// 更新された際にこちらも更新。
		std::wstring strUpdateTable(szValue, szEnd);
		ret.push_back(strUpdateTable);

		// 検索開始位置を移動
		szFind=szEnd;
	}

	return ret;
}

// 置き換えを行うSQL。置き換え文字列がさし変わった場合に自動で更新する。( ssc::replace_string )
bool UltimateGrid_DB_SQLite::SetSelectReplace(const wchar_t* s)
{
	m_setReplace.clear();
	m_strSelectSQLNotReplace = s;

	std::vector<std::wstring> vec = ssc::replace_print_match_str_list(s);
	for(int i=0; i<vec.size(); i++) {
		m_setReplace.insert(vec[i]);
	}

	if( m_pReplaceString != 0 ) {
		vec = m_pReplaceString->match_str_list(s);
		for(int i=0; i<vec.size(); i++) {
			m_setReplace.insert(vec[i]);
		}
	}

	m_updateevent = GetSSCValue(s, L"--updateevent");
	m_insertevent = GetSSCValue(s, L"--insertevent");

	std::vector<std::wstring> autoupdate = GetSSCValue(s, L"--autoupdate");
	if(!autoupdate.empty()) {
		SetAutoUpdate(true, autoupdate[0].c_str());
	}

	std::vector<std::wstring> hidecol = GetSSCValue(s, L"--hidecol");
	if(!hidecol.empty()) {
		SetHideCol( wcstol(hidecol[0].c_str(), 0, 10) );
	}

	std::vector<std::wstring> v = GetSSCValue(s, L"--keycol");
	if(!v.empty()) {
		m_strOneRecordGridKeycol = v[0];
	}
	v = GetSSCValue(s, L"--keyval");
	if(!v.empty()) {
		m_strOneRecordGridKeyval = v[0];

	}

	m_bNeedReplace = true;
	BuildSelectSQL();
	return ResetSelect(  );
}

void UltimateGrid_DB_SQLite::OnTimer(UINT nIDEvent) 
{
	if( nIDEvent == 0 ) {
		this->KillTimer(0);
		BuildSelectSQL();
		ResetSelect();

		if( m_bIsAutoFixColumn ) {
			FixColumn();
		}
	}
	else if(nIDEvent == 1) {
		KillTimer(1);
		m_DelayCall.call();
	}
}

void UltimateGrid_DB_SQLite::on_replace_print_set(_on_replace_print_set_arg* arg)
{
	// 今回変更された条件は、これと関係する？
	if( m_setReplace.find(arg->find) != m_setReplace.end() ) {
		m_bNeedReplace=true;
		// 表示されていれば更新処理確認。
		if( IsVisible() ) {
			this->KillTimer(0);
			this->SetTimer(0, 200, 0);
		}
		else {
			m_bNeedReplace=true;
		}
	}
}

// SQLデータセットから値を設定
bool UltimateGrid_DB_SQLite::SetSelect(const wchar_t* s)
{
	m_strSelectSQLBase = s;

	BuildSelectSQL();

	return ResetSelect();
}

bool UltimateGrid_DB_SQLite::SetSelectWhere(const wchar_t* s)
{
	m_strWhere = s;

	BuildSelectSQL();

	// 表示の実施
	if( m_strSelectSQLBase != L"" ) {
		return ResetSelect();
	}

	return false;
}

void UltimateGrid_DB_SQLite::BuildSelectSQL()
{
	if(m_bNeedReplace) {
		m_strSelectSQLBase = ssc::replace_print(m_strSelectSQLNotReplace.c_str()).c_str();
		// ローカルの置換クラスを持っている場合はそちらも使用する。
		if(m_pReplaceString != 0) {
			m_strSelectSQLBase = m_pReplaceString->replace(m_strSelectSQLBase.c_str());
		}
		
		if(m_bIsOneRecordGrid) {
			std::wstring s = ssc::replace_print(m_strOneRecordGridKeyval.c_str()).c_str();
			// ローカルの置換クラスを持っている場合はそちらも使用する。
			if(m_pReplaceString != 0) {
				s = m_pReplaceString->replace(s.c_str());
			}
			
			m_nOneRecordGridKeyval = _wtol(s.c_str());
		}
	}

	if( m_strWhere != L"" ) {
		m_strSelectSQL = ssc::wstrprintf(L"select * from (%s) where %s",  m_strSelectSQLBase.c_str(), m_strWhere.c_str());
	}
	else {
		m_strSelectSQL = m_strSelectSQLBase;
	}
}

dbg_timer(t);

void UltimateGrid_DB_SQLite::OnPaint()
{
	// 表示処理時に設定されているSQL実行して表示更新する。
	if(m_strSelectSQL != L"" && m_bResetSelectCalled ) {
		BuildSelectSQL();
		ResetSelect();
		m_bResetSelectCalled = false;
		
		Tree_CollapseAll();

		_OnShowWindow_arg a = {TRUE, 0};
		_OnShowWindow(&a);
	}

	UltimateGrid_SSCEx::OnPaint();
}
void UltimateGrid_DB_SQLite::OnShowWindow(BOOL bShow, UINT nStatus)
{
	// 表示処理時に設定されているSQL実行して表示更新する。
	if(bShow && m_strSelectSQL != L"" && m_bResetSelectCalled ) {
		m_bShowWindowProcess=true;
		BuildSelectSQL();
		ResetSelect();
		m_bShowWindowProcess=false;
		m_bResetSelectCalled = false;

		
		Tree_CollapseAll();
	}
	_OnShowWindow_arg a = {bShow, nStatus};
	_OnShowWindow(&a);
}

bool UltimateGrid_DB_SQLite::IsVisible()
{
	return (this->GetSafeHwnd() != 0 && this->IsWindowVisible()) || m_bShowWindowProcess;
}

bool UltimateGrid_DB_SQLite::ResetSelect()
{
	// 非表示ならなにもしない。
	if( !IsVisible() ) {
		m_bResetSelectCalled=true;
		return SetDataset(m_pDB_SQLite->selectf( L"select * from (%s) limit 0", m_strSelectSQL.c_str()));
	}

t.start("\n\n%s(%d):★☆★☆★☆ResetSelect 全体★☆★☆★☆ %S", __FILE__, __LINE__, m_strSelectSQL.c_str());
	bool b = false;
	b = SetDataset(m_pDB_SQLite->select(m_strSelectSQL.c_str()));
t.report();

	return b;
}


// SQLデータセットから値を設定
bool UltimateGrid_DB_SQLite::SetDataset(ssc::db_sqlite_dataset_autoptr pDs)
{
	// nullじゃなければ実行
	if(pDs.isnull()) {
		return false;
	}
	m_bIsSetDataset=true; // データセット設定中


LAPF(t);
	// 再描画をいったん停止

	bool bIsVisible=false;
	if( IsVisible() ) {
		bIsVisible=true;
		StopDraw();
	}

LAPF(t);
	m_eGridState = GS_INITING;
	m_eEditState = ES_NONE;

//	// 横ヘッダをアイコン表示可能サイズに。
//	SetSH_Width(26);
//	// ヘッダは表示。
//	SetTH_Height(20);

LAPF(t);
	// 現在の位置を保持
	long nOldRow = this->GetCurrentRow();
	long nOldCol = this->GetCurrentCol();

	// キーセットを空にする
	m_vecKeys.clear();
	m_vecKeysTitle.clear();
	// 列名セットを空にする
	m_vecColnames.clear();
	m_vecOffsetColnames.clear();
	// ツリーキーセットを空にする
	m_vecTreeKey.clear();
	m_vecTreeParentKey.clear();
	m_mapvecCollectColInt64.clear();

LAPF(t);
	//this->ResetAll();
	
	int rows = GetNumberRows() - 1;
	int cols = GetNumberCols() - 1;
	ResetCells(0, rows, -2,  cols);

	while(this->GetNumberRows()) {
		this->DeleteRow(0);
	}
LAPF(t);

	bool ret=false;
	if(m_bIsOneRecordGrid) {
		ret = SetDataset_OneRecord(pDs);
	}
	else {
		ret = SetDataset_Table(pDs);
	}

	if( bIsVisible ) {
		StartDraw();
	}

LAPF(t);

	m_eGridState = GS_NORMAL; // 設定完了
	m_bIsSetDataset=false; // データセット設定中

	// 現在の位置を保持
	this->GotoRow(nOldRow);
	this->GotoCol(nOldCol);

	return ret;
}

bool UltimateGrid_DB_SQLite::SetDataset_OneRecord(ssc::db_sqlite_dataset_autoptr pDs)
{
	if(!pDs.isnull()) {
		pDs->step();
	}

	// 最大列数取得
	int maxcol=0;
	for(int i=0; i<m_vecCellData.size(); i++) {
		if(maxcol < m_vecCellData[i].size()) maxcol = m_vecCellData[i].size();
	}

	// サイズ変更
	this->SetNumberRows(m_vecCellData.size());
	this->SetNumberCols(maxcol);

	// 値の設定
	for(int i=0; i<m_vecCellData.size(); i++) {
		for(int j=0; j<m_vecCellData[i].size(); j++) {
			//QuickSetText(j,i,m_vecCellData[i][j].text.get(pDs.get()));
			//QuickSetLabelText(j,i,m_vecCellData[i][j].label.get(pDs.get()));

			CUGCell		cell;
			this->GetCell(j,i,&cell);
			if(m_vecCellData[i][j].text.value != L"") cell.SetText(m_vecCellData[i][j].text.get(pDs.get()));
			if(m_vecCellData[i][j].label.value != L"") cell.SetLabelText(m_vecCellData[i][j].label.get(pDs.get()));
			if(m_vecCellData[i][j].hIcon != 0) cell.SetIcon(m_vecCellData[i][j].hIcon, 16);
			if(m_vecCellData[i][j].type != 0) {
				cell.SetCellType(m_vecCellData[i][j].type);
				// チェックボックス処理
				if( m_vecCellData[i][j].type == UGCT_CHECKBOX) {
					cell.SetCellTypeEx(UGCT_CHECKBOX3DRECESS);
				}
			}
			if(m_vecCellData[i][j].align != 0) cell.SetAlignment(m_vecCellData[i][j].align);
			this->SetCell(j,i,&cell);
		}
	}
	return true;
}


bool UltimateGrid_DB_SQLite::SetDataset_Table(ssc::db_sqlite_dataset_autoptr pDs)
{
	// 列タイトル設定
	int nCol = pDs->column_count() - m_nColOffset; // 列数取得
	// クリア
	this->SetNumberCols(nCol);
LAPF(t);
	// 列ヘッダ設定
	CUGCell		cell;

	// キー列を回収
	m_strKeyCol = pDs->column_name(0);

	// 非表示の列名取得
	for (int i = 0; i < m_nColOffset; i++) {
		m_vecOffsetColnames.push_back(pDs->column_name(i));
	}
LAPF(t);

	// 列タイプ保持をクリア
	m_vecColType.clear();
	m_vecColType.resize(nCol);

	std::map<std::wstring, int > mapvecCollectCol; // 収集列

	int nTitleRow = -1 - m_nTitleRow; // タイトル行の行数により、移動
	this->SetTH_NumberRows(m_nTitleRow+1);
	// 実際に設定していく
	for (int i = 0; i < nCol; i++) {
		const wchar_t* pStr = pDs->column_name(i+m_nColOffset);

		// bool アイコンが設定されている場合、タイトル無し。
		bool isIcon = m_mapTrueIconList.find(i) != m_mapTrueIconList.end();
		bool isInvisible = m_setTitleInvisibleCol.find(i) != m_setTitleInvisibleCol.end();
		if( !isIcon && !isInvisible ) {
			this->GetCell(0,nTitleRow,&cell);
			cell.SetText(pStr);
			cell.SetAlignment(UG_ALIGNVCENTER | UG_ALIGNLEFT);
			if( m_pHedderFont != 0 ) cell.SetFont(m_pHedderFont);
			this->SetCell(i,nTitleRow,&cell);
		}
	
		// 列タイプを取得
		const wchar_t* pType = pDs->column_decltype(i+m_nColOffset);
		if( pType != 0 ) {
			m_vecColType[i] = pType;
		}

		m_vecColnames.push_back(pStr);

		// 収集列
		if( m_setCollectCol.find(pStr) != m_setCollectCol.end() ) {
			mapvecCollectCol[pStr] = i;
		}
	}

	int nCnt=0;
	// 行数を設定
	this->SetNumberRows(0,FALSE);

	// 行データ。
	// データを取得して設定していく
	int row=0;
	m_nDBDataCount = 0;
	while(pDs->step()) {
		m_nDBDataCount++;
LAPS(t, "step()");

		// 収集値を回収
		std::set<std::wstring>::iterator it = m_setCollectCol.begin();
		for(;it != m_setCollectCol.end(); it++) {
			m_mapvecCollectColInt64[it->c_str()].push_back( pDs->column_int64(it->c_str()) );
		}
LAPS(t, "m_mapvecCollectColInt64()");

		// キーを回収
		if(row-m_nTitleRow >= 0) {
			m_vecKeys.push_back(pDs->column_int64(0));
			AppendRow();
		}
		else {
			m_vecKeysTitle.push_back(pDs->column_int64(0));
		}
LAPS(t, "AppendRow()");
		// ツリー用のキーを回収
		if( row-m_nTitleRow >= 0 && m_bIsTree ) {
 			m_vecTreeKey.push_back(pDs->column_int64(m_nTreeKeyCol));
			m_vecTreeParentKey.push_back(pDs->column_int64(m_nParentKeyCol));
		}


		UpdateRow(row-m_nTitleRow, pDs); LAPFX(t, UpdateRow(row-m_nTitleRow, pDs));

		// 追加アイコンを未設定にする
		CUGCell		cell;
		this->GetCell(-1,row-m_nTitleRow,&cell);
		cell.SetIcon(0, 16 );
		if( row-m_nTitleRow <= 0 && m_pHedderFont != 0 ) cell.SetFont(m_pHedderFont);
		this->SetCell(-1,row-m_nTitleRow,&cell);

LAPS(t, "cell.SetIcon()");
		row++;
	}
LAPSRA(t);

	// ツリー構造を作成
	if( m_bIsTree ) {
		m_vecTreeDepth.clear();
		m_vecTreeDepth.resize( m_vecTreeParentKey.size() );
		// 深さ検索
		for(int i=0; i<m_vecTreeParentKey.size(); i++) {
			m_vecTreeDepth[i] = TreeDepth(i+1, m_vecTreeKey[i], 0);
		}
		
		// 最大深さ検索
		m_nTreeDepthMax=0;
		for(int i=0; i<m_vecTreeDepth.size(); i++) {
			if( m_nTreeDepthMax < m_vecTreeDepth[i]) {
				m_nTreeDepthMax = m_vecTreeDepth[i];
			}
		}

		// 試しに通常に入れてみる。
		int nTreeDepth = m_nTreeDepthMax;
		for(int i=0;i<nTreeDepth;i++) {
			InsertCol(1);
		}

		// サイズ設定
		for(int i=0; i<nTreeDepth; i++) {
			SetColWidth(i,24);
		}
		// タイトルを移動。
		CString s;
		QuickGetText(0,-1,&s);
		QuickSetText(0,-1,L"");
		QuickSetText(nTreeDepth,-1,s);


		//int nFirstCol=nTreeDepth+1;

		// ツリー化
		if(m_vecTreeDepth.size() > 0) {
			for(int i=0; i<m_vecTreeDepth.size(); i++) {
				int nCol = m_vecTreeDepth[i];
				bool bIsParent = (m_vecTreeDepth[i] < m_vecTreeDepth[i+1]);

				// ツリーセルに設定
				// 自分が親だった場合はセルを設定
				if( bIsParent ) {
					for(int c = nCol; c <= nTreeDepth; c++) {
						this->GetCell(c,i,&cell);
						cell.SetCellType(m_nExpandIndex);
						// 自分セルの深さ以外にも設定しないと動作がおかしいんだけど、
						// 設定すると、＋－が表示されちゃう。
						if(c != nCol) {
							cell.SetCellTypeEx(1);
						}
						this->SetCell(c,i,&cell);
					}
					JoinCells(nCol, i, nTreeDepth, i);
				}
				// 自分が親じゃなくてルートだった場合、非表示の+-セル設定
				else {
					for(int c = nCol; c < nTreeDepth; c++) {
						this->GetCell(c,i,&cell);
						cell.SetCellType(m_nExpandIndex);
						cell.SetCellTypeEx(1);
						this->SetCell(c,i,&cell);
						int n=0;
					}
					JoinCells(nCol, i, nTreeDepth, i);
				}
				if(nCol != 0 && QuickGetText(0,i) != 0) {
					QuickSetText(nCol, i, QuickGetText(0,i));
					QuickSetText(0,i, L"");
				}
			}
		}

		// ルートをすべて閉じる。
		int nRoot = (nTreeDepth)*-1;
		m_expand.CollapseAll(0);

		// 最初のセルをツリーの横に表示させる。
		EnableCellOverLap(TRUE);
	}

LAPF(t);
	// 自動追加を行う場合はここで行挿入
	if( m_bIsAutoInsert ) {
		AppendRow();
		int nRow = GetNumberRows();
		m_nInsertRow = nRow-1;
		//this->InsertRow(row-m_nTitleRow);
		//m_nInsertRow = row-m_nTitleRow;

		// 追加アイコンを設定
		this->GetCell(-1,m_nInsertRow,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
		this->SetCell(-1,m_nInsertRow,&cell);
	}

	m_nInsertEditRow = m_nInsertRow;

	return true;
}

int UltimateGrid_DB_SQLite::TreeDepth(int nBegin, int nParentKey, int nDepth)
{
	int next = nBegin-1;
	if( next < 0 ) {
		return nDepth;
	}

	if( m_vecTreeKey[next] == nParentKey ) {
		if( m_vecTreeParentKey[next] == 0 ) {
			return nDepth;
		}
		else {
			return TreeDepth(next, m_vecTreeParentKey[nBegin-1], nDepth+1);
		}
	}
	else if(m_vecTreeParentKey[next] == nParentKey) {
		return TreeDepth(next, nParentKey, nDepth);
	}

	return TreeDepth(next, nParentKey, nDepth);
}

void UltimateGrid_DB_SQLite::SetColDroplist(int col, ssc::db_sqlite_dataset_autoptr p)
{

	//m_mapColDroplist[col] = p;

	m_mapColDroplist[col] = L"";

	while(p->step()) {
		m_mapColDroplist[col] += p->column_wchar(0);
		m_mapColDroplist[col] += L"\n";
	}
}

void UltimateGrid_DB_SQLite::SetColBoolIcon(int col, HICON hTrue, HICON hFalse, size_t iconsize)
{
	m_mapFalseIconList[col] = 0;
	m_mapTrueIconList[col] = 0;

	m_nIconsize = iconsize;

	m_mapFalseIconList[col] = hFalse;
	m_mapTrueIconList[col] = hTrue;
}

// キー値が管理下にあるか確認
bool UltimateGrid_DB_SQLite::IsRowData(int keyval, int* pOutRow)
{
	// タイトル行の検索
	for(int i=0; i<m_vecKeysTitle.size(); i++) {
		if(m_vecKeysTitle[i]==keyval) {
			if(pOutRow!=0) *pOutRow = -m_vecKeysTitle.size() + i ;
			return true;
		}
	}

	// キー行の検索
	for(int i=0; i<m_vecKeys.size(); i++) {
		if(m_vecKeys[i]==keyval) {
			if(pOutRow!=0)*pOutRow=i;
			return true;
		}
	}

	return false;
}

void UltimateGrid_DB_SQLite::UpdateRowData(int keyval)
{
	int row = 0;

	// もってないので関係ない。
	if(!IsRowData(keyval, &row)) {
		return;
	}

	ssc::db_sqlite_dataset_autoptr p = m_pDB_SQLite->selectf(L"select * from (%s) where %s=%d",	m_strSelectSQL.c_str(), m_strKeyCol.c_str(), keyval);
	if( !p.isnull() ) {
		if( p->step() ) UpdateRow(row, p);
	}
}

void UltimateGrid_DB_SQLite::UpdateRow(int row, ssc::db_sqlite_dataset_autoptr& pDs)
{
	//row = row - m_nTitleRow;

	// 列タイトル設定
	int nCol = pDs->column_count() - m_nColOffset; // 列数取得

	CUGDataSource* p = this->GetDataSource(0);
	this->RemoveDataSource(0);

	CUGCell		cell;
	for(int i=0; i<nCol; i++) {
		int dataCol = i + m_nColOffset;
		this->GetCell(i,row,&cell);
		//p->GetCell(i,row,&cell);
LAPS(t, "UpdateRow()/GetCell()");
			cell.SetAlignment(UG_ALIGNVCENTER | UG_ALIGNLEFT);
LAPS(t, "UpdateRow()/SetAlignment()");
			cell.SetText(pDs->column_wchar(dataCol));
LAPS(t, "UpdateRow()/SetText( column_wchar )");
			if( m_bEnableRow2Color && 0 <= row) {
				if( row%2==0 ) {
					cell.SetBackColor(m_2Color1);
				}
				else {
					cell.SetBackColor(m_2Color2);
					//if(nCol==0)cell.SetCellType(m_nExpandIndex); // お試し
				}
LAPS(t, "UpdateRow()/cell.SetBackColor()");
			}

		this->SetCell(i,row,&cell);
		//p->GetCell(i,row,&cell);
LAPS(t, "UpdateRow()/SetCell()");

	}
	//this->AddDataSource(p);
	//this->RemoveDataSource(0);

	SetCellProperty(row);
LAPS(t, "UpdateRow()/SetCellProperty()");
}

void UltimateGrid_DB_SQLite::SetCellProperty(int row)
{
	int nCol = GetNumberCols();
	for(int i=0; i<nCol; i++) {
		CUGCell		cell;

		this->GetCell(i,row,&cell);
			const TCHAR* strText = cell.GetText();

			cell.SetAlignment(UG_ALIGNVCENTER | UG_ALIGNLEFT);
			// bool アイコンが設定されている場合にこちらを使用
			if( m_mapTrueIconList.find(i) != m_mapTrueIconList.end()) {
				if(_wtoi(strText)) {
					cell.SetIcon(m_mapTrueIconList[i], m_nIconsize);
				}
				else {
					cell.SetIcon(m_mapFalseIconList[i], m_nIconsize);
				}
			}
			else {
				// ドロップダウンリスト設定されている場合にこちらを使用。
				if( m_mapColDroplist.find(i) != m_mapColDroplist.end() ) {
					cell.SetCellType(UGCT_DROPLIST);
					cell.SetCellTypeEx(UGCT_DROPLISTHIDEBUTTON);
					cell.SetLabelText(m_mapColDroplist[i].c_str());
				}
			}
			// チェックボックスが指定されている場合にセルタイプを変更
			if ( m_mapCheckboxCol.find(i) != m_mapCheckboxCol.end() ) {
				cell.SetCellType(UGCT_CHECKBOX);
				if(_wtoi(strText)) {
					cell.SetLabelText(m_mapCheckboxCol[i].strTrue.c_str());
				}
				else {
					cell.SetLabelText(m_mapCheckboxCol[i].strFalse.c_str());
				}
			}

		this->SetCell(i,row,&cell);
	}
}

// ２つの交互背景色
void UltimateGrid_DB_SQLite::SetRow2Color(COLORREF col1, COLORREF col2)
{
	m_2Color1 = col1;
	m_2Color2 = col2;
	m_bEnableRow2Color = TRUE;
}

// ２つの交互背景色
void UltimateGrid_DB_SQLite::EnableRow2Color(BOOL bEnable)
{
	m_bEnableRow2Color = bEnable;
}

// 行のキーIDを取得する
__int64 UltimateGrid_DB_SQLite::GetKeyval(int row)
{
	if(m_vecKeys.size()==0) {
		_OnError(L"キーが存在しない状態でキー取得が行われました。");
		return -1;
	}

	return m_vecKeys[row];
}

// 現在選択中の行のキーIDを取得する。
__int64 UltimateGrid_DB_SQLite::GetSelectRowKeyval()
{
	if(m_vecKeys.size() <= GetCurrentRow()) {
		//_OnError(L"UltimateGrid_DB_SQLite::GetSelectRowKeyval 位置がおかしいです。 size[%d] request[%d]", m_vecKeys.size(), GetCurrentRow()+m_nTitleRow);
		// ありえる。
		return -1;
	}

	return GetKeyval(GetCurrentRow());
}

// 現在選択中の行のキーIDを取得する。
std::vector<__int64> UltimateGrid_DB_SQLite::GetSelectRangeKeyval()
{
	std::vector<__int64> ret;

	// 選択中の行からIDを列挙
	long row;
	int col;
	if(EnumFirstSelected(&col,&row)==UG_SUCCESS) {
		ret.push_back(m_vecKeys[row]);
		while (EnumNextSelected(&col, &row) == UG_SUCCESS) {
			ret.push_back(m_vecKeys[row]);
		}
	}

	return ret;
}

void UltimateGrid_DB_SQLite::OnRButtonUp(UINT nFlags, CPoint point) 
{
    CMenu    *popupMenuP = NULL;
    CMenu    cMenu;
    int      err = 0;
    
    // メニューをロード
    if (!err) if (!cMenu.LoadMenu(IDM_RCLICK)) err = 1;
    // サブメニューを取得
    if (!err) if ((popupMenuP = cMenu.GetSubMenu(0)) == NULL) err = 1;
    // メニューをポップアップ
    if (!err)
    {
        ClientToScreen(&point);
        if (!popupMenuP->TrackPopupMenu(
            TPM_LEFTBUTTON, point.x, point.y, this)) err = 1;
    }
    // メニューを破棄
    cMenu.DestroyMenu();
    

	CUGCtrl::OnRButtonUp(nFlags, point);
}

// 削除イベント
void UltimateGrid_DB_SQLite::OnDelete(ssc::db_sqlite::_on_delete_arg* arg)
{
	// 関連したデータだったら再構築。
	// キー列の名前で判断
	if(wcscmp(arg->key, m_strKeyCol.c_str())==0) {
		// 表示ビューの数が変わっていたら再構築
		int nDBDataCount = m_pDB_SQLite->front_int64f(L"select count(*) from (%s)", m_strSelectSQL.c_str());
		if( m_nDBDataCount != nDBDataCount ) {
			ResetSelect();
		}
	}
}

// 更新イベント
void UltimateGrid_DB_SQLite::OnUpdate(ssc::db_sqlite::_on_update_arg* arg)
{
	// 関連したデータだったら再構築。
	// キー列の名前で判断
	if(wcscmp(arg->key, m_strKeyCol.c_str())==0) {
		// 表示ビューの数が変わっていたら再構築
		int nDBDataCount = m_pDB_SQLite->front_int64f(L"select count(*) from (%s)", m_strSelectSQL.c_str());
		if( m_nDBDataCount != nDBDataCount ) {
			//ResetSelect();
			DELAYCALL_CLASS_A0(m_DelayCall, ResetSelect);
			//d.call();
			//_delayResetSelect(0);
			//_delayResetSelect.exec();
		}
		else {
			// 関連行を再構築
			for(int i=0; i<arg->keyvalvec->size(); i++) {
				//UpdateRowData(arg->keyvalvec->at(i));
				//m_DelayCall.add(this, &THIS_CLASS::UpdateRowData,(int)arg->keyvalvec->at(i));
				DELAYCALL_CLASS_A1(m_DelayCall, UpdateRowData, (int)arg->keyvalvec->at(i));
				//d.call();
				//_delayUpdateRowData((void*)arg->keyvalvec->at(i));
				//_delayUpdateRowData.exec();
			}
		}
	}

	// 関連テーブル更新だったら再構築
	for(int i=0; i<m_updateevent.size(); i++) {
		// 関連テーブル？
		if(m_updateevent[i] == arg->table) {
			// １回だけ処理すればOK。
			//BuildSelectSQL();
			//_delayBuildSelectSQL(0);
			//_delayBuildSelectSQL.exec();
			DELAYCALL_CLASS_A0(m_DelayCall, BuildSelectSQL);
			//m_DelayCall.add(this, &THIS_CLASS::BuildSelectSQL);

			//ResetSelect();
			//_delayResetSelect(0);
			//_delayResetSelect.exec();
			DELAYCALL_CLASS_A0(m_DelayCall, ResetSelect);
			break;
		}
	}
}

// キーを指定して行の移動
void UltimateGrid_DB_SQLite::SetCurrentRow(int nKeyval)
{
	for(int i=0; i<m_vecKeys.size(); i++) {
		if(nKeyval == m_vecKeys[i]) {
			this->SetTopRow(i);
		}
	}
}

// キーを指定して行の移動
void UltimateGrid_DB_SQLite::SetHedderFont(CFont* pFont)
{
	m_pHedderFont = pFont;
}

// CUGCtrl 仮想関数
void UltimateGrid_DB_SQLite::OnSetup()
{
	m_nExpandIndex = AddCellType(&m_expand);
}

// ツリー表示する？する場合は親キーの列と子キーの列を指定
void UltimateGrid_DB_SQLite::SetTree(bool isTree, __int64 nTreeKeyCol, __int64 nParentKeyCol)
{
	m_bIsTree = isTree;
	m_nParentKeyCol = nParentKeyCol;
	m_nTreeKeyCol = nTreeKeyCol;
}

// 
void UltimateGrid_DB_SQLite::OnTH_LClicked(_OnTH_LClicked_arg* arg)
{
	// ヘッダクリックで条件設定
}

void UltimateGrid_DB_SQLite::SetColCheckbox(int col, const wchar_t* szTrue, const wchar_t* szFalse)
{
	CheckboxCol c;
	c.nCol = col;
	c.strFalse = szFalse;
	c.strTrue = szTrue;
	m_mapCheckboxCol[col] = c;
}

void UltimateGrid_DB_SQLite::SetColTitleInvisible(int col)
{
	m_setTitleInvisibleCol.insert(col);
}

void UltimateGrid_DB_SQLite::FixColumn()
{
	int n = GetSH_Width();
	for(int i=m_nTreeDepthMax+1; i<this->GetNumberCols()-1; i++) {
		this->BestFit(i,i,this->GetNumberRows(),UG_BESTFIT_TOPHEADINGS);
		n += GetColWidth(i);
	}

	CRect r;
	this->GetClientRect(r);
	int nLast = r.Width()-n-this->GetVS_Width();
	SetColWidth(this->GetNumberCols()-1, nLast);
	//SetColWidth(

}

void UltimateGrid_DB_SQLite::CollectColInt64(const wchar_t* sz)
{
	m_setCollectCol.insert(sz);
}

__int64 UltimateGrid_DB_SQLite::GetCollectColInt64(const wchar_t* sz, int row)
{
	return m_mapvecCollectColInt64[sz][row];
}

void UltimateGrid_DB_SQLite::Tree_CollapseAll()
{
	m_expand.CollapseAll(-1);
}

int UltimateGrid_DB_SQLite::GetDataCol(int col)
{
	return col + m_nTreeDepthMax;
}

// グリッドのデータ設定
void UltimateGrid_DB_SQLite::SetGridData(const GridData* ar, size_t row, size_t col)
{
	// 自動で１レコードグリッドになります。
	m_bIsOneRecordGrid = true;

	// 引数を取っておきます。
	m_pGridData = ar;
	
	std::vector< std::vector< GridData > > vecGridData; // 生データvector置換保持
	// 引数からvector を作成
	vecGridData.resize(row);
	for(int i=0; i<row; i++) {
		for(int j=0; j<col; j++) {
			if(ar[i * col + j].text !=0) {
				vecGridData[i].push_back(ar[i * col + j]);
			}
		}
	}

	// 設定情報を解析
	CellData tmp;
	m_vecCellData.resize(vecGridData.size());
	for(int i=0; i<vecGridData.size(); i++) {
		for(int j=0; j<vecGridData[i].size(); j++) {
			m_vecCellData[i].push_back( tmp.create(vecGridData[i][j]) );
		}
	}
}

void UltimateGrid_DB_SQLite::SetReplace(ssc::replace_string* pReplaceString)
{
	m_pReplaceString = pReplaceString;

	if( m_pReplaceString != 0 ) {
		EVENT_CALL_CLASS(m_pReplaceString->_on_set, on_replace_print_set);
	}	
}

// １レコードの設定対象キー
void UltimateGrid_DB_SQLite::SetOneRecordGridKey(const wchar_t* keycol, const wchar_t* keyval)
{
}

// 複数行選択
bool UltimateGrid_DB_SQLite::IsMultiSelected()
{
	int c;
	long r;
	int n = EnumFirstSelected(&c,&r);

	return EnumNextSelected(&c,&r) == UG_SUCCESS;
}

void UltimateGrid_DB_SQLite::OnAddDelayCall()
{
	this->KillTimer(1);
	this->SetTimer(1, 200, 0);
}

} // namespace ssc
