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
 * $Id: enumerateUtils.h 949 2010-09-16 21:15:00Z mbezdeka $
 */
#pragma once

#include "stdafx.h"
#include "extract.h"
#include <string>

class CEnumerateUtils
{
public:
	  /**
     * 
     */
	static std::wstring	int2Romans(int index);
	  /**
     * Convers integer to char: a-z,aa-az,ba-bz,...,aaa-aaz,..etc.
     * @param index		integer to convert to char
	   * @param capitals	if true, creates capitals Chars as: A-Z, AA-AZ
     * @return string representing index
     */
	static std::wstring	int2Char(int index, bool capitals);
	  /**
     * Compare function for drawNumbers, decides order of message shape according to their position
     * @param p1	pointer to first message shape to compare
     * @param p2	pointer to second message shape to compare
     * @return whether first shape is "smaller" then the second one
     */
	static bool messageCompare(Visio::IVShapePtr p1, Visio::IVShapePtr p2);
	
  static bool loadGroupSettings(Visio::IVApplicationPtr vsoApp, _bstr_t groupID, int& startIndex, int& numberingType, std::wstring& addition);
	static bool saveGroupSettings(Visio::IVApplicationPtr vsoApp, _bstr_t groupID, int startIndex, int numberingType, BSTR addition);
	
  static void selectGroup(Visio::IVApplicationPtr vsoApp, _bstr_t groupID);
	
  static Visio::IVShapePtr getClosestMessage(Visio::IVApplicationPtr vsoApp, const Visio::IVShapePtr shapePtr, bool onlyNumbered);
	
  static int getGroupCount(Visio::IVApplicationPtr vsoApp);
	static void setGroupCount(Visio::IVApplicationPtr vsoApp, int count);
	
  static VAORC disableEnumeration(Visio::IVShapePtr shape);
	static VAORC enableEnumeration(Visio::IVShapePtr shape, _bstr_t groupID, std::set<_bstr_t>& formerGroups);
	
  static void fillComboWithTypes(WTL::CComboBox& combo);
  
  static int getAutoEnumGroup(Visio::IVApplicationPtr vsoApp);
  static void eraseAutoEnumGroup(Visio::IVApplicationPtr vsoApp);
  
  static void eraseEnumInfo(Visio::IVApplicationPtr vsoApp);
};

// $Id: enumerateUtils.h 949 2010-09-16 21:15:00Z mbezdeka $