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

// SQLite �ɃA�N�Z�X����O���b�h
// �P��ڂɂ̓L�[���擾�����悤�ɂ���Ă��邱�Ƃ�O��ɓ����Ă��܂��B
// �f�t�H���g��2��ڂ���\�����܂��B
// SQL�̏����ŕK�v�ȗ񂪂���ꍇ�A�擪�ɒǉ����āA�I�t�Z�b�g��ݒ肵�Ďg�p���܂��B
class UltimateGrid_DB_SQLite : public UltimateGrid_SSCEx
{
	EVENT_USER_CLASS(UltimateGrid_DB_SQLite);
	DELAYCALL_USER_CLASS(UltimateGrid_DB_SQLite);

// �f���Q�[�g
public:
	// �G���[�C�x���g
	ssc::eventf _OnError;

	// �}���̒l�擾�f���Q�[�g
	// ������ newval �����������邱�Ƃ�SQL�֓n���������ύX�\
	struct _OnSQL_arg{int col; const wchar_t* colName; CString* newval; };

	// �C���T�[�g���A�b�v�f�[�g���Ăяo����鎞�̃C�x���g
	// ������ newval �����������邱�Ƃ�SQL�֓n���������ύX�\
	ssc::event _OnSQLInsertOrUpdate; struct _OnSQLInsertOrUpdate_arg{int col; const wchar_t* colName; CString* newval; };

	// �C���T�[�g���Ăяo����鎞�̃C�x���g
	// ������ newval �����������邱�Ƃ�SQL�֓n���������ύX�\
	ssc::event _OnSQLInsert; struct _OnSQLInsert_arg{int col; const wchar_t* colName; CString* newval; };

	// �A�b�v�f�[�g���Ăяo����鎞�̃C�x���g
	// ������ newval �����������邱�Ƃ�SQL�֓n���������ύX�\
	ssc::event _OnSQLUpdate; struct _OnSQLUpdate_arg{int col; const wchar_t* colName; CString* newval; };

	// �폜�C�x���g
	ssc::event _OnDelete; struct _OnDelete_arg{int ret;}; // ret �ɂP�������Ă���΍폜�������s���܂��B

	// �\���C�x���g
	ssc::event _OnShowWindow; struct _OnShowWindow_arg{BOOL bShow; UINT nStatus;}; // ret �ɂP�������Ă���΍폜�������s���܂��B

// �O���C���^�[�t�F�C�X
public:
	UltimateGrid_DB_SQLite();
	//void SetUpdateView(db_sqlite* p_DB_SQLite, const wchar_t* szUpdateView);	// �X�V�p�r���[�̐ݒ�
	void SetDB_SQLite(ssc::db_sqlite* p_DB_SQLite);
	void SetAutoUpdate(bool bIsAutoUpdate, const wchar_t* szUpdateView);	// �����A�b�v�f�[�g
	void SetAutoInsert(bool bIsAutoInsert, const wchar_t* szInsertView, int nDefaults=0, ...);	// �����C���T�[�g
	void SetAutoDelete(bool bIsAutoDelete, const wchar_t* szDeleteView);	// �����폜
	bool SetDataset(ssc::db_sqlite_dataset_autoptr pDs);	// SQL�f�[�^�Z�b�g����l��ݒ�
	bool SetDataset_Table(ssc::db_sqlite_dataset_autoptr pDs);
	bool SetDataset_OneRecord(ssc::db_sqlite_dataset_autoptr pDs);

	bool SetSelect(const wchar_t* s);	// SQL�f�[�^�Z�b�g����l��ݒ�
	bool SetSelectReplace(const wchar_t* s);	// �u���������s��SQL�B�u�����������񂪂����ς�����ꍇ�Ɏ����ōX�V����B( ssc::replace_string )
	bool SetSelectf(const wchar_t* fmt, ... ){ return SetSelect(ssc::va_wstrprintf(&fmt).c_str()); }	// SQL�f�[�^�Z�b�g����l��ݒ�
	bool SetSelectWhere(const wchar_t* s);	// SQL�f�[�^�Z�b�g����l��ݒ�
	bool SetSelectWheref(const wchar_t* fmt,...){ return SetSelectWhere(ssc::va_wstrprintf(&fmt).c_str()); };

	void SetEdit(bool bIsEdit){ m_bIsEdit = bIsEdit; }
	void BuildSelectSQL();

	bool ResetSelect();
	void SetHideCol(int hideColNum);	// �擪����̔�\���񐔂ƃf�t�H���g�l��ݒ�
	void SetTitleRow(int titlerow){m_nTitleRow=titlerow;};	// �擪����̃w�b�_�s�����w��B

	void SetColDroplist(int col, ssc::db_sqlite_dataset_autoptr p);
	void SetColBoolIcon(int col, HICON hTrue, HICON hFalse, size_t iconsize);
	void SetColCheckbox(int col, const wchar_t* szTrue, const wchar_t* szFalse);

	bool IsRowData(int keyval, int* pRow=0); // �L�[�l���Ǘ����ɂ��邩�m�F
	void UpdateRowData(int keyval); // �L�[�̒l���w�肵�āA�f�[�^�\���̍X�V
	void UpdateRow(int row, ssc::db_sqlite_dataset_autoptr& pDs);
	void SetCellProperty(int row);

	// �`��̊J�n��~
	void StartDraw();
	void StopDraw();

	// �s�P�ʂ̂Q�g���J���[�̐ݒ�
	void SetRow2Color(COLORREF col1, COLORREF col2);
	void EnableRow2Color(BOOL bEnable);

	__int64 GetSelectRowKeyval(); // ���ݑI�𒆂̍s�̃L�[ID���擾����B
	std::vector<__int64> GetSelectRangeKeyval(); // ���ݑI�𒆂̍s�̃L�[ID���擾����B
	__int64 GetKeyval(int row); // �s�̃L�[ID���擾����

	void SetHedderFont(CFont* pFont);

	void SetTree(bool isTree, __int64 nTreeKeyCol, __int64 nParentKeyCol); // �c���[�\������H����ꍇ�͐e�L�[�̗�Ǝq�L�[�̗���w��
	void Tree_CollapseAll();

	bool IsVisible();
	void SetColTitleInvisible(int col);

	void FixColumn();
	void CollectColInt64(const wchar_t* sz);
	__int64 GetCollectColInt64(const wchar_t* sz, int row);

	bool IsSetDataset(){ return m_bIsSetDataset; } // �f�[�^�Z�b�g�ݒ蒆�H

	void SetAutoFixColumn(bool bAutoFixColumn){ m_bIsAutoFixColumn=bAutoFixColumn; }

	int GetDataCol(int col);

	// �P���R�[�h�\���O���b�h�p����
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
	void SetOneRecordGridKey(const wchar_t* keycol, const wchar_t* key); // �P���R�[�h�̐ݒ�ΏۃL�[

	void SetReplace(ssc::replace_string* pReplaceString);

	bool IsMultiSelected();

public:
	void SetCurrentRow(int nKeyval); // �L�[���w�肵�čs�̈ړ�

// �����f���Q�[�g�Ăяo��
private:
	void OnDClicked(_OnDClicked_arg* arg);	// �X�V�J�n�i�_�u���N���b�N�j
	//void OnCharDown(_OnCharDown_arg* arg);	// �X�V�J�n�i�L�[�{�[�h���́j
	void OnKeyDown(_OnKeyDown_arg* arg);		// �X�V�J�n�i�L�[�{�[�h���́j
	void OnKeyUp(_OnKeyUp_arg* arg);			// �X�V�J�n�i�L�[�{�[�h���́j
	void OnRowChange(_OnRowChange_arg* arg);	// �s�ύX
	void OnPreEditFinish(_OnPreEditFinish_arg* arg);	// �X�V�I���i�e�L�X�g�ҏW�j
	void OnEditFinish(_OnEditFinish_arg* arg);	// �X�V�I���i�e�L�X�g�ҏW�j
	void OnEditStart(_OnEditStart_arg* arg);	// �X�V�I���i�e�L�X�g�ҏW�j
	void OnEditContinue(_OnEditContinue_arg* arg);	// �X�V�I���i�e�L�X�g�ҏW�j
	void OnCellTypeNotify(_OnCellTypeNotify_arg* arg);	// �e�Z���^�C�v�ɂ����̓C�x���g�Ή�
	void OnCanMove(_OnCanMove_arg* arg); // �Z���ړ��O�m�F
	void OnTH_LClicked(_OnTH_LClicked_arg* arg); // 

	BOOL OnDataChange(const wchar_t* newval, int row, int col);

	void OnDelete(ssc::db_sqlite::_on_delete_arg* arg);
	void OnUpdate(ssc::db_sqlite::_on_update_arg* arg);

	//
	int OnRowChangeImplement(int oldrow, int newrow); // ���ʂ̍s�ύX����
	void OnEditStartImplement(int row);	// ���ʂ̃e�L�X�g�ҏW�J�n
	bool IsInsertEditing(); // �}�����͒����H
	void CancelInsert(); // �}���L�����Z������

	// 
	void on_replace_print_set(_on_replace_print_set_arg* arg);

	void OnAddDelayCall();
public:
	// CUGCtrl ���z�֐�
	virtual void OnSetup();

// ����
private:
	int Paste();
	int Paste(CString &string);
	int PasteOne(int col, int row, CString string);

	// �c���[�̐[���𒲍�
	int TreeDepth(int nBegin, int nTreeKey, int nDepth);


	//{{AFX_DATA(UltimateGrid_DB_SQLite)
	//}}AFX_DATA

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(UltimateGrid_DB_SQLite)
	public:
	protected:
	//}}AFX_VIRTUAL

public:
	// �������ꂽ���b�Z�[�W �}�b�v�֐�
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

// �����o�ϐ�
protected:
	ssc::event_delay _delayResetSelect;
	void _DelayResetSelect(){ ResetSelect(); }

	ssc::event_delay _delayUpdateRowData;
	void _DelayUpdateRowData(void* p){ UpdateRowData((int)p); }

	ssc::event_delay _delayBuildSelectSQL;
	void _DelayBuildSelectSQL(){ BuildSelectSQL(); }

protected:
	// �c���[���p�̃Z���^�C�v
	CUGExpandType		m_expand;
	int					m_nExpandIndex;

	// �c���[�֘A�̕ێ��l
	bool m_bIsTree;
	int m_nParentKeyCol;
	int m_nTreeKeyCol;
	std::vector<__int64> m_vecTreeParentKey;	// �e�L�[�̗�
	std::vector<__int64> m_vecTreeKey;		// ����ڃL�[�̗�
	std::vector<__int64> m_vecTreeDepth;		// �[��
	int m_nTreeDepthMax;

	std::vector<__int64> m_vecKeys; // 1��ڂ̃L�[
	std::vector<__int64> m_vecKeysTitle; // 1��ڂ̃L�[
	int m_nColOffset; // �\�����J�n�����̃I�t�Z�b�g
	std::vector<std::wstring> m_vecColDefault; // �f�t�H���g�l
	std::vector<std::wstring> m_vecColType; // ��̃^�C�v


	std::wstring	m_strSelectSQLNotReplace;	// �\���Ώۂ̃r���[(Replace)
	std::set<std::wstring> m_setReplace;		// �u�����������񃊃X�g

	std::wstring	m_strSelectSQLBase;			// �\���Ώۂ̃r���[
	std::wstring	m_strWhere;					// ��������
	std::wstring	m_strSelectSQL;				// �\���Ώۂ̃r���[

	std::wstring	m_strUpdateView; // �X�V�Ώۂ̃r���[
	std::wstring	m_strInsertView; // �X�V�Ώۂ̃r���[
	std::wstring	m_strDeleteView; // �폜�Ώۂ̃r���[
	std::wstring	m_strKeyCol; // �X�V�Ώۂ̃r���[
	ssc::db_sqlite*		m_pDB_SQLite; // �X�V�Ώۂ̃f�[�^�x�[�X

	//std::map<int, db_sqlite_dataset_autoptr > m_mapColDroplistDataset; // ��̃h���b�v�_�E�����X�g
	std::map<int, std::wstring > m_mapColDroplist; // ��̃h���b�v�_�E�����X�g
	std::map<int, HICON > m_mapTrueIconList; // ���bool�A�C�R�����X�g
	std::map<int, HICON > m_mapFalseIconList; // ���bool�A�C�R�����X�g
	size_t						 m_nIconsize;		// �A�C�R���T�C�Y

	// �X�V�pSQL
	std::vector<std::wstring> m_vecColnames; // 1��ڂ̃L�[
	std::vector<std::wstring> m_vecOffsetColnames; // 1��ڂ̃L�[

	// �O���b�h��
	enum GridState {
		GS_NOT_INIT,
		GS_INITING,
		GS_NORMAL,
	};
	GridState m_eGridState;

	// �ݒ�
	bool m_bIsAutoUpdate;
	bool m_bIsAutoInsert;
	bool m_bIsAutoDelete;
	bool m_bIsEdit;

	// �}�����
	// �@�ύX�J�n�i�V�K�sID�̊m��A�ێ��j�V�K�ǉ��p�s�̕\��
	// �A�ύX���iupdate�������Ȃ��j
	// �B�ύX�m��iInsert�@�s�ړ� ���s���Ȃ���Ίm�肵�đ}�����܂��B�j
	// �C���s�Ȃ�s�ړ������Ȃ��B�I���̍D���ȓ��삶��Ȃ����ǁc
	enum EditState {
		ES_NONE,
		ES_EDITING,
		ES_INSERT_EDITING_EDITCONTINUE,
		ES_INSERT_EDITING_NOCHANGE,
		ES_INSERT_EDITING_CHANGED,
	};
	EditState m_eEditState;
	int m_nInsertRow;		// �}���s
	int m_nInsertEditRow;	// �}���s�̕ҏW���s

	int m_nStopDrawCnt; // �`���~��
	int m_nTitleRow; // �^�C�g���s
	int m_nDBDataCount;

	BOOL m_bEnableRow2Color;
	COLORREF m_2Color1;
	COLORREF m_2Color2;

	CFont* m_pHedderFont;  // �w�b�_�p�̃t�H���g

	struct CheckboxCol {
		int nCol;
		std::wstring strTrue;
		std::wstring strFalse;
	};
	std::map<int, CheckboxCol> m_mapCheckboxCol;

	bool m_bResetSelectCalled; // ���Z�b�gSelect �֐����Ăяo����āA��������Ȃ��������Ƃ�����H
	bool m_bShowWindowProcess; // �\���������H
	bool m_bNeedReplace; // ������u���K�v�H

	std::set<int> m_setTitleInvisibleCol;
	
	std::map<std::wstring, std::vector<__int64> > m_mapvecCollectColInt64;	// int�L�[�l�Ƃ��Ď��W�����l�B
	std::set<std::wstring> m_setCollectCol;

	bool m_bIsSetDataset; // �f�[�^�Z�b�g�ݒ蒆�H
	bool m_bIsAutoFixColumn; // �����I�ɗ��Fix����B

	std::vector<std::wstring> m_updateevent;
	std::vector<std::wstring> m_insertevent;

	// ���H�f�[�^
	struct Datatype {
		bool isDatasetColumn;
		std::wstring value;

		Datatype(){ isDatasetColumn=false; }
		void create(const std::wstring& s)
		{
			// '%'���擪�Ȃ�A�f�[�^�x�[�X�̗�f�[�^��\���ΏۂƂ���B
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
			// '|'�����ɂ��ă��x���e�L�X�g�𕪂���B
			if(v.text != 0) {
				text.create(v.text);
			}
			if(v.label != 0) {
				label.create(v.label);
			}

			return *this;
		}
	};
	// SetGridData �����p�f�[�^
	const GridData* m_pGridData; // ���f�[�^
	std::vector< std::vector< CellData > > m_vecCellData; // ���H�f�[�^�ێ�
	bool m_bIsOneRecordGrid; // 1���R�[�h�p�O���b�h�H

	ssc::replace_string* m_pReplaceString; // ���[�J���̒u���p�N���X
	std::wstring m_strOneRecordGridKeycol; // �L�[
	std::wstring m_strOneRecordGridKeyval; // �L�[
	__int64 m_nOneRecordGridKeyval; // �L�[

	UltimateGrid_CellType_Checkbox m_RadioCheckCell; // �P�Z���P���W�I�̃Z���`��
	int m_RadioCheckCellIndex; // �P�Z���P���W�I�̃Z���`��

	delay_call m_DelayCall; // �x���Ăяo������
};

} // namespace ssc {
#endif // #ifndef UltimateGrid_DB_SQLite_hpp

