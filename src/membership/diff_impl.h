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
 * Copyright (c) 2008 Matus Madzin <gotti@mail.muni.cz>
 *
 */

#ifndef __DIFF_IMPL__
#define __DIFF_IMPL__

#include "membership_alg.h"

enum Operation {ADD, REMOVE};
enum Direction {SEND, RECEIVE};	

class Distance;
class Unmatched;
class Difference;

class Distance
{
private:
  StrictEventPtr event;
  int line;

public:
  Distance(StrictEventPtr e, int l)
  {
    event = e;
    line = l;
  }

  StrictEventPtr getEvent()
  {
    return event;
  }

  void setEvent(StrictEventPtr e)
  {
    event = e;
  }

  int getLine()
  {
    return line;
  }

  void setLine(int l)
  {
    line = l;
  }
};

class Unmatched
{
private:
  StrictEventPtr event;
  MscMessagePtr original_message;

public:
  Unmatched(StrictEventPtr e, MscMessagePtr m)
  {
    event = e;
    original_message = m;
  }
  
  StrictEventPtr getEvent()
  {
    return event;
  }
  
  MscMessagePtr getMessage()
  {
    return original_message;
  }
};

class Difference
{
private:
  Difference* previous;
  StrictEventPtr location;
  MscMessagePtr message;
  enum Direction dir;
  enum Operation op;
  int attribute;     //! ATTRIBUTE part, INSERT operation: attribute of matching event of missing event.
  int line1;
  int line2;

public:
  Difference()
  {
    previous = NULL;
  }

  void setPrevious(Difference* diff)
  {
    previous = diff;
  }

  Difference* getPrevious()
  {
    return previous;
  }

  void setOperation(enum Operation operation)
  {
    op = operation;
  }

  enum Operation getOperation()
  {
    return op;
  }

  void setAttribute(int i)
  {
    attribute = i;
  }

  int getAttribute()
  {
    return attribute;
  }

  void setLine1(int row)
  {
    line1 = row;
  }
  
  int getLine1()
  {
    return line1;
  }

  void setLine2(int col)
  {
    line2 = col;
  }

  int getLine2()
  {
    return line2;
  }

  void setLocation(StrictEventPtr e)
  {
    location = e;
  }

  StrictEventPtr getLocation()
  {
    return location;
  }

  /**
   * parameter m - message connected with the event 
   *               (REMOVE - message of deleted event)
   *               (ADD - message which is supposed to add)
   *           send - bool whether the event is supposed to be send.
   */
  void setMessage(MscMessagePtr m, bool send)
  {
    message = m;
    
    if(send)
      dir = SEND;
    else
      dir = RECEIVE;
  }

  MscMessagePtr getMessage()
  {
    return message;
  }

  enum Direction getDirection()
  {
    return dir;
  }
};

void diff(InstancePtr first, InstancePtr second);

#endif
