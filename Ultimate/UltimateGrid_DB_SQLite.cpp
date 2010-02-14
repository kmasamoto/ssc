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

	// �܂Ƃ߂ČĂяo���p�̃C���^�[�t�F�C�X�B
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
	m_nTitleRow = 0; // �^�C�g���s�͂Ȃ��B
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

	// �y�[�X�g����ɂ��āA�����I���������ꍇ�͓����l���ǂ�ǂ�R�s�[������B
	// �����s�I�����Ă̓\��t���̏ꍇ�́A1�s�̂ݎ�t����B
	if( IsMultiSelected() ) {
		// 1�s�f�[�^�H
		wchar_t* tab = wcschr(buf, L'\t');
		wchar_t* cr = wcschr(buf, L'\r');
		wchar_t* lf = wcschr(buf, L'\n');


		// ���sorTAB���������H
		bool bMultiData = (tab != 0)||(cr != 0)||(lf != 0);
		if(bMultiData) {
			_OnError(L"�����s�̃f�[�^�𕡐��̑I��͈͂ɓ\��t���邱�Ƃ͂ł��܂���B");
			return UG_ERROR;
		}

		// �\��t�������̎��s
		int col;
		long row;
		if( EnumFirstSelected(&col, &row) == UG_SUCCESS) {
			do {
				PasteOne(col,row,buf);
			} while( EnumNextSelected(&col,&row) == UG_SUCCESS );
		}
	}
	// �����I�������Ă��Ȃ���΁A�N���b�v�{�[�h�̓��e���m�F���ď������s���B
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
	// �I�����������s
	this->UltimateGrid_SSCEx::OnRowChange(row, m_GI->m_currentRow);

	string.ReleaseBuffer();

	ClearSelections();
	SelectRange(m_GI->m_currentCol,m_GI->m_currentRow,maxcol,maxrow);

	// �ĕ`������������~
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

// db_sqlite ��ݒ�
void UltimateGrid_DB_SQLite::SetDB_SQLite(ssc::db_sqlite* p_DB_SQLite)
{
	m_pDB_SQLite = p_DB_SQLite;

	// �폜������A�b�v�f�[�g�ɂ��ʒu�̕ύX���ɑΉ����Ȃ��Ƃ�����B
	EVENT_CALL_CLASS(m_pDB_SQLite->_on_post_delete, OnDelete);
	EVENT_CALL_CLASS(m_pDB_SQLite->_on_post_update, OnUpdate);
}

// �����폜
void UltimateGrid_DB_SQLite::SetAutoDelete(bool bIsAutoDelete, const wchar_t* szDeleteView)
{
	m_strDeleteView = szDeleteView;
	m_bIsAutoDelete = bIsAutoDelete;
}

// �����A�b�v�f�[�g
void UltimateGrid_DB_SQLite::SetAutoUpdate(bool bIsAutoUpdate, const wchar_t* szUpdateView)
{
	m_strUpdateView = szUpdateView;
	m_bIsAutoUpdate = bIsAutoUpdate;
}
// �����C���T�[�g
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

// �X�V�J�n�i�_�u���N���b�N�j
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
	// �}�����������s���폜
	this->DeleteRow(m_nInsertEditRow);

	// �}���s��ݒ�
	m_nInsertRow = m_nInsertEditRow;

	// �ҏW��Ԃ�ݒ�
	m_eEditState = ES_NONE;

	// �A�C�R�����Đݒ�
	// �ǉ��A�C�R����ݒ�
	CUGCell		cell;
	this->GetCell(-1,m_nInsertRow,&cell);
	cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
	this->SetCell(-1,m_nInsertRow,&cell);
}

// �X�V�J�n�i�L�[�{�[�h���́j
void UltimateGrid_DB_SQLite::OnKeyDown(_OnKeyDown_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	if(!m_bIsAutoInsert && !m_bIsAutoUpdate){
		return;
	}

	// �}���L�����Z��
	if( IsInsertEditing() && *arg->vcKey == VK_ESCAPE )  {
		CancelInsert();
	}
	else if( isupper(*arg->vcKey) || isdigit(*arg->vcKey) || (*arg->vcKey == '-') || (*arg->vcKey == L'-') ) {
		// �V���[�g�J�b�g�L�[�H
		if(*arg->vcKey == L'V' && ::GetKeyState(VK_CONTROL)) {
			// ctrl + v �������ꂽ�ꍇ�� WM_PASTE �𐶐�
			this->SendMessage(WM_PASTE);
			return;
		}
		// �V���[�g�J�b�g�L�[�H
		else if(*arg->vcKey == L'C' && ::GetKeyState(VK_CONTROL)) {
			// ctrl + v �������ꂽ�ꍇ�� WM_PASTE �𐶐�
			this->SendMessage(WM_COPY);
			return;
		}
		// �V���[�g�J�b�g�L�[�H
		else if(*arg->vcKey == L'X' && ::GetKeyState(VK_CONTROL)) {
			// ctrl + v �������ꂽ�ꍇ�� WM_PASTE �𐶐�
			this->SendMessage(WM_CUT);
			return;
		}
		else {
			this->StartEdit(*arg->vcKey);
		}
	}
	// IME�J�n
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

		// �폜�r���[���n����Ă��Ȃ��̂ŉ������܂���B
		if(m_strDeleteView == L"") {
			_OnError(L"�����폜�̃r���[���ݒ肳��Ă��܂���B");
			return;
		}
		if(m_strSelectSQL == L"") {
			_OnError(L"�폜��ɍĕ\������ׂ�select�����ݒ肳��Ă��܂���B");
			return;
		}

		// �폜�������{�B
		_OnDelete_arg a;
		a.ret=1;
		_OnDelete(&a);

		if( a.ret == 1 ) {
			int nRow = GetCurrentRow();
			// �폜���s
			//bool b = m_pDB_SQLite->execf(L"delete from %s where %s = %d", m_strDeleteView.c_str(), m_strKeyCol.c_str(), m_vecKeys[nRow]);
			bool b = m_pDB_SQLite->exec_delete(m_strDeleteView.c_str(), m_strKeyCol.c_str(), m_vecKeys[nRow]);

			// ���s�ɐ���������A���̉�ʂ��č\���B
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
				_OnError(L"�폜�Ɏ��s���܂����B������폜�s�r���[���w�肳��Ă��Ȃ����m�F���Ă��������B\n[%s]", m_pDB_SQLite->error_msg());
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
		// �ĕ`����ĊJ
		SetRedraw(TRUE);
		AdjustComponentSizes();
		m_CUGVScroll->Invalidate();
		m_CUGHScroll->Invalidate();
		Invalidate();
		RedrawAll();
	}
}

// �s�ύX
void UltimateGrid_DB_SQLite::OnRowChange(_OnRowChange_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;

	OnRowChangeImplement(arg->oldrow, arg->newrow);
}

 // �Z���ړ��O�m�F
void UltimateGrid_DB_SQLite::OnCanMove(_OnCanMove_arg* arg)
{
	if( arg->oldrow != arg->newrow ) {
		arg->ret = OnRowChangeImplement(arg->oldrow, arg->newrow);
	}
}

// �X�V�J�n�i�_�u���N���b�N�j
void UltimateGrid_DB_SQLite::OnEditContinue(_OnEditContinue_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	arg->ret = 0;
	this->GotoCell(*arg->newcol,*arg->newrow);
}

int UltimateGrid_DB_SQLite::OnRowChangeImplement(int oldrow, int newrow)
{
	if(m_eGridState != GS_NORMAL) return 1; // �ړ�OK

	int nAutoNumberCol=-1;

	// �G�f�B�b�g���Ȃ�X�V���s
	if(m_eEditState == ES_EDITING) {
		m_eEditState = ES_NONE; // �ҏW��Ԃ͂Ȃ�

		// �ǉ��A�C�R���𖢐ݒ�ɂ���
		CUGCell		cell;
		this->GetCell(-1,oldrow,&cell);
		cell.SetIcon(0, 16 );
		this->SetCell(-1,oldrow,&cell);

		Invalidate();
	}
	// �}���ҏW���Ȃ�}�����s
	else if( IsInsertEditing() ) {
		// ���ۂɑ}���������s��
		// ��\���̗�̓f�t�H���g�l��ݒ��SQL���쐬����B
		std::wstring s = L"(";

		// �f�[�^��ݒ�
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
				assert(0); // ��\����̓f�t�H���g�l������K�v������܂��B
			}
			s += L", ";
		}
		for(int i=0; i<m_vecColnames.size(); i++) {
			const wchar_t* p = this->QuickGetText(i, m_nInsertEditRow);
			if(p==0) {
				p=L"";
			}

			// �X�V�Z���擾
			CString strSetValue= p;
			_OnSQLInsert_arg a = {i, m_vecColnames[i].c_str(), &strSetValue };
			_OnSQLInsert(&a);

			if( strSetValue == L"(�I�[�g�i���o�[)" ) {
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
			_OnError(L"�ǉ��Ɏ��s���܂����B�s���ȃf�[�^�������Ă��Ȃ����m�F���Ă��������B%s", m_pDB_SQLite->error_msg());
			return 0;
		}
		else {
			__int64 n = m_pDB_SQLite->front_int64f(L"select max(%s) from %s", m_strKeyCol.c_str(), m_strInsertView.c_str());
			m_vecKeys.push_back( n );

			// �ҏW��Ԃ�ݒ�
			m_eEditState = ES_NONE;

			// �I�[�g�i���o�[�̈ʒu�ɂ͐V�K�̂h�c��ݒ肷��B
			if(nAutoNumberCol != -1) {
				CString s;
				s.Format(L"%d",n);
				this->QuickSetText(nAutoNumberCol, m_nInsertEditRow, s);
			}

			// �ǉ��A�C�R���𖢐ݒ�ɂ���
			CUGCell		cell;
			this->GetCell(-1,oldrow,&cell);
			cell.SetIcon(0, 16 );
			this->SetCell(-1,oldrow,&cell);

			// �ǉ��A�C�R����ݒ�
			this->GetCell(-1,m_nInsertRow,&cell);
			cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
			this->SetCell(-1,m_nInsertRow,&cell);
		}

		Invalidate();
	}

	return 1;
}

// ��\����̃f�t�H���g�l
void UltimateGrid_DB_SQLite::SetHideCol(int hideColNum)
{
	m_nColOffset = hideColNum;
}

// �X�V�I���i�e�L�X�g�ҏW�j
void UltimateGrid_DB_SQLite::OnEditStart(_OnEditStart_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	OnEditStartImplement(arg->row);
}

// ���ʂ̃e�L�X�g�ҏW�J�n
void UltimateGrid_DB_SQLite::OnEditStartImplement(int row)
{
	if(m_eGridState != GS_NORMAL) return;
	// �ŏI�s�Ȃ�ǉ����s���B
	if( row == m_nInsertRow ) {
		if( IsInsertEditing() ) {
			// �܂��o�^����Ă��Ȃ��͂��B
			int n=0;
		}

		m_eEditState = ES_INSERT_EDITING_NOCHANGE;
		m_nInsertEditRow = m_nInsertRow;

		// �s�ǉ�
		m_nInsertRow++;
		//InsertRow(m_nInsertRow);
		AppendRow();

		// �f�t�H���g�l��ݒ肷��B
		for(int i=0; m_nColOffset + i<m_vecColDefault.size(); i++) {
			if( m_vecColDefault[m_nColOffset+i] != L"NULL" ) {
				this->QuickSetText(i,m_nInsertEditRow, m_vecColDefault[m_nColOffset + i].c_str() );
			}
		}
		SetCellProperty(m_nInsertEditRow);

		// �ǉ��A�C�R����ݒ�
		CUGCell		cell;
		this->GetCell(-1,m_nInsertRow,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_ADD,16), 16 );
		this->SetCell(-1,m_nInsertRow,&cell);

		// �ҏW�A�C�R����ݒ�
		this->GetCell(-1,m_nInsertEditRow,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_ADDGO,16), 16 );
		this->SetCell(-1,m_nInsertEditRow,&cell);

		// �\�����Ō�ɐݒ肷��B
		this->SetTopRow(m_nInsertEditRow+1);
		//this->MoveCurrentRow(0);
		//this->(m_nInsertEditRow);
		Invalidate();
	}
	else if( IsInsertEditing() && m_nInsertEditRow == row) {
		// �ҏW���s
		int n=0;
	}
	else {
		// �ҏW�A�C�R����ݒ�
		CUGCell		cell;
		this->GetCell(-1,row,&cell);
		cell.SetIcon( GetIconRes(IDI_PENCIL_EDIT,16), 16 );
		this->SetCell(-1,row,&cell);

		m_eEditState = ES_EDITING;
		Invalidate();
	}
}

// �e�Z���^�C�v�ɂ����̓C�x���g�Ή�
void UltimateGrid_DB_SQLite::OnCellTypeNotify(_OnCellTypeNotify_arg* arg)
{
	// �R���{�{�b�N�X�I��
	if(arg->ID == UGCT_DROPLIST ){
		// �X�V�m�F
		if( arg->msg == UGCT_DROPLISTSELECT ) {
			// �ύX���������Ă���΍X�V�����B
			// �ύX���������Ă��Ȃ���΍X�V�����Ȃ��B
			if(this->QuickGetText(arg->col,arg->row) != 0) {
				wchar_t* p = *(wchar_t**)arg->param;
				if( wcscmp(this->QuickGetText(arg->col,arg->row), p) == 0 ) {
					arg->ret = FALSE;
					return;
				}
			}
			else {
				//arg->ret = FALSE;
				//_OnError(L"�f�[�^���擾�ł��܂���ł����Brow[%d]col[%d]", arg->row, arg->col);
				//return;
			}
		}

		// �X�V�m��
		if( arg->msg == UGCT_DROPLISTPOSTSELECT ) {
			// �R���{�{�b�N�X�̑I��������
			// �ύX���������Ă����̂œo�^���������s����B
			OnDataChange(*(wchar_t**)arg->param, arg->row, arg->col);
		}
	}

	// �`�F�b�N�{�b�N�X
	if(arg->ID == UGCT_CHECKBOX ){
		const wchar_t* t = QuickGetText(arg->col,arg->row);
		OnDataChange(t, arg->row, arg->col);
	}
}

BOOL UltimateGrid_DB_SQLite::OnDataChange(const wchar_t* newval, int row, int col)
{
	// �ŏI�s�ɑ΂���ύX�Ȃ�A�X�V�����͍s��Ȃ��B
	if(m_eEditState == ES_INSERT_EDITING_NOCHANGE && m_nInsertEditRow == row) {
		// �ύX����B
		m_eEditState = ES_INSERT_EDITING_CHANGED;
		return TRUE;
	}
	// �ŏI�s�ɑ΂���ύX���p��
	else if( m_eEditState == ES_INSERT_EDITING_CHANGED && m_nInsertEditRow == row) {
		// �����ł��Ȃɂ����Ȃ�
		return TRUE;
	}

	// �����X�V�t���O�����Ƃ���Ă��Ȃ���΁A������Ŏ����ŏ������܂��B
	if(m_bIsAutoUpdate) {
		if(m_strUpdateView == L"") {
			assert(m_strUpdateView != L""); // �����ŃA�T�[�g���o��ꍇ�͍X�V�r���[���ݒ肳��Ă��Ȃ��̂Ɏ����X�V���悤�Ƃ��܂����B
			_OnError(L"�X�V�r���[���ݒ肳��Ă��܂���B");
			return FALSE; // �X�V�L�����Z���B
		}
		if(m_pDB_SQLite == 0) {
			assert(0);
			_OnError(L"SQLite �I�u�W�F�N�g�����ݒ�ł��B");
			return FALSE; // �X�V�L�����Z���B
		}

		// �X�V�Z���擾
		CString strSetValue=newval;

		bool bUpdateSuccess = false;

		// �P���R�[�h�����H
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

		// ���s
		if(!bUpdateSuccess) {
			_OnError( L"�X�V�Ɏ��s���܂����B \n[%s]", (wchar_t*)sqlite3_errmsg16(m_pDB_SQLite->sqlite3()) );
			return FALSE; // �X�V�L�����Z���B
		}
	}

	return TRUE;
}

// �X�V�I���i�e�L�X�g�ҏW�j
void UltimateGrid_DB_SQLite::OnPreEditFinish(_OnPreEditFinish_arg* arg)
{
	// �L�����Z���Ȃ���OK���o���B
	if(arg->cancelFlag) {
		return;
	}

	// �P���R�[�h�̏ꍇ�͎������`�Ȃ��B
	if(m_bIsOneRecordGrid) {
		return;
	}

	// ���t�ݒ肾�����ꍇ�͎����Ő��`
	if( m_vecColType[arg->col] == L"DATETIME" ) {
		COleDateTime date;
		arg->ret = date.ParseDateTime(*arg->string,VAR_DATEVALUEONLY);
		if( arg->ret != 0 ) {
			*arg->string = date.Format(L"%Y-%m-%d");
		}
	}
}

// �X�V�I���i�e�L�X�g�ҏW�j
void UltimateGrid_DB_SQLite::OnEditFinish(_OnEditFinish_arg* arg)
{
	if(m_eGridState != GS_NORMAL) return;
	// �L�����Z���Ȃ���OK���o���B
	if(arg->cancelFlag) {
		// �}���ҏW���Ȃ�}���L�����Z��
		if(m_eEditState == ES_INSERT_EDITING_NOCHANGE) {
			CancelInsert();
		}
		return;
	}

	// �ύX���������Ă���΍X�V�����B
	// �ύX���������Ă��Ȃ���΍X�V�����Ȃ��B
	if(this->QuickGetText(arg->col,arg->row) != 0) {
		if( wcscmp(this->QuickGetText(arg->col,arg->row), *arg->string) == 0 ) {
			return;
		}
	}

	// �V�K�}�����͍X�V���Ȃ��B
	if( IsInsertEditing() ) {
		return;
	}

	arg->ret = OnDataChange(*arg->string, arg->row, arg->col);
}

static std::vector<std::wstring> GetSSCValue(const wchar_t* s, const wchar_t* name)
{
	std::vector<std::wstring> ret;

	// �C�x���g�o�^
	const wchar_t* szFind=s;
	while( (szFind = wcsstr(szFind,name)) != 0 ) {
		// �l���m�F
		const wchar_t* szValue = szFind + wcslen(name);
		// �󔒂�����
		while(isspace(*szValue)) szValue++;

		// ���s������
		const wchar_t* szEnd=szValue;
		while(*szEnd!=L'\0' && *szEnd!=L'\n' && *szEnd!=L'\r') szEnd++;

		// �X�V���ꂽ�ۂɂ�������X�V�B
		std::wstring strUpdateTable(szValue, szEnd);
		ret.push_back(strUpdateTable);

		// �����J�n�ʒu���ړ�
		szFind=szEnd;
	}

	return ret;
}

// �u���������s��SQL�B�u�����������񂪂����ς�����ꍇ�Ɏ����ōX�V����B( ssc::replace_string )
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
	// ����ύX���ꂽ�����́A����Ɗ֌W����H
	if( m_setReplace.find(arg->find) != m_setReplace.end() ) {
		m_bNeedReplace=true;
		// �\������Ă���΍X�V�����m�F�B
		if( IsVisible() ) {
			this->KillTimer(0);
			this->SetTimer(0, 200, 0);
		}
		else {
			m_bNeedReplace=true;
		}
	}
}

// SQL�f�[�^�Z�b�g����l��ݒ�
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

	// �\���̎��{
	if( m_strSelectSQLBase != L"" ) {
		return ResetSelect();
	}

	return false;
}

void UltimateGrid_DB_SQLite::BuildSelectSQL()
{
	if(m_bNeedReplace) {
		m_strSelectSQLBase = ssc::replace_print(m_strSelectSQLNotReplace.c_str()).c_str();
		// ���[�J���̒u���N���X�������Ă���ꍇ�͂�������g�p����B
		if(m_pReplaceString != 0) {
			m_strSelectSQLBase = m_pReplaceString->replace(m_strSelectSQLBase.c_str());
		}
		
		if(m_bIsOneRecordGrid) {
			std::wstring s = ssc::replace_print(m_strOneRecordGridKeyval.c_str()).c_str();
			// ���[�J���̒u���N���X�������Ă���ꍇ�͂�������g�p����B
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
	// �\���������ɐݒ肳��Ă���SQL���s���ĕ\���X�V����B
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
	// �\���������ɐݒ肳��Ă���SQL���s���ĕ\���X�V����B
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
	// ��\���Ȃ�Ȃɂ����Ȃ��B
	if( !IsVisible() ) {
		m_bResetSelectCalled=true;
		return SetDataset(m_pDB_SQLite->selectf( L"select * from (%s) limit 0", m_strSelectSQL.c_str()));
	}

t.start("\n\n%s(%d):������������ResetSelect �S�́����������� %S", __FILE__, __LINE__, m_strSelectSQL.c_str());
	bool b = false;
	b = SetDataset(m_pDB_SQLite->select(m_strSelectSQL.c_str()));
t.report();

	return b;
}


// SQL�f�[�^�Z�b�g����l��ݒ�
bool UltimateGrid_DB_SQLite::SetDataset(ssc::db_sqlite_dataset_autoptr pDs)
{
	// null����Ȃ���Ύ��s
	if(pDs.isnull()) {
		return false;
	}
	m_bIsSetDataset=true; // �f�[�^�Z�b�g�ݒ蒆


LAPF(t);
	// �ĕ`������������~

	bool bIsVisible=false;
	if( IsVisible() ) {
		bIsVisible=true;
		StopDraw();
	}

LAPF(t);
	m_eGridState = GS_INITING;
	m_eEditState = ES_NONE;

//	// ���w�b�_���A�C�R���\���\�T�C�Y�ɁB
//	SetSH_Width(26);
//	// �w�b�_�͕\���B
//	SetTH_Height(20);

LAPF(t);
	// ���݂̈ʒu��ێ�
	long nOldRow = this->GetCurrentRow();
	long nOldCol = this->GetCurrentCol();

	// �L�[�Z�b�g����ɂ���
	m_vecKeys.clear();
	m_vecKeysTitle.clear();
	// �񖼃Z�b�g����ɂ���
	m_vecColnames.clear();
	m_vecOffsetColnames.clear();
	// �c���[�L�[�Z�b�g����ɂ���
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

	m_eGridState = GS_NORMAL; // �ݒ芮��
	m_bIsSetDataset=false; // �f�[�^�Z�b�g�ݒ蒆

	// ���݂̈ʒu��ێ�
	this->GotoRow(nOldRow);
	this->GotoCol(nOldCol);

	return ret;
}

bool UltimateGrid_DB_SQLite::SetDataset_OneRecord(ssc::db_sqlite_dataset_autoptr pDs)
{
	if(!pDs.isnull()) {
		pDs->step();
	}

	// �ő�񐔎擾
	int maxcol=0;
	for(int i=0; i<m_vecCellData.size(); i++) {
		if(maxcol < m_vecCellData[i].size()) maxcol = m_vecCellData[i].size();
	}

	// �T�C�Y�ύX
	this->SetNumberRows(m_vecCellData.size());
	this->SetNumberCols(maxcol);

	// �l�̐ݒ�
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
				// �`�F�b�N�{�b�N�X����
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
	// ��^�C�g���ݒ�
	int nCol = pDs->column_count() - m_nColOffset; // �񐔎擾
	// �N���A
	this->SetNumberCols(nCol);
LAPF(t);
	// ��w�b�_�ݒ�
	CUGCell		cell;

	// �L�[������
	m_strKeyCol = pDs->column_name(0);

	// ��\���̗񖼎擾
	for (int i = 0; i < m_nColOffset; i++) {
		m_vecOffsetColnames.push_back(pDs->column_name(i));
	}
LAPF(t);

	// ��^�C�v�ێ����N���A
	m_vecColType.clear();
	m_vecColType.resize(nCol);

	std::map<std::wstring, int > mapvecCollectCol; // ���W��

	int nTitleRow = -1 - m_nTitleRow; // �^�C�g���s�̍s���ɂ��A�ړ�
	this->SetTH_NumberRows(m_nTitleRow+1);
	// ���ۂɐݒ肵�Ă���
	for (int i = 0; i < nCol; i++) {
		const wchar_t* pStr = pDs->column_name(i+m_nColOffset);

		// bool �A�C�R�����ݒ肳��Ă���ꍇ�A�^�C�g�������B
		bool isIcon = m_mapTrueIconList.find(i) != m_mapTrueIconList.end();
		bool isInvisible = m_setTitleInvisibleCol.find(i) != m_setTitleInvisibleCol.end();
		if( !isIcon && !isInvisible ) {
			this->GetCell(0,nTitleRow,&cell);
			cell.SetText(pStr);
			cell.SetAlignment(UG_ALIGNVCENTER | UG_ALIGNLEFT);
			if( m_pHedderFont != 0 ) cell.SetFont(m_pHedderFont);
			this->SetCell(i,nTitleRow,&cell);
		}
	
		// ��^�C�v���擾
		const wchar_t* pType = pDs->column_decltype(i+m_nColOffset);
		if( pType != 0 ) {
			m_vecColType[i] = pType;
		}

		m_vecColnames.push_back(pStr);

		// ���W��
		if( m_setCollectCol.find(pStr) != m_setCollectCol.end() ) {
			mapvecCollectCol[pStr] = i;
		}
	}

	int nCnt=0;
	// �s����ݒ�
	this->SetNumberRows(0,FALSE);

	// �s�f�[�^�B
	// �f�[�^���擾���Đݒ肵�Ă���
	int row=0;
	m_nDBDataCount = 0;
	while(pDs->step()) {
		m_nDBDataCount++;
LAPS(t, "step()");

		// ���W�l�����
		std::set<std::wstring>::iterator it = m_setCollectCol.begin();
		for(;it != m_setCollectCol.end(); it++) {
			m_mapvecCollectColInt64[it->c_str()].push_back( pDs->column_int64(it->c_str()) );
		}
LAPS(t, "m_mapvecCollectColInt64()");

		// �L�[�����
		if(row-m_nTitleRow >= 0) {
			m_vecKeys.push_back(pDs->column_int64(0));
			AppendRow();
		}
		else {
			m_vecKeysTitle.push_back(pDs->column_int64(0));
		}
LAPS(t, "AppendRow()");
		// �c���[�p�̃L�[�����
		if( row-m_nTitleRow >= 0 && m_bIsTree ) {
 			m_vecTreeKey.push_back(pDs->column_int64(m_nTreeKeyCol));
			m_vecTreeParentKey.push_back(pDs->column_int64(m_nParentKeyCol));
		}


		UpdateRow(row-m_nTitleRow, pDs); LAPFX(t, UpdateRow(row-m_nTitleRow, pDs));

		// �ǉ��A�C�R���𖢐ݒ�ɂ���
		CUGCell		cell;
		this->GetCell(-1,row-m_nTitleRow,&cell);
		cell.SetIcon(0, 16 );
		if( row-m_nTitleRow <= 0 && m_pHedderFont != 0 ) cell.SetFont(m_pHedderFont);
		this->SetCell(-1,row-m_nTitleRow,&cell);

LAPS(t, "cell.SetIcon()");
		row++;
	}
LAPSRA(t);

	// �c���[�\�����쐬
	if( m_bIsTree ) {
		m_vecTreeDepth.clear();
		m_vecTreeDepth.resize( m_vecTreeParentKey.size() );
		// �[������
		for(int i=0; i<m_vecTreeParentKey.size(); i++) {
			m_vecTreeDepth[i] = TreeDepth(i+1, m_vecTreeKey[i], 0);
		}
		
		// �ő�[������
		m_nTreeDepthMax=0;
		for(int i=0; i<m_vecTreeDepth.size(); i++) {
			if( m_nTreeDepthMax < m_vecTreeDepth[i]) {
				m_nTreeDepthMax = m_vecTreeDepth[i];
			}
		}

		// �����ɒʏ�ɓ���Ă݂�B
		int nTreeDepth = m_nTreeDepthMax;
		for(int i=0;i<nTreeDepth;i++) {
			InsertCol(1);
		}

		// �T�C�Y�ݒ�
		for(int i=0; i<nTreeDepth; i++) {
			SetColWidth(i,24);
		}
		// �^�C�g�����ړ��B
		CString s;
		QuickGetText(0,-1,&s);
		QuickSetText(0,-1,L"");
		QuickSetText(nTreeDepth,-1,s);


		//int nFirstCol=nTreeDepth+1;

		// �c���[��
		if(m_vecTreeDepth.size() > 0) {
			for(int i=0; i<m_vecTreeDepth.size(); i++) {
				int nCol = m_vecTreeDepth[i];
				bool bIsParent = (m_vecTreeDepth[i] < m_vecTreeDepth[i+1]);

				// �c���[�Z���ɐݒ�
				// �������e�������ꍇ�̓Z����ݒ�
				if( bIsParent ) {
					for(int c = nCol; c <= nTreeDepth; c++) {
						this->GetCell(c,i,&cell);
						cell.SetCellType(m_nExpandIndex);
						// �����Z���̐[���ȊO�ɂ��ݒ肵�Ȃ��Ɠ��삪���������񂾂��ǁA
						// �ݒ肷��ƁA�{�|���\�����ꂿ�Ⴄ�B
						if(c != nCol) {
							cell.SetCellTypeEx(1);
						}
						this->SetCell(c,i,&cell);
					}
					JoinCells(nCol, i, nTreeDepth, i);
				}
				// �������e����Ȃ��ă��[�g�������ꍇ�A��\����+-�Z���ݒ�
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

		// ���[�g�����ׂĕ���B
		int nRoot = (nTreeDepth)*-1;
		m_expand.CollapseAll(0);

		// �ŏ��̃Z�����c���[�̉��ɕ\��������B
		EnableCellOverLap(TRUE);
	}

LAPF(t);
	// �����ǉ����s���ꍇ�͂����ōs�}��
	if( m_bIsAutoInsert ) {
		AppendRow();
		int nRow = GetNumberRows();
		m_nInsertRow = nRow-1;
		//this->InsertRow(row-m_nTitleRow);
		//m_nInsertRow = row-m_nTitleRow;

		// �ǉ��A�C�R����ݒ�
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

// �L�[�l���Ǘ����ɂ��邩�m�F
bool UltimateGrid_DB_SQLite::IsRowData(int keyval, int* pOutRow)
{
	// �^�C�g���s�̌���
	for(int i=0; i<m_vecKeysTitle.size(); i++) {
		if(m_vecKeysTitle[i]==keyval) {
			if(pOutRow!=0) *pOutRow = -m_vecKeysTitle.size() + i ;
			return true;
		}
	}

	// �L�[�s�̌���
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

	// �����ĂȂ��̂Ŋ֌W�Ȃ��B
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

	// ��^�C�g���ݒ�
	int nCol = pDs->column_count() - m_nColOffset; // �񐔎擾

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
					//if(nCol==0)cell.SetCellType(m_nExpandIndex); // ������
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
			// bool �A�C�R�����ݒ肳��Ă���ꍇ�ɂ�������g�p
			if( m_mapTrueIconList.find(i) != m_mapTrueIconList.end()) {
				if(_wtoi(strText)) {
					cell.SetIcon(m_mapTrueIconList[i], m_nIconsize);
				}
				else {
					cell.SetIcon(m_mapFalseIconList[i], m_nIconsize);
				}
			}
			else {
				// �h���b�v�_�E�����X�g�ݒ肳��Ă���ꍇ�ɂ�������g�p�B
				if( m_mapColDroplist.find(i) != m_mapColDroplist.end() ) {
					cell.SetCellType(UGCT_DROPLIST);
					cell.SetCellTypeEx(UGCT_DROPLISTHIDEBUTTON);
					cell.SetLabelText(m_mapColDroplist[i].c_str());
				}
			}
			// �`�F�b�N�{�b�N�X���w�肳��Ă���ꍇ�ɃZ���^�C�v��ύX
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

// �Q�̌��ݔw�i�F
void UltimateGrid_DB_SQLite::SetRow2Color(COLORREF col1, COLORREF col2)
{
	m_2Color1 = col1;
	m_2Color2 = col2;
	m_bEnableRow2Color = TRUE;
}

// �Q�̌��ݔw�i�F
void UltimateGrid_DB_SQLite::EnableRow2Color(BOOL bEnable)
{
	m_bEnableRow2Color = bEnable;
}

// �s�̃L�[ID���擾����
__int64 UltimateGrid_DB_SQLite::GetKeyval(int row)
{
	if(m_vecKeys.size()==0) {
		_OnError(L"�L�[�����݂��Ȃ���ԂŃL�[�擾���s���܂����B");
		return -1;
	}

	return m_vecKeys[row];
}

// ���ݑI�𒆂̍s�̃L�[ID���擾����B
__int64 UltimateGrid_DB_SQLite::GetSelectRowKeyval()
{
	if(m_vecKeys.size() <= GetCurrentRow()) {
		//_OnError(L"UltimateGrid_DB_SQLite::GetSelectRowKeyval �ʒu�����������ł��B size[%d] request[%d]", m_vecKeys.size(), GetCurrentRow()+m_nTitleRow);
		// ���肦��B
		return -1;
	}

	return GetKeyval(GetCurrentRow());
}

// ���ݑI�𒆂̍s�̃L�[ID���擾����B
std::vector<__int64> UltimateGrid_DB_SQLite::GetSelectRangeKeyval()
{
	std::vector<__int64> ret;

	// �I�𒆂̍s����ID���
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
    
    // ���j���[�����[�h
    if (!err) if (!cMenu.LoadMenu(IDM_RCLICK)) err = 1;
    // �T�u���j���[���擾
    if (!err) if ((popupMenuP = cMenu.GetSubMenu(0)) == NULL) err = 1;
    // ���j���[���|�b�v�A�b�v
    if (!err)
    {
        ClientToScreen(&point);
        if (!popupMenuP->TrackPopupMenu(
            TPM_LEFTBUTTON, point.x, point.y, this)) err = 1;
    }
    // ���j���[��j��
    cMenu.DestroyMenu();
    

	CUGCtrl::OnRButtonUp(nFlags, point);
}

// �폜�C�x���g
void UltimateGrid_DB_SQLite::OnDelete(ssc::db_sqlite::_on_delete_arg* arg)
{
	// �֘A�����f�[�^��������č\�z�B
	// �L�[��̖��O�Ŕ��f
	if(wcscmp(arg->key, m_strKeyCol.c_str())==0) {
		// �\���r���[�̐����ς���Ă�����č\�z
		int nDBDataCount = m_pDB_SQLite->front_int64f(L"select count(*) from (%s)", m_strSelectSQL.c_str());
		if( m_nDBDataCount != nDBDataCount ) {
			ResetSelect();
		}
	}
}

// �X�V�C�x���g
void UltimateGrid_DB_SQLite::OnUpdate(ssc::db_sqlite::_on_update_arg* arg)
{
	// �֘A�����f�[�^��������č\�z�B
	// �L�[��̖��O�Ŕ��f
	if(wcscmp(arg->key, m_strKeyCol.c_str())==0) {
		// �\���r���[�̐����ς���Ă�����č\�z
		int nDBDataCount = m_pDB_SQLite->front_int64f(L"select count(*) from (%s)", m_strSelectSQL.c_str());
		if( m_nDBDataCount != nDBDataCount ) {
			//ResetSelect();
			DELAYCALL_CLASS_A0(m_DelayCall, ResetSelect);
			//d.call();
			//_delayResetSelect(0);
			//_delayResetSelect.exec();
		}
		else {
			// �֘A�s���č\�z
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

	// �֘A�e�[�u���X�V��������č\�z
	for(int i=0; i<m_updateevent.size(); i++) {
		// �֘A�e�[�u���H
		if(m_updateevent[i] == arg->table) {
			// �P�񂾂����������OK�B
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

// �L�[���w�肵�čs�̈ړ�
void UltimateGrid_DB_SQLite::SetCurrentRow(int nKeyval)
{
	for(int i=0; i<m_vecKeys.size(); i++) {
		if(nKeyval == m_vecKeys[i]) {
			this->SetTopRow(i);
		}
	}
}

// �L�[���w�肵�čs�̈ړ�
void UltimateGrid_DB_SQLite::SetHedderFont(CFont* pFont)
{
	m_pHedderFont = pFont;
}

// CUGCtrl ���z�֐�
void UltimateGrid_DB_SQLite::OnSetup()
{
	m_nExpandIndex = AddCellType(&m_expand);
}

// �c���[�\������H����ꍇ�͐e�L�[�̗�Ǝq�L�[�̗���w��
void UltimateGrid_DB_SQLite::SetTree(bool isTree, __int64 nTreeKeyCol, __int64 nParentKeyCol)
{
	m_bIsTree = isTree;
	m_nParentKeyCol = nParentKeyCol;
	m_nTreeKeyCol = nTreeKeyCol;
}

// 
void UltimateGrid_DB_SQLite::OnTH_LClicked(_OnTH_LClicked_arg* arg)
{
	// �w�b�_�N���b�N�ŏ����ݒ�
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

// �O���b�h�̃f�[�^�ݒ�
void UltimateGrid_DB_SQLite::SetGridData(const GridData* ar, size_t row, size_t col)
{
	// �����łP���R�[�h�O���b�h�ɂȂ�܂��B
	m_bIsOneRecordGrid = true;

	// ����������Ă����܂��B
	m_pGridData = ar;
	
	std::vector< std::vector< GridData > > vecGridData; // ���f�[�^vector�u���ێ�
	// ��������vector ���쐬
	vecGridData.resize(row);
	for(int i=0; i<row; i++) {
		for(int j=0; j<col; j++) {
			if(ar[i * col + j].text !=0) {
				vecGridData[i].push_back(ar[i * col + j]);
			}
		}
	}

	// �ݒ�������
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

// �P���R�[�h�̐ݒ�ΏۃL�[
void UltimateGrid_DB_SQLite::SetOneRecordGridKey(const wchar_t* keycol, const wchar_t* keyval)
{
}

// �����s�I��
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
