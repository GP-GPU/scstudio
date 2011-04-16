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
 * Copyright (c) 2010 Ondrej Bouda <ondrej.bouda@wheee.cz>
 *
 * $Id: enums.h 1003 2010-11-28 09:58:04Z mbezdeka $
 */

#pragma once

enum TOrderingDirection {
  ORDER_ASC,
  ORDER_DESC,
};

enum MsgSeqDirection
{
  MSDIR_LEFT,
  MSDIR_RIGHT,
  MSDIR_LEFT_RIGHT,
  MSDIR_RIGHT_LEFT,
};

enum MsgSeqCoregionTreatment
{
  MSCOR_ERROR,
  MSCOR_CONTINUE,
  MSCOR_CONNECT_LINE,
  MSCOR_CONNECT_SIDE_SIDE,
};

enum SelectionType
{
  SELECTION_ADD,
  SELECTION_REPLACE,
};

enum MsgSnapType
{
  MSSNAP_STRAIGHTEN,
  MSSNAP_PRESERVE_VERTICAL,
  MSSNAP_PRESERVE_SLOPE,
};

enum MsgJumpDirection
{
  MSJUMP_RIGHT = 0,
  MSJUMP_LEFT,
};

enum MsgDirection
{
  MSG_RIGHT = 0,
  MSG_LEFT,
};

enum MsgConnectedEndpoints
{
  MSCE_NONE = 0,
  MSCE_BEGIN,
  MSCE_END,
  MSCE_BOTH,
  MSCE_UNKNOWN,
};

enum ShapeColor
{
  SC_BLACK = 0,
  SC_WHITE,
  SC_RED,
  SC_GREEN,
  SC_BLUE,
  SC_YELLOW,
  SC_PINK,
};

// $Id: enums.h 1003 2010-11-28 09:58:04Z mbezdeka $
