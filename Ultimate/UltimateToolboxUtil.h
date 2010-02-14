#ifndef UltimateToolboxUtil_hpp
#define UltimateToolboxUtil_hpp

#include "OXTreeCtrl.h"
#include "../mfcutil.h"
#include <stack>

// �c���[�\��
struct TreeConstructItem{
	int nDeps;			// �A�C�e���[�x
	wchar_t* szTitle;		// ������
	int nIcon;			// �A�C�R��ID -1 �̏ꍇ�͎w��Ȃ��B
	int nSelectIcon;	// �I�����̃A�C�R��ID -1 �̏ꍇ�� nIcon �̒l���g�p
	CFont* pFont;		// �t�H���g 0�Ȃ�w�肳��Ȃ��B
	DWORD dwItemData;	// �A�C�e���f�[�^
	HTREEITEM hItem;
};

class TreeConstructer
{
// �I�y���[�V����
public:
	// �n���ꂽ�\�z�p�f�[�^����A�c���[���\�z����B
	void Construct(COXTreeCtrl& tree, TreeConstructItem* treeStruct, size_t treeStructSize, int iconSize)
	{
		m_ImageList.Create(iconSize, iconSize, ILC_COLOR32  | ILC_MASK, 8, 1);
		// �܂��C���[�W���X�g�ɃA�C�R���쐬
		for(int i=0; i<treeStructSize; i++) {
			// �܂����[�h����Ă��Ȃ��H
			if( treeStruct[i].nIcon != -1 && m_mapIndex.find(treeStruct[i].nIcon) == m_mapIndex.end()) {
				// ���[�h���s
				m_mapIndex[treeStruct[i].nIcon] = AddIcon(&m_ImageList, treeStruct[i].nIcon, iconSize);
			}
			// �܂����[�h����Ă��Ȃ��H
			if( treeStruct[i].nSelectIcon != -1 && m_mapIndex.find(treeStruct[i].nSelectIcon) == m_mapIndex.end()) {
				// ���[�h���s
				m_mapIndex[treeStruct[i].nSelectIcon] = AddIcon(&m_ImageList, treeStruct[i].nSelectIcon, iconSize);
			}
		}
		tree.SetImageList(&m_ImageList, TVSIL_NORMAL);

		// �c���[�𐶐�
		std::stack<HTREEITEM> que_hItemParents;

		for(int i=0; i<treeStructSize; i++) {
			// �[����ݒ�
			while(que_hItemParents.size() > treeStruct[i].nDeps) {
				que_hItemParents.pop();
			}

			HTREEITEM hItem;
			if( treeStruct[i].nDeps == 0) {
				hItem = tree.InsertItem(treeStruct[i].szTitle);
			}
			else {
				hItem = tree.InsertItem(treeStruct[i].szTitle, que_hItemParents.top());
			}
			// �e��ێ�
			que_hItemParents.push(hItem);

			// �A�C�R����ݒ�
			if( treeStruct[i].nIcon != -1 ) {
				// �I�����̃A�C�R���f�t�H���g�ݒ�
				int nSel = (treeStruct[i].nSelectIcon == -1 ? treeStruct[i].nIcon : treeStruct[i].nSelectIcon);
				tree.SetItemImage(hItem, m_mapIndex[treeStruct[i].nIcon], m_mapIndex[nSel]);
			}

			// �t�H���g��ݒ�
			if( treeStruct[i].pFont != 0 ) {
				tree.SetItemFont(hItem, treeStruct[i].pFont, 0);
			}

			// �f�[�^��ݒ�
			tree.SetItemData(hItem, treeStruct[i].dwItemData);

			// �c���[�A�C�e���n���h����ݒ�
			treeStruct[i].hItem = hItem;
		}
	}
	
// �����ϐ�
private:
	CImageList m_ImageList; // �c���[�̃C���[�W���X�g
	std::map<int, int> m_mapIndex; // �C���f�b�N�X�ƃC���f�b�N�X�̑Ή���ێ�
};


#endif // #ifndef UltimateToolboxUtil_hpp
