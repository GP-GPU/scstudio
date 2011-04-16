/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2007-2010 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: finddlg.cpp 1038 2011-02-09 22:26:51Z mbezdeka $
 */

#include "stdafx.h"
#include "extract.h"
#include "finddlg.h"

CFindDlg::CFindDlg(Visio::IVApplicationPtr vsoApp)
 : ATL::CDialogImpl<CFindDlg>(), CWinDataExchange<CFindDlg>()
{
  m_vsoApp = vsoApp;
}

LRESULT CFindDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

  DoDataExchange();
  InitializeTree(m_drawing1, true); // pattern
  InitializeTree(m_drawing2, false);

  return bHandled = FALSE;
}

LRESULT CFindDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  if(wID == IDOK)
  {
    m_diffEnabled = (m_diff.GetCheck() == BST_CHECKED);
    m_pages1 = GetTreeSelection(m_drawing1);    
    std::vector<Visio::IVPagePtr>  m_pages2 = GetTreeSelection(m_drawing2);
    if(m_pages2.size() != 1)
      m_page2 == NULL;
    else
      m_page2 = m_pages2.at(0);

    if (m_pages1.empty() || m_page2 == NULL)
    {
      MessageBox(L"You must select at least one flow!");
      return 0;    
    }
  }

  EndDialog(wID);
  return 0;
}

void CFindDlg::InitializeTree(CTreeViewCtrl& tree, bool isPattern)
{
  for(int i = 1; i <= m_vsoApp->Windows->Count; i++)
  {
    Visio::IVWindowPtr vsoWindow = m_vsoApp->Windows->Item[i];
    Visio::IVDocumentPtr vsoDocument = vsoWindow->Document;

    HTREEITEM docitem = tree.InsertItem(vsoWindow->Caption, 0, 0);
    tree.SetItemData(docitem, vsoDocument->ID);
    for(int j = 1; j <= vsoDocument->Pages->Count; j++)
    {
      Visio::IVPagePtr vsoPage = vsoDocument->Pages->Item[j];

      TDrawingType dtype = get_drawing_type(vsoPage);
      // check the drawing type
      if(isPattern && dtype == DT_BMSC ||
        !isPattern && (dtype == DT_BMSC || dtype == DT_HMSC))
      {
        HTREEITEM pageitem = tree.InsertItem(vsoPage->Name, docitem, 0);
        tree.SetItemData(pageitem, vsoPage->ID);
      }
    }

    tree.Expand(docitem);
  }
}

std::vector<Visio::IVPagePtr> CFindDlg::GetTreeSelection(CTreeViewCtrl& tree)
{
  std::vector<Visio::IVPagePtr> vsoPages;
  HTREEITEM parent = tree.GetRootItem();
  HTREEITEM child = NULL;

  //Observe whole tree and get checked !child! items
  while(parent)
  {
    child = tree.GetChildItem(parent);
    //Observe all children
    while(child)
    {
      if(tree.GetCheckState(child))
      {
        Visio::IVDocumentPtr vsoDocument = m_vsoApp->Documents->ItemFromID[tree.GetItemData(parent)];
        vsoPages.push_back(vsoDocument->Pages->ItemFromID[tree.GetItemData(child)]);
      }
      //Get sibling child
      child = tree.GetNextSiblingItem(child);
    }
    //Get sibling parent
    parent = tree.GetNextSiblingItem(parent);
  }
  return vsoPages;
}

LRESULT CFindDlg::OnNMClickDrawing1(int, LPNMHDR pNMHDR, BOOL&)
{
  return OnTreeViewClicked(m_drawing1);
}

LRESULT CFindDlg::OnNMClickDrawing2(int, LPNMHDR pNMHDR, BOOL&)
{
  return OnTreeViewClicked(m_drawing2);
}

LRESULT CFindDlg::OnTreeViewClicked(CTreeViewCtrl& tree)
{
  //Get cursor position
  POINT pt;
  GetCursorPos(&pt);
  tree.ScreenToClient(&pt);
  //Check where is the cursor position to drawing
  UINT flags = 0;
  HTREEITEM hItem = tree.HitTest(pt, &flags);
  if(!hItem)
    return 0;

  return OnTreeViewItemClicked(tree, hItem, flags);
}

LRESULT CFindDlg::OnTreeViewItemClicked(CTreeViewCtrl& tree, HTREEITEM item, UINT unFlags)
{
  bool multiSelect = (tree == m_drawing1);

  if(unFlags & TVHT_ONITEMSTATEICON) // cursor is on the check-box
    return processCheckBoxes(tree, item, multiSelect, true);
  else if(unFlags & TVHT_ONITEMLABEL) // cursor is on the label item
    return processCheckBoxes(tree, item, multiSelect);

  return 0;
}

void CFindDlg::setCheckBoxes(CTreeViewCtrl& tree, BOOL bCheck)
{
  HTREEITEM parent = tree.GetRootItem();
  HTREEITEM child = NULL;

  //Observe whole tree and set all nodes check-boxes to bCheck
  while(parent)
  {
    tree.SetCheckState(parent, bCheck);
    child = tree.GetChildItem(parent);
    //Observe all children
    while(child)
    {
      tree.SetCheckState(child, bCheck);
      child = tree.GetNextSiblingItem(child); //Get sibling child
    }
    parent = tree.GetNextSiblingItem(parent); //Get sibling parent
  }  
}

LRESULT CFindDlg::processCheckBoxes(CTreeViewCtrl& tree, HTREEITEM item, bool multiSelect, bool reverted)
{
  BOOL newCheckState = !tree.GetCheckState(item);
  //Get parent item
  HTREEITEM parent = tree.GetParentItem(item);

  if(!multiSelect)
  {
    setCheckBoxes(tree, 0); //Reset all check-boxes
    if(reverted) tree.SetCheckState(item, !newCheckState);
  }

  //Check/uncheck current item
  if(!reverted)  //NOTE: If we click on check-box, it's automatically checked/unchecked
    tree.SetCheckState(item, newCheckState);

  if(parent) //'item' is a child item -> Check if there is another checked/uncheckd sibling item
  { 
    HTREEITEM sibling = tree.GetChildItem(parent); //Get first sibling item
    if(newCheckState)
        tree.SetCheckState(parent, 1);

    while(sibling)
    {
      if((tree.GetCheckState(sibling) != newCheckState) && (sibling != item))
        break;
      sibling = tree.GetNextSiblingItem(sibling);
    }
    if(!sibling) // If sibling is NULL, then every sibling items have the same check-box status
        tree.SetCheckState(parent, newCheckState);            
  }
  else //'item' is a parent item -> check or uncheck all child items
  {
    HTREEITEM childItem = tree.GetChildItem(item);

    while(childItem)
    {
      tree.SetCheckState(childItem, newCheckState);
      childItem = tree.GetNextSiblingItem(childItem);
      //If multiselect is off and parent item has more 
      //than one child we don't know which child should be checked -> check only first childItem
      if(childItem && !multiSelect && newCheckState)
        break; //Jump out from cycle
    }
  }

  return 0;
}

// $Id: finddlg.cpp 1038 2011-02-09 22:26:51Z mbezdeka $
