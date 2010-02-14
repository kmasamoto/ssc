#ifndef UltimateGridUtil_hpp
#define UltimateGridUtil_hpp

#include "UGCtrl.h"

struct Cell {
	bool bNumber;
	double dblNumber;
	UG_ALIGN align;
	HICON hIcon;
	int nIconSize;
	const wchar_t* text;
	int celltype;
	int celltypeex;
	bool isdate;
	CPen* borderPen;
	int border;
};

#define CELL_ICON(ico,size)		{false,0,(UG_ALIGN)(UG_ALIGNVCENTER | UG_ALIGNLEFT),  ico,size,	0,		0,	0,							false, &m_BluePen, 12}
#define CELL_TEXT(text)			{false,0,(UG_ALIGN)(UG_ALIGNVCENTER | UG_ALIGNLEFT),  0,0,		text,	0,	0,							false, &m_BluePen, 12}
#define CELL_DATE(text, idx)	{false,0,(UG_ALIGN)(UG_ALIGNVCENTER | UG_ALIGNLEFT),  0,0,		text,	idx,UGCT_DROPLISTHIDEBUTTON,	true, &m_BluePen, 12}
#define CELL_NUM(num)			{true,num,(UG_ALIGN)(UG_ALIGNVCENTER | UG_ALIGNLEFT), 0,0,		0,		0,	0,							false, &m_BluePen, 12}
#define CELL_DROP(text)			{false,0,(UG_ALIGN)(UG_ALIGNVCENTER | UG_ALIGNLEFT), 0,0,		text,	UGCT_DROPLIST,UGCT_DROPLISTHIDEBUTTON,false, &m_BluePen, 12}

inline void SetRowCell(CUGCtrl* pGrid, int row, Cell cellTbl[], size_t cellTblSize)
{
	for(int i=0; i<cellTblSize; i++) {
		CUGCell		cell;
		pGrid->GetCell(i,row,&cell);
		if(cellTbl[i].bNumber != 0) cell.SetNumber(cellTbl[i].dblNumber);
		if(cellTbl[i].align != 0) cell.SetAlignment(cellTbl[i].align);
		if(cellTbl[i].hIcon != 0) cell.SetIcon(cellTbl[i].hIcon, cellTbl[i].nIconSize);
		if(cellTbl[i].text != 0) cell.SetText(cellTbl[i].text);
		if(cellTbl[i].celltype != 0) cell.SetCellType(cellTbl[i].celltype);
		if(cellTbl[i].celltypeex != 0) cell.SetCellTypeEx(cellTbl[i].celltypeex);
		if(cellTbl[i].isdate) {
			cell.SetLabelText(L"CELLTYPE_IS_EDITABLE");
		}
		if( cellTbl[i].borderPen )	cell.SetBorderColor(cellTbl[i].borderPen);
		if( cellTbl[i].border )	cell.SetBorder(cellTbl[i].border);
		pGrid->SetCell(i,row,&cell);
	}
}


#endif // #ifndef UltimateGridUtil_hpp
