#ifndef UltimateGrid_SSCEx_hpp
#define UltimateGrid_SSCEx_hpp

#include "UGCtrl.h"
#include "ssc/Event.h"

namespace ssc {

#define USE_OVERWRITE_TO_EVENT(CLASS) typedef CLASS USE_OVERWRITE_TO_EVENT_THIS_CLASS;

#define OVERRIDE_TO_EVENT_A2(RET, FUNC, A1T,A1V, A2T,A2V)									\
	ssc::event _##FUNC;																		\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; A1T A1V; A2T A2V; };	\
	virtual RET FUNC(A1T A1V, A2T A2V){ _##FUNC##_arg a={this, A1V, A2V}; _##FUNC(&a); }

// 戻り値あり　引数５個
#define OVERRIDE_TO_EVENT_RA5(RET, RETDEFAULT, FUNC, A1T,A1V, A2T,A2V, A3T,A3V, A4T,A4V, A5T,A5V)	\
	ssc::event _##FUNC;																				\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; RET ret; A1T A1V; A2T A2V; A3T A3V; A4T A4V; A5T A5V;};	\
	virtual RET FUNC(A1T A1V, A2T A2V, A3T A3V, A4T A4V, A5T A5V){ _##FUNC##_arg a={this, RETDEFAULT, A1V, A2V, A3V, A4V, A5V}; _##FUNC(&a); return a.ret; }

// 戻り値あり　引数４個
#define OVERRIDE_TO_EVENT_RA4(RET, RETDEFAULT, FUNC, A1T,A1V, A2T,A2V, A3T,A3V, A4T,A4V)	\
	ssc::event _##FUNC;																				\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; RET ret; A1T A1V; A2T A2V; A3T A3V; A4T A4V; };	\
	virtual RET FUNC(A1T A1V, A2T A2V, A3T A3V, A4T A4V){ _##FUNC##_arg a={this, RETDEFAULT, A1V, A2V, A3V, A4V}; _##FUNC(&a); return a.ret; }

// 戻り値あり　引数３個
#define OVERRIDE_TO_EVENT_RA3(RET, RETDEFAULT, FUNC, A1T,A1V, A2T,A2V, A3T,A3V)	\
	ssc::event _##FUNC;																				\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; RET ret; A1T A1V; A2T A2V; A3T A3V; };	\
	virtual RET FUNC(A1T A1V, A2T A2V, A3T A3V){ _##FUNC##_arg a={this, RETDEFAULT, A1V, A2V, A3V}; _##FUNC(&a); return a.ret; }

// 戻り値なし　引数５個
#define OVERRIDE_TO_EVENT_A5(RET, FUNC, A1T,A1V, A2T,A2V, A3T,A3V, A4T,A4V, A5T,A5V)	\
	ssc::event _##FUNC;																				\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; A1T A1V; A2T A2V; A3T A3V; A4T A4V; A5T A5V;};	\
	virtual RET FUNC(A1T A1V, A2T A2V, A3T A3V, A4T A4V, A5T A5V){ _##FUNC##_arg a={this, A1V, A2V, A3V, A4V, A5V}; _##FUNC(&a); }

// 戻り値なし　引数4個
#define OVERRIDE_TO_EVENT_A4(RET, FUNC, A1T,A1V, A2T,A2V, A3T,A3V, A4T,A4V)	\
	ssc::event _##FUNC;																				\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; A1T A1V; A2T A2V; A3T A3V; A4T A4V; };	\
	virtual RET FUNC(A1T A1V, A2T A2V, A3T A3V, A4T A4V){ _##FUNC##_arg a={this, A1V, A2V, A3V, A4V}; _##FUNC(&a); }

// 戻り値なし　引数６個
#define OVERRIDE_TO_EVENT_A6(RET, FUNC, A1T,A1V, A2T,A2V, A3T,A3V, A4T,A4V, A5T,A5V, A6T,A6V)	\
	ssc::event _##FUNC;																				\
	struct _##FUNC##_arg { USE_OVERWRITE_TO_EVENT_THIS_CLASS* pThis; A1T A1V; A2T A2V; A3T A3V; A4T A4V; A5T A5V; A6T A6V;};	\
	virtual RET FUNC(A1T A1V, A2T A2V, A3T A3V, A4T A4V, A5T A5V, A6T A6V){ _##FUNC##_arg a={this, A1V, A2V, A3V, A4V, A5V, A6V}; _##FUNC(&a); }

class UltimateGrid_SSCEx : public CUGCtrl
{
	USE_OVERWRITE_TO_EVENT(UltimateGrid_SSCEx);
public:
	// オーバーライド継承で実装する関数をイベントにディスパッチ
	OVERRIDE_TO_EVENT_A2(void, OnCharDown, UINT*, vcKey, BOOL, processed)	// _OnCharDown
	OVERRIDE_TO_EVENT_A2(void, OnKeyUp, UINT*, vcKey, BOOL, processed)		// _OnKeyUp
	OVERRIDE_TO_EVENT_A2(void, OnKeyDown, UINT*, vcKey, BOOL, processed)	// _OnKeyDown
	OVERRIDE_TO_EVENT_RA4(int, 1, OnEditContinue, int,oldcol,long,oldrow,int*,newcol,long*,newrow) // _OnEditContinue
	OVERRIDE_TO_EVENT_RA3(int, 1, OnEditStart, int,col, long,row, CWnd**,edit) // _OnEditStart
	OVERRIDE_TO_EVENT_RA5(int, 1, OnCellTypeNotify, long,ID, int,col, long,row, long,msg, long,param) // _OnCellTypeNotify
	OVERRIDE_TO_EVENT_A5(void, OnDClicked, int,col, long,row, RECT*,rect, POINT*,point, BOOL,processed) // _OnDClicked
	OVERRIDE_TO_EVENT_A2(void, OnColChange, int,oldcol, int,newcol)	// _OnCharDown
	OVERRIDE_TO_EVENT_A2(void, OnRowChange, long,oldrow, long,newrow)	// _OnRowChange
	OVERRIDE_TO_EVENT_RA4(int, 1, OnCanMove, int,oldcol, long,oldrow, int,newcol, long,newrow) // _OnCanMove
	OVERRIDE_TO_EVENT_RA3(int, TRUE, OnMenuStart, int,col, long,row, int,section) // _OnMenuStart
	OVERRIDE_TO_EVENT_A4(void, OnMenuCommand, int,col, long,row, int,section, int,item) // _OnMenuStart
	OVERRIDE_TO_EVENT_A5(void, OnSelectionChanged, int,startCol,long,startRow,int,endCol,long,endRow,int,blockNum) // _OnSelectionChanged
	OVERRIDE_TO_EVENT_A6(void, OnTH_LClicked, int,col,long,row,int,updn,RECT*,rect,POINT*,point,BOOL,processed) // _OnTH_LClicked
	OVERRIDE_TO_EVENT_A6(void, OnTH_RClicked, int,col,long,row,int,updn,RECT*,rect,POINT*,point,BOOL,processed) // _OnTH_LClicked
	OVERRIDE_TO_EVENT_A5(void, OnTH_DClicked, int,col,long,row,RECT*,rect,POINT*,point,BOOL,processed) // _OnTH_DClicked
	OVERRIDE_TO_EVENT_A6(void, OnSH_LClicked, int,col,long,row,int,updn,RECT*,rect,POINT*,point,BOOL,processed) // _OnTH_LClicked
	OVERRIDE_TO_EVENT_A6(void, OnSH_RClicked, int,col,long,row,int,updn,RECT*,rect,POINT*,point,BOOL,processed) // _OnTH_LClicked
	OVERRIDE_TO_EVENT_A5(void, OnSH_DClicked, int,col,long,row,RECT*,rect,POINT*,point,BOOL,processed) // _OnTH_DClicked

	//OVERRIDE_TO_EVENT_RA5(1, int, OnEditFinish, int, col, long, row, CWnd*, edit, CString*, string, BOOL, cancelFlag) //  _OnEditFinish
	///*
	// OnEditFinish
	// テキストの変更
	EVENT_RA6(int,1, _OnEditFinish, UltimateGrid_SSCEx*,pThis, int,col, long,row, CWnd*,edit, CString*,string, BOOL,cancelFlag);
	_OnEditFinish_t _OnEditFinish;

	EVENT_RA6(int,1, _OnPreEditFinish, UltimateGrid_SSCEx*, pThis, int,col, long,row, CWnd*,edit, CString*,string, BOOL,cancelFlag);
	_OnPreEditFinish_t _OnPreEditFinish;

	int OnEditFinish(int col, long row,CWnd* edit,CString* string,BOOL cancelFlag)
	{
		int ret = _OnPreEditFinish(this, col, row, edit, string, cancelFlag);
		if( ret != 1 ) {
			return ret;
		}
		return _OnEditFinish(this, col, row, edit, string, cancelFlag);
	}
	//*/
};

} // namespace ssc 
#endif // #ifndef UltimateGrid_SSCEx_hpp
