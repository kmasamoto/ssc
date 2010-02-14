#pragma once

namespace ssc {

class UltimateGrid_CellType_Checkbox : public CUGCheckBoxType
{
	
	/***************************************************
	OnDraw  - overloaded CUGCellType::OnDraw
		This function will draw the checkbox according to
		the extended style for the checkbox.

	    **See CUGCellType::OnDraw for more details
		about this function

	Params
		dc		- device context to draw the cell with
		rect	- rectangle to draw the cell in
		col		- column that is being drawn
		row		- row that is being drawn
		cell	- cell that is being drawn
		selected- TRUE if the cell is selected, otherwise FALSE
		current - TRUE if the cell is the current cell, otherwise FALSE
	Return
		<none>
	****************************************************/
	virtual void OnDraw(CDC *dc,RECT *rect,int col,long row,CUGCell *cell,int selected,int current)
	{
		double fHScale = 1.0;
		double fVScale = 1.0;
		
		if (dc->IsPrinting())
		{
			fHScale = m_ctrl->GetUGPrint()->GetPrintHScale(dc);
			fVScale = m_ctrl->GetUGPrint()->GetPrintVScale(dc);
		}

		if (!m_drawThemesSet)
			m_useThemes = cell->UseThemes();

		// draw border of the cell using build-in routine
		DrawBorder( dc, rect, rect, cell );

		int style = 0;
		bool isDisabled = false;
		if( cell->IsPropertySet( UGCELL_CELLTYPEEX_SET ))
		{
			style = cell->GetCellTypeEx();
			isDisabled = (style & UGCT_CHECKBOXDISABLED) > 0;
		}

		UGXPCellType ct = (style & UGCT_CHECKBOXROUND) ? XPCellTypeRadio : XPCellTypeCheck;

		if (cell->GetNumber() > 0)
		{
			if (style&UGCT_CHECKBOXCHECKMARK)
			{
				ct = (style & UGCT_CHECKBOXROUND) ? XPCellTypeRadioYes : XPCellTypeCheckYes;
			}
			else
			{
				ct = (style & UGCT_CHECKBOXROUND) ? XPCellTypeRadioNo : XPCellTypeCheckNo;
			}
		}

		UGXPThemeState ts = UGXPThemes::GetState(selected>0, current>0);

		if (isDisabled) ts = ThemeStateTriState;

		if (cell->GetNumber() > 1)
		{
			ts = ThemeStateTriState;
		}

		int right = rect->right,
			left = rect->left,
			top = 0,
			checkSize = 0;
		CRect checkrect(0,0,0,0);
		CPen * oldpen;

		checkSize = rect->bottom - rect->top - (int)( UGCT_CHECKMARGIN * 2 * fHScale );
		if( checkSize > UGCT_CHECKSIZE * fHScale )
			checkSize = (int)(UGCT_CHECKSIZE * fHScale);
		top = ( rect->bottom - rect->top - checkSize ) / 2;

		checkrect.CopyRect(rect);
		// calculate position and set of the check box
		AdjustRect( &checkrect, cell->GetAlignment(), cell->GetCellTypeEx(), top, checkSize );

		//*** draw the background ***
		if ( selected || ( current && m_ctrl->m_GI->m_currentCellMode & 2 ))
			DrawBackground( dc, rect, cell->GetHBackColor(), row, col, cell, (current != 0), (selected != 0));
		else
			DrawBackground( dc, rect, cell->GetBackColor(), row, col, cell, (current != 0), (selected != 0));

		if (!m_useThemes || !UGXPThemes::DrawBackground(NULL, *dc, ct, ts, &checkrect, NULL, true))
		{	
			//*** draw the checkbox ***
			if( checkSize >= ( UGCT_CHECKMARGIN * 2 ) )
			{
				//draw a 3D Recessed check box
				if( style & UGCT_CHECKBOX3DRECESS )
				{	
					oldpen = (CPen*)dc->SelectObject((CPen*)&m_darkPen);
					dc->MoveTo(checkrect.left,checkrect.bottom);
					dc->LineTo(checkrect.left,checkrect.top);
					dc->LineTo(checkrect.right,checkrect.top);
					dc->SelectObject(&m_lightPen);
					dc->LineTo(checkrect.right,checkrect.bottom);
					dc->LineTo(checkrect.left,checkrect.bottom);
					checkrect.top++;
					checkrect.left++;
					checkrect.right--;
					checkrect.bottom--;
					dc->SelectObject(&m_facePen);
					dc->MoveTo(checkrect.left,checkrect.bottom);
					dc->LineTo(checkrect.right,checkrect.bottom);
					dc->LineTo(checkrect.right,checkrect.top);
					dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(BLACK_PEN))));
					dc->LineTo(checkrect.left,checkrect.top);
					dc->LineTo(checkrect.left,checkrect.bottom);
					dc->SelectObject(oldpen);

					checkrect.top++;
					checkrect.left++;
					if( cell->GetNumber() > 1 )
						FillDitheredRect( dc, checkrect );
				}
				//draw a 3D Raised check box
				else if( style & UGCT_CHECKBOX3DRAISED )
				{	
					oldpen = (CPen*)dc->SelectObject((CPen*)&m_lightPen);
					dc->MoveTo(checkrect.left,checkrect.bottom);
					dc->LineTo(checkrect.left,checkrect.top);
					dc->LineTo(checkrect.right,checkrect.top);
					dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(BLACK_PEN))));
					dc->LineTo(checkrect.right,checkrect.bottom);
					dc->LineTo(checkrect.left,checkrect.bottom);
					checkrect.top++;
					checkrect.left++;
					checkrect.right--;
					checkrect.bottom--;
					dc->SelectObject(&m_darkPen);
					dc->MoveTo(checkrect.left,checkrect.bottom);
					dc->LineTo(checkrect.right,checkrect.bottom);
					dc->LineTo(checkrect.right,checkrect.top);
					dc->SelectObject(&m_facePen);
					dc->LineTo(checkrect.left,checkrect.top);
					dc->LineTo(checkrect.left,checkrect.bottom);
					dc->SelectObject(oldpen);

					checkrect.top++;
					checkrect.left++;
					if( cell->GetNumber() > 1 )
						FillDitheredRect( dc, checkrect );
				}
				// draw a flat (radio like) check box
				else if( style & UGCT_CHECKBOXROUND )
				{
					//draw the circle
					if( selected || ( current && m_ctrl->m_GI->m_currentCellMode&2 ))
						oldpen = (CPen*)dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(WHITE_PEN))));
					else
						oldpen = (CPen*)dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(BLACK_PEN))));

					dc->Arc( checkrect, CPoint(checkrect.left,(checkrect.bottom-checkrect.top)/2), CPoint(checkrect.left,(checkrect.bottom-checkrect.top)/2));
				}
				//draw a plain check box
				else
				{	
					if( selected || ( current && m_ctrl->m_GI->m_currentCellMode&2 ))
						oldpen = (CPen*)dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(WHITE_PEN))));
					else
						oldpen = (CPen*)dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(BLACK_PEN))));

					dc->MoveTo(checkrect.left,checkrect.top);
					dc->LineTo(checkrect.right,checkrect.top);
					dc->LineTo(checkrect.right,checkrect.bottom);
					dc->LineTo(checkrect.left,checkrect.bottom);
					dc->LineTo(checkrect.left,checkrect.top);
					dc->SelectObject(oldpen);
						
					checkrect.left++;
					checkrect.top++;
					if( cell->GetNumber() > 1 )
						FillDitheredRect( dc, checkrect );
				}

				if ( cell->GetReadOnly() == TRUE )
				{	// cell is set to read only
					FillDitheredRect( dc, checkrect );
				}

				//draw the check
				if( cell->GetNumber() > 0 )
				{
					if( cell->GetNumber() > 1 )
						oldpen = (CPen*)dc->SelectObject((CPen*)&m_darkPen);
					else if( selected || ( current && m_ctrl->m_GI->m_currentCellMode&2 ))
						oldpen = (CPen*)dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(WHITE_PEN))));
					else
						oldpen = (CPen*)dc->SelectObject((CPen*)CPen::FromHandle((HPEN)(CPen*)CPen::FromHandle((HPEN)GetStockObject(BLACK_PEN))));

					//draw a check mark
					if(style&UGCT_CHECKBOXCHECKMARK)
					{ 
						dc->MoveTo(checkrect.left+2,checkrect.bottom-4);
						dc->LineTo(checkrect.left+4,checkrect.bottom-2);
						dc->LineTo(checkrect.right+3,checkrect.top-1);
						if(checkSize > 9)
						{
							dc->MoveTo(checkrect.left+2,checkrect.bottom-5);
							dc->LineTo(checkrect.left+4,checkrect.bottom-3);
							dc->LineTo(checkrect.right+3,checkrect.top-2);
							dc->MoveTo(checkrect.left+5,checkrect.bottom-2);
							dc->LineTo(checkrect.right+4,checkrect.top-1);
							dc->MoveTo(checkrect.left+2,checkrect.bottom-6);
							dc->LineTo(checkrect.left+5,checkrect.bottom-3);
						}
						dc->SelectObject(oldpen);
					}
					// draw the radio style fill area
					else if( style & UGCT_CHECKBOXROUND )
					{
						checkrect.left += 2;
						checkrect.top += 2;
						checkrect.right -= 2;
						checkrect.bottom -= 2;

						CBrush brush;

						if( cell->GetNumber() > 1 )
						{
							LOGPEN logPen;
							m_darkPen.GetLogPen( &logPen );
							brush.CreateSolidBrush( logPen.lopnColor );
							dc->SelectObject( brush );
						}
						else if( selected || ( current && m_ctrl->m_GI->m_currentCellMode&2 ))
							dc->SelectObject((CBrush*)CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
						else
							dc->SelectObject((CBrush*)CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));

						dc->Ellipse( checkrect );
					}
					//draw the X mark
					else
					{
						checkrect.left++;
						checkrect.top++;
						checkrect.right-=2;
						checkrect.bottom-=2;
						dc->MoveTo(checkrect.left,checkrect.top);
						dc->LineTo(checkrect.right+1,checkrect.bottom+1);
						dc->MoveTo(checkrect.left,checkrect.bottom);
						dc->LineTo(checkrect.right+1,checkrect.top-1);
						if(checkSize > 9)
						{
							dc->MoveTo(checkrect.left+1,checkrect.top);
							dc->LineTo(checkrect.right+1,checkrect.bottom);
							dc->MoveTo(checkrect.left,checkrect.bottom-1);
							dc->LineTo(checkrect.right,checkrect.top-1);
							dc->MoveTo(checkrect.left,checkrect.top+1);
							dc->LineTo(checkrect.right,checkrect.bottom+1);
							dc->MoveTo(checkrect.left+1,checkrect.bottom);
							dc->LineTo(checkrect.right+1,checkrect.top);
						}
						dc->SelectObject(oldpen);
					}
				}
			}
			
		}

		if (!( style & UGCT_CHECKBOXUSEALIGN ))
		{
			// adjust text rect
			rect->left += (( UGCT_CHECKMARGIN * 2 ) + checkSize );
			rect->right = right;
			m_drawLabelText = TRUE;
			// draw the text using the default drawing routine
			CUGCellType::DrawText(dc,rect,0,col,row,cell,selected,current);
		}	

		// restore original value of the left side
		rect->left = left;
	}

};

}
