#ifndef MFCMacro_h
#define MFCMacro_h

inline int AddIcon(CImageList* pImageList, int nID, int size)
{
	return pImageList->Add( (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(nID), IMAGE_ICON, size,size, LR_SHARED) );
}

inline HICON GetIconRes(int id, int size)
{
	return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, size,size, LR_SHARED);
}

// nIDR [IN] リソースID
// pTextSize [OUT] テキストのサイズを返す
// type [IN] リソースタイプ
inline std::wstring LoadTextResource(const wchar_t* resid, const TCHAR* type=_T("Text"))
{
	/* リソースからデータを読み込む */
	HRSRC rHandle;
	rHandle = FindResourceW(NULL, resid, type );
	HANDLE hTest;
	hTest = LoadResource(NULL, rHandle);
	/* 読み込んだデータのポインタ取得 */
	LPBYTE lpTest;
	lpTest = (LPBYTE)LockResource(hTest);

	int nSize = SizeofResource(NULL, rHandle);

	return ssc::wstrprintf(L"%S", lpTest);
}

#endif
