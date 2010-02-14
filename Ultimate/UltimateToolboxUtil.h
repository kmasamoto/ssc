#ifndef UltimateToolboxUtil_hpp
#define UltimateToolboxUtil_hpp

#include "OXTreeCtrl.h"
#include "../mfcutil.h"
#include <stack>

// ツリー構造
struct TreeConstructItem{
	int nDeps;			// アイテム深度
	wchar_t* szTitle;		// 文字列
	int nIcon;			// アイコンID -1 の場合は指定なし。
	int nSelectIcon;	// 選択時のアイコンID -1 の場合は nIcon の値を使用
	CFont* pFont;		// フォント 0なら指定されない。
	DWORD dwItemData;	// アイテムデータ
	HTREEITEM hItem;
};

class TreeConstructer
{
// オペレーション
public:
	// 渡された構築用データから、ツリーを構築する。
	void Construct(COXTreeCtrl& tree, TreeConstructItem* treeStruct, size_t treeStructSize, int iconSize)
	{
		m_ImageList.Create(iconSize, iconSize, ILC_COLOR32  | ILC_MASK, 8, 1);
		// まずイメージリストにアイコン作成
		for(int i=0; i<treeStructSize; i++) {
			// まだロードされていない？
			if( treeStruct[i].nIcon != -1 && m_mapIndex.find(treeStruct[i].nIcon) == m_mapIndex.end()) {
				// ロード実行
				m_mapIndex[treeStruct[i].nIcon] = AddIcon(&m_ImageList, treeStruct[i].nIcon, iconSize);
			}
			// まだロードされていない？
			if( treeStruct[i].nSelectIcon != -1 && m_mapIndex.find(treeStruct[i].nSelectIcon) == m_mapIndex.end()) {
				// ロード実行
				m_mapIndex[treeStruct[i].nSelectIcon] = AddIcon(&m_ImageList, treeStruct[i].nSelectIcon, iconSize);
			}
		}
		tree.SetImageList(&m_ImageList, TVSIL_NORMAL);

		// ツリーを生成
		std::stack<HTREEITEM> que_hItemParents;

		for(int i=0; i<treeStructSize; i++) {
			// 深さを設定
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
			// 親を保持
			que_hItemParents.push(hItem);

			// アイコンを設定
			if( treeStruct[i].nIcon != -1 ) {
				// 選択時のアイコンデフォルト設定
				int nSel = (treeStruct[i].nSelectIcon == -1 ? treeStruct[i].nIcon : treeStruct[i].nSelectIcon);
				tree.SetItemImage(hItem, m_mapIndex[treeStruct[i].nIcon], m_mapIndex[nSel]);
			}

			// フォントを設定
			if( treeStruct[i].pFont != 0 ) {
				tree.SetItemFont(hItem, treeStruct[i].pFont, 0);
			}

			// データを設定
			tree.SetItemData(hItem, treeStruct[i].dwItemData);

			// ツリーアイテムハンドルを設定
			treeStruct[i].hItem = hItem;
		}
	}
	
// 内部変数
private:
	CImageList m_ImageList; // ツリーのイメージリスト
	std::map<int, int> m_mapIndex; // インデックスとインデックスの対応を保持
};


#endif // #ifndef UltimateToolboxUtil_hpp
