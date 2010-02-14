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

// nIDR [IN] ���\�[�XID
// pTextSize [OUT] �e�L�X�g�̃T�C�Y��Ԃ�
// type [IN] ���\�[�X�^�C�v
inline std::wstring LoadTextResource(const wchar_t* resid, const TCHAR* type=_T("Text"))
{
	/* ���\�[�X����f�[�^��ǂݍ��� */
	HRSRC rHandle;
	rHandle = FindResourceW(NULL, resid, type );
	HANDLE hTest;
	hTest = LoadResource(NULL, rHandle);
	/* �ǂݍ��񂾃f�[�^�̃|�C���^�擾 */
	LPBYTE lpTest;
	lpTest = (LPBYTE)LockResource(hTest);

	int nSize = SizeofResource(NULL, rHandle);

	return ssc::wstrprintf(L"%S", lpTest);
}

#endif
