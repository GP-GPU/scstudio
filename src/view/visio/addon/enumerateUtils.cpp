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
* Copyright (c) 2010 Martin Bezdeka <mbezdeka@seznam.cz>
*
* $Id: enumerateUtils.cpp 862 2010-08-26 22:25:05Z mbezdeka $
*/

#include "enumerateUtils.h"

std::wstring CEnumerateUtils::int2Romans(int index)
{
  if(index < 0 || index > 3999)
  {
    MessageBox(GetActiveWindow(),
      _T("Index too big! Use another type of enumeration. Roman index can be from 0-3999."),
      _T("Error"),MB_OK | MB_ICONERROR);
    return _T("?");
  }

  if(index == 0)
    return _T("N");

  int numeral[] = {1000, 900, 500, 400, 100,90, 50, 40, 10, 9, 5, 4, 1};
  std::wstring romans[] = {L"M", L"CM", L"D", L"CD", L"C", L"XC", L"L", L"XL", L"X", L"IX", L"V", L"IV", L"I" };
  std::wstring result;

  for(int i=0; i < 13; i++)
    while(index >=numeral[i])
    {
      index -= numeral[i];
      result += romans[i];
    }

    return result;
}

std::wstring CEnumerateUtils::int2Char(int index, bool capitals)
{	
  index--;

  if(index < 0)
  {
    MessageBox(GetActiveWindow(),
      _T("Index less than one! Char index can be from 1-inf."),
      _T("Error"),MB_OK | MB_ICONERROR);
    return _T("?");
  }

  std::wstring result;

  if(index == 0)
    return (capitals ? _T("A") : _T("a"));

  bool bLast = true;

  while(index > 0)
  {
    if(bLast)
    {
      result.insert(0,(capitals ? (stringize() << (char)((index % 26)+65)) : (stringize() << (char)((index % 26)+97))));
      index = index / 26;

      if(index/27 == 0)
        bLast = false;
    }
    else
    {
      result.insert(0,(capitals ? (stringize() << (char)((index % 27)+64)) : (stringize() << (char)((index % 27)+96))));
      index = index / 27;
    }
  }

  return result;
}

bool CEnumerateUtils::messageCompare(Visio::IVShapePtr p1, Visio::IVShapePtr p2)
{
  double beginP1Y = p1->Cells["BeginY"]->ResultIU;
  double beginP2Y = p2->Cells["BeginY"]->ResultIU;

  if(beginP1Y - beginP2Y != 0.0)
    return beginP1Y > beginP2Y;
  else
  {
    double beginP1X = p1->Cells["BeginX"]->ResultIU;
    double beginP2X = p2->Cells["BeginX"]->ResultIU;

    if(beginP1X - beginP2X != 0.0)
      return beginP1X < beginP2X;
    else
    {	
      double endP1X = p1->Cells["EndX"]->ResultIU;
      double endP2X = p2->Cells["EndX"]->ResultIU;

      return endP1X < endP2X;
    }
  }
}

bool CEnumerateUtils::loadGroupSettings(Visio::IVApplicationPtr vsoApp, _bstr_t groupID, int& startIndex, int& numberingType, std::wstring& addition)
{
  Visio::IVShapePtr page = vsoApp->ActivePage->PageSheet;
  if(!page->SectionExists[visSectionUser][0])
    return false;

  _bstr_t userGroupID = stringize() << _T("User.") << groupID;
  Visio::IVCellPtr groupCell = page->GetCells(userGroupID);
  if(!groupCell)
    return false;

  std::wstringstream ss(std::wstringstream::in | std::stringstream::out);
  ss << groupCell->ResultStr[visNone];
  ss >> startIndex >> numberingType >> addition;

  //Get spaces in addition back 
  for(size_t i=0; i<addition.size(); i++)
    if(addition[i] == '$')
      addition[i] = ' ';

  addition = addition + _T(" ");

  return true;
}

bool CEnumerateUtils::saveGroupSettings(Visio::IVApplicationPtr vsoApp, _bstr_t groupID, int startIndex, int numberingType, BSTR addition)
{
  Visio::IVShapePtr page = vsoApp->ActivePage->PageSheet;

  if(!page->SectionExists[visSectionUser][0])
    page->AddSection(visSectionUser);

  _bstr_t userGroupID = stringize() << _T("User.") << groupID;

  if(page->GetCellExists(userGroupID,0) != -1)
    page->AddNamedRow(visSectionUser, groupID, visTagDefault);

  Visio::IVCellPtr groupCell = page->GetCells(userGroupID);

  if(!groupCell)
    return false;

  std::wstring additionTemp = addition;

  for(size_t i=0; i<additionTemp.size(); i++)
    if(additionTemp[i] == 32)
      additionTemp[i] = '$';

  groupCell->FormulaU = stringize() << _T("=\"") << startIndex << _T(" ")
    << numberingType << _T(" ")
    << additionTemp << _T("\"");
  return true;
}

void CEnumerateUtils::selectGroup(Visio::IVApplicationPtr vsoApp, _bstr_t groupID)
{
  Visio::IVPagePtr page = vsoApp->ActivePage;
  Visio::IVSelectionPtr selection = page->CreateSelection(Visio::visSelTypeEmpty, Visio::visSelModeSkipSuper);

  for(int i=1; i<=page->Shapes->Count; i++)	
  {
    Visio::IVShapePtr shape = page->Shapes->Item[i];
    if(_tcsicmp(shape->Data1,_T("1")) == 0 && _tcsicmp(shape->Data3,groupID) == 0)
      selection->Select(shape,visSelect);
  }

  vsoApp->ActiveWindow->Selection = selection;
}

Visio::IVShapePtr CEnumerateUtils::getClosestMessage(Visio::IVApplicationPtr vsoApp, const Visio::IVShapePtr shapePtr, bool onlyNumbered)
{
  Visio::IVShapesPtr shapes =  vsoApp->ActivePage->Shapes;
  std::map<double, Visio::IVShapePtr> messages;

  double x = shapePtr->Cells["PinX"]->ResultIU;
  double y = shapePtr->Cells["PinY"]->ResultIU;

  for(int i=1; i<=shapes->Count; i++)
  {
    Visio::IVShapePtr shape = shapes->Item[i];

    if(shape == shapePtr)
      continue;

    if(_tcsicmp(shape->Data1,_T("1")) == 0 || !onlyNumbered)
    {
      double tempX = shape->Cells["PinX"]->ResultIU;
      double tempY = shape->Cells["PinY"]->ResultIU;

      double distance = sqrt(pow(x-tempX,2) + pow(y-tempY,2));

      messages.insert(std::pair<double, Visio::IVShapePtr>(distance, shape));
    }
  }
  if (!messages.size())
    return NULL;

  return messages.begin()->second;
}

int CEnumerateUtils::getGroupCount(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVShapePtr pagesheet = vsoApp->ActivePage->PageSheet;

  if(!pagesheet->GetCellExists(_T("User.EnumCount"),0))
  {
    pagesheet->AddNamedRow(visSectionUser,_T("EnumCount"),visTagDefault);
    pagesheet->Cells["User.EnumCount"]->FormulaU = _T("0");
    return 0;
  }

  return pagesheet->Cells["User.EnumCount"]->ResultInt[visNone][visTruncate];
}

void CEnumerateUtils::setGroupCount(Visio::IVApplicationPtr vsoApp, int count)
{
  Visio::IVShapePtr pagesheet = vsoApp->ActivePage->PageSheet;

  if(!pagesheet->GetCellExists(_T("User.EnumCount"),0) && count != 0)
  {
    pagesheet->AddNamedRow(visSectionUser,_T("EnumCount"),visTagDefault);
  }
  else if(count == 0)
  {
    pagesheet->DeleteRow(visSectionUser,pagesheet->CellsRowIndex["User.EnumCount"]);
    return;
  }

  pagesheet->Cells["User.EnumCount"]->FormulaU = stringize() << count;
}

int CEnumerateUtils::getAutoEnumGroup(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVShapePtr pagesheet = vsoApp->ActivePage->PageSheet;

  if(pagesheet->GetCellExists(_T("User.AutoEnumGroupID"),0))
  {
    return pagesheet->Cells["User.AutoEnumGroupID"]->ResultInt[visNone][visTruncate]; 
  }

  return -1;
}

void CEnumerateUtils::eraseAutoEnumGroup(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVShapePtr pagesheet = vsoApp->ActivePage->PageSheet;
  if(getAutoEnumGroup(vsoApp) != -1)
    pagesheet->DeleteRow(visSectionUser,pagesheet->CellsRowIndex["User.AutoEnumGroupID"]);
}

VAORC CEnumerateUtils::enableEnumeration(Visio::IVShapePtr shape, _bstr_t groupID, std::set<_bstr_t>& formerGroups)
{
  if(_tcsicmp(shape->Data1,_T("")) == 0)
  {
    shape->Data1 = _T("1");
    shape->Data2 = shape->Text;
    shape->Data3 = groupID;
    shape->AddNamedRow(visSectionAction,_T("enumeration"),visTagDefault);
    shape->Cells["Actions.enumeration.Action"]->FormulaU = _T("=RUNADDONWARGS(\"Sequence Chart Studio\",\"/event=104\")");
    shape->Cells["Actions.enumeration.Menu"]->FormulaU = _T("=\"Select numbering group\"");
  }
  else if(_tcsicmp(shape->Data1,_T("1")) == 0)
  {
    formerGroups.insert(shape->Data3);
    shape->Data3 = groupID;
  }
  else 
  {
    if(MessageBox(GetActiveWindow(),
      _T("Data fields (Format -> Special...) are not empty! Message enumeration use Data1, Data2 and Data3 for storing enumeration information. One or more shapes can't be numbered! Do you want to clear data fields (invalid shape lose its caption)?"),
      _T("Error"),MB_YESNO | MB_ICONWARNING))
    {
      shape->Data1 = _T("1");
      shape->Data2 = _T("");
      shape->Data3 = _T("");
    }
    else
      return VAORC_FAILURE;
  }

  return VAORC_SUCCESS;
}

VAORC CEnumerateUtils::disableEnumeration(Visio::IVShapePtr shape)
{

  if(_tcsicmp(shape->Data1,_T("1")) == 0)		//if the enumeration was enabled
  {
    shape->Text = shape->Data2;		
    shape->Data1 = _T("");		//Clean the data fields
    shape->Data2 = _T("");
    shape->Data3 = _T("");
    shape->DeleteRow(visSectionAction,shape->GetCells(_T("Actions.enumeration"))->Row);
  }
  else if(_tcsicmp(shape->Data1,_T("")) != 0)
  {
    if(MessageBox(GetActiveWindow(),
      _T("Data1 field has wrong format. Message enumeration was corrupted! Do you want to delete data fields?"),
      _T("Error"),MB_YESNO | MB_ICONWARNING) == IDYES)
    {
      shape->Data1 = _T("");		//Clean the data fields
      shape->Data2 = _T("");
      shape->Data3 = _T("");
    }
    else
      return VAORC_FAILURE;
  }

  return VAORC_SUCCESS;
}

void CEnumerateUtils::eraseEnumInfo(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVShapePtr page = vsoApp->ActivePage->PageSheet;

  for(int i=1; i<=getGroupCount(vsoApp); i++)
  {
    _bstr_t userGroupID = stringize() << _T("User.Enum") << i;
    if(page->GetCellExists(userGroupID,0) != -1)
      continue;
    page->DeleteRow(visSectionUser,page->GetCells(userGroupID)->Row);
  }
  //erase AutoNumbering information if exists
  eraseAutoEnumGroup(page->Application);
  //restore IDs
  setGroupCount(page->Application,0);
}

void CEnumerateUtils::fillComboWithTypes(WTL::CComboBox& combo)
{
  combo.InsertString(0,_T("1, 2, 3,..."));
  combo.InsertString(1,_T("I, II, III,..."));
  combo.InsertString(2,_T("a, b, c,..."));
  combo.InsertString(3,_T("A, B, C,..."));
}

// $Id: enumerateUtils.cpp 862 2010-08-26 22:25:05Z mbezdeka $