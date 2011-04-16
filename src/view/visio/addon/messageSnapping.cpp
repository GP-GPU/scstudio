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
* $Id: messageSnapping.cpp 952 2010-09-16 21:53:03Z mbezdeka $
*/

#include "stdafx.h"
#include "dllmodule.h"
#include "messageSnapping.h"
#include "messageJump.h"
#include "shapeUtils.h"

#define SNAP_REG_PATH _T("Software\\Sequence Chart Studio\\MessageSnapping")

bool CMessageSnapping::isEnabled()
{
  return GetRegistry<bool>(SNAP_REG_PATH, NULL, _T("SnapEnabled"), 0);
}

bool CMessageSnapping::isArrowKeysEnabled()
{
  return GetRegistry<bool>(SNAP_REG_PATH, NULL, _T("KeysEnabled"), 0);
}

MsgSnapType CMessageSnapping::getSnapType()
{
  return (MsgSnapType)GetRegistry<int>(SNAP_REG_PATH, NULL, _T("SnapType"), -1);
}

void CMessageSnapping::setEnabled(bool bEnable)
{
  SetRegistry<bool>(HKEY_CURRENT_USER, SNAP_REG_PATH, _T("SnapEnabled"), bEnable);  
}

void CMessageSnapping::setSnapType(MsgSnapType snapType)
{
  SetRegistry<int>(HKEY_CURRENT_USER, SNAP_REG_PATH, _T("snapType"), snapType);  
}

std::vector<Visio::IVShapePtr> CMessageSnapping::getIntersectInstances(Visio::IVShapePtr msgShape, double msgPosY, MsgSnapType snapType)
{
  std::vector<Visio::IVShapePtr> instShapes;
  Visio::IVApplicationPtr vsoApp = msgShape->Application;
  Visio::IVShapesPtr shapesOnPage = vsoApp->ActivePage->Shapes;

  double InstBegY, InstEndY;

  double msgBegY = CShapeUtils::getShapeCell(msgShape,"BeginY");
  double msgEndY = CShapeUtils::getShapeCell(msgShape,"EndY");

  //Searches for instances on the page and the ones matching the criteria puts into vector
  for(int i=1; i<=shapesOnPage->Count; i++)
  {
    Visio::IVShapePtr shape = shapesOnPage->Item[i];
    if(get_shape_type(shape) == ST_BMSC_INSTANCE)
    {
      InstBegY = CShapeUtils::getShapeCell(shape, "BeginY");
      InstEndY = CShapeUtils::getShapeCell(shape, "EndY");

      //If instance is upside down, swap coordinates
      if((InstBegY - InstEndY) < 0)
        std::swap(InstBegY, InstEndY);
      
      if(snapType == MSSNAP_STRAIGHTEN)
      {
        //If actual mouse position intersect instance we add it to the vector
        if((InstBegY > msgPosY) && (InstEndY < msgPosY))
          instShapes.push_back(shape);
      }
      else if(snapType == MSSNAP_PRESERVE_VERTICAL)
      {
        //If at least one ending point intersect instance we add it to the vector
        if(((InstBegY > msgBegY) && (InstEndY < msgBegY)) || ((InstBegY > msgEndY) && (InstEndY < msgEndY)))
          instShapes.push_back(shape);
      }
      else if(snapType ==  MSSNAP_PRESERVE_SLOPE)
      {
        MscPoint* result = getIntersectionWithInstance(msgShape, shape);
        if(result)
          instShapes.push_back(shape);
        delete result;
      }
      else
      {
        MessageBox(GetActiveWindow(),_T("getIntersectInstances(): Unknow snapType was selected"),_T("Error"),MB_OK | MB_ICONERROR);
        break;  // error occured, jump out from cyclus
      }
    }
  }

  return instShapes;
}

std::pair<Visio::IVShapePtr, Visio::IVShapePtr> CMessageSnapping::getClosestInstancePair(Visio::IVShapePtr msgShape, double msgSnapPointX, double msgSnapPointY,
                                                                                         const std::vector<Visio::IVShapePtr>& instances, MsgSnapType snapType)
{
  Visio::IVShapePtr leftClosestInstance = NULL;
  Visio::IVShapePtr rightClosestInstance = NULL;
  double leftClosestInstanceDist = 0, rightClosestInstanceDist = 0;
  double instBeginX, instBeginY, instEndX, instEndY, distance, mouseDistX;

  double msgBeginY = msgShape->Cells["BeginY"]->Result[visPageUnits];
  double msgEndY = msgShape->Cells["EndY"]->Result[visPageUnits];

#define _ip(x) CShapeUtils::getShapeCell((*it),x);

  for(std::vector<Visio::IVShapePtr>::const_iterator it = instances.begin(); it != instances.end(); it++)
  {
    instBeginX = _ip("BeginX"); instBeginY = _ip("BeginY"); instEndX = _ip("EndX"); instEndY = _ip("EndY");
    //Check rotation of instance...
    if(instBeginY < instEndY)
    {
      std::swap(instBeginY,instEndY);
      std::swap(instBeginX,instEndX);
    }
    //Get intersection point with instance
    MscPoint* intPoint = NULL;
    switch(snapType)
    {
      case MSSNAP_STRAIGHTEN:
        intPoint = CMessageSnapping::getIntersectionWithInstance(MscPoint(msgSnapPointX - 1, msgSnapPointY), MscPoint(msgSnapPointX, msgSnapPointY), *it);
        break;
      case MSSNAP_PRESERVE_SLOPE:
        intPoint = CMessageSnapping::getIntersectionWithInstance(msgShape, (*it));
        break;
    }
    
    //Check whether point is only on instanceLine
    if(!intPoint || !isPointOnInstancesLine(*intPoint, *it))
      continue;

    mouseDistX = msgSnapPointX - intPoint->get_x();
    distance = sqrt(pow((msgSnapPointX - intPoint->get_x()),2) + pow((msgSnapPointY - intPoint->get_y()),2));    

    delete intPoint;

    if(mouseDistX < 0)
    {
      if(!rightClosestInstance || distance < rightClosestInstanceDist)
      {
          rightClosestInstance = (*it);
          rightClosestInstanceDist = distance;
      }
    }
    else if(mouseDistX > 0)
    {
      if(!leftClosestInstance || distance < leftClosestInstanceDist)  
      {
        leftClosestInstance = (*it);
        leftClosestInstanceDist = distance;
      }
    }    
    else
      return std::pair<Visio::IVShapePtr, Visio::IVShapePtr>(NULL,NULL);  //NOTE: Ordinary snap will handle position change
  }
#undef _ip

  //restrict snapping for FOUND and LOST messages
  double shapeLeft = msgShape->Cells["BeginX"]->Result[""];
  double shapeRight = msgShape->Cells["EndX"]->Result[""];
  
  switch(get_shape_type(msgShape))
  {
  case ST_BMSC_MESSAGE_FOUND:
    (shapeLeft <= shapeRight) ? leftClosestInstance = NULL : rightClosestInstance = NULL; 
    break;
  case ST_BMSC_MESSAGE_LOST:
    (shapeLeft <= shapeRight) ? rightClosestInstance = NULL : leftClosestInstance = NULL; 
    break;
  }

  return std::pair<Visio::IVShapePtr, Visio::IVShapePtr>(leftClosestInstance,rightClosestInstance);
}

MsgConnectedEndpoints CMessageSnapping::glueMsgToInstancesPair(Visio::IVShapePtr msgShape, Visio::IVShapePtr leftInstance, Visio::IVShapePtr rightInstance, 
                                              double yPos, MsgSnapType snapType, bool dontGlueIfConnected)
{
  int result = 0;
  //if there are no instances, return
  if(!leftInstance && !rightInstance)
    return (MsgConnectedEndpoints)result;

  double yLeftPos, yRightPos; 

  //Select where ends of message should be snapped
  switch(snapType)
  {
  case MSSNAP_STRAIGHTEN:
    yLeftPos = yRightPos = yPos;  //Y-coord for snap is same for both ends of message
    break;
  case MSSNAP_PRESERVE_VERTICAL:
    yLeftPos = CShapeUtils::getShapeCell(msgShape,"BeginY"); //Y-coord for begin point of msg
    yRightPos = CShapeUtils::getShapeCell(msgShape,"EndY");  //Y-coord for end point of msg
    break;
  case MSSNAP_PRESERVE_SLOPE:
    MscPoint* p = getIntersectionWithInstance(msgShape, leftInstance);
    if(p) yLeftPos = p->get_y(); delete p;

    p = getIntersectionWithInstance(msgShape, rightInstance);
    if(p) yRightPos = p->get_y(); delete p;
    break;
  }

  MsgDirection msgDir = CShapeUtils::getMsgDirection(msgShape);
  //Glue to left instance or to right instance (at least one endpoint must be snapped to return true)
  if(glueMsgToInstance(msgShape, leftInstance, "BeginX", yLeftPos, msgDir, dontGlueIfConnected))
    result++;
  if(glueMsgToInstance(msgShape, rightInstance, "EndX", yRightPos, msgDir, dontGlueIfConnected))
    result = (!result) ? 2 : 3;

  return (MsgConnectedEndpoints)result;
}

bool CMessageSnapping::glueMsgToInstance(Visio::IVShapePtr msgShape, Visio::IVShapePtr instanceShape, 
                                         const _bstr_t & msgCell, double yPos, VisUnitCodes units)
{
  return glueMsgToInstance(msgShape, instanceShape, msgCell, yPos, CShapeUtils::getMsgDirection(msgShape), true, units);
}

bool CMessageSnapping::glueMsgToInstance(Visio::IVShapePtr msgShape, Visio::IVShapePtr instanceShape, 
                                         const _bstr_t & msgCell, double yPos, MsgDirection msgDirection, bool dontGlueIfConnected, VisUnitCodes units)
{
  if(!msgShape || !instanceShape)
    return false;

  bool isBegin = _tcsicmp(msgCell,_T("BeginX")) == 0;
  //NOTE: If user snap message by himself (using red rectangle), we are done
  if(dontGlueIfConnected)
    if(msgShape->GetCells(isBegin ? "BegTrigger" : "EndTrigger")->ResultInt[visNone][visTruncate] == 2)
      return false;

  //NOTE: Return if endpoint is found or lost message
  TShapeType shapeType = get_shape_type(msgShape);
  if( (shapeType == 2 && _tcsicmp(msgCell,_T("EndX")) == 0) || (shapeType == 3 && isBegin) )
    return false;

  bool xPos = 0;
  MscPoint instBeg(CShapeUtils::getShapeCell(instanceShape, "BeginX", units), CShapeUtils::getShapeCell(instanceShape, "BeginY", units));
  MscPoint instEnd(CShapeUtils::getShapeCell(instanceShape, "EndX", units), CShapeUtils::getShapeCell(instanceShape, "EndY", units));

  //If instance include coregion, adjust glue point
  MsgDirection posToCoregion = (msgDirection == MSG_RIGHT) ? (isBegin ? MSG_RIGHT : MSG_LEFT) : (isBegin ? MSG_LEFT : MSG_RIGHT);
  if(!coregionTreatment(instanceShape, instBeg, instEnd, posToCoregion, xPos, yPos, units))
    return false;

  double sizeOfInstance = fabs(instBeg.get_y() - instEnd.get_y());
  double msgOffset      = fabs(instBeg.get_y() - yPos);

  //avoid division by zero - when instances are in horizontal position
  if(sizeOfInstance == 0 || msgOffset > sizeOfInstance)
    return false;

  msgShape->Application->EventsEnabled = false;
  msgShape->Cells[msgCell]->GlueToPos(instanceShape, msgOffset/sizeOfInstance, (double)xPos);
  msgShape->Application->EventsEnabled = true;

  return true;
}

MscPoint* CMessageSnapping::getIntersectionWithInstance(const MscPoint& beginPoint,const MscPoint& endPoint, Visio::IVShapePtr instanceShape, VisUnitCodes units)
{
  if(!instanceShape)
    return NULL;

  MscPoint* result = NULL;
  MscPoint p1,p2,p3,p4;

  p1 = endPoint;
  p2 = beginPoint;
#define _i(x) CShapeUtils::getShapeCell(instanceShape, x, units)
  p3 = MscPoint(_i("EndX"), _i("EndY"));
  p4 = MscPoint(_i("BeginX"), _i("BeginY"));
#undef _i
 
  std::vector<Visio::IVShapePtr> cor = CShapeUtils::getConnectedShapes(instanceShape, ST_BMSC_COREGION);
  for(u_int i=0; i<cor.size(); i++)
  {
    if(!(result = getIntersectionWithCoregion(beginPoint, endPoint, cor.at(i), units)))
      continue;
    return result;
  }

  if(!(result = CPageUtils::getIntersectionPoint(p1, p2, p3, p4)))
    return NULL;
  //Swap y coord if instance is upside down
  if(p4.get_y() < p3.get_y())
    std::swap(p4, p3);
  //Check if it's outside of instance
  if(result->get_y() > p4.get_y() || result->get_y() < p3.get_y())
  {
    delete result;
    return NULL;
  }

  return result;
}

MscPoint* CMessageSnapping::getIntersectionWithInstance(Visio::IVShapePtr msgShape, Visio::IVShapePtr instanceShape, VisUnitCodes units)
{
  if(!msgShape)
    return NULL;

#define _s(x) CShapeUtils::getShapeCell(msgShape, x, units)
  MscPoint p1(_s("EndX"), _s("EndY"));
  MscPoint p2(_s("BeginX"), _s("BeginY"));
#undef _s
  
  return getIntersectionWithInstance(p2, p1, instanceShape, units);
}

bool CMessageSnapping::isPointOnInstancesLine(const MscPoint& point, Visio::IVShapePtr instShape, VisUnitCodes units)
{
  if(!instShape)
    return false;

#define _sp(x) CShapeUtils::getShapeCell(instShape, x, units)

  double InstOffset = 0;

  //Get values from cells
  if(instShape->CellExists["Controls.mscHeadWidth"][visExistsAnywhere])  //NOTE: Check if the instance is not headless
    InstOffset = _sp("Controls.mscHeadWidth.X"); //NOTE: get width of instances rectangles

  double sizeOfInstance = _sp("Width");
  double msgOffset      = sqrt( pow(point.get_x() - _sp("BeginX"),2) + pow(point.get_y() - _sp("BeginY"),2) );

  //Glue to position, if message is on "rectangles" on the instance it won't be glued
  return !((msgOffset < InstOffset) || (msgOffset > (sizeOfInstance - InstOffset)));

#undef _sp
}

bool CMessageSnapping::getMsgVector(Visio::IVShapePtr msgShape, MscPoint* resultVector, VisUnitCodes units)
{
  if(!msgShape || !resultVector || !isMessageShape(msgShape))
    return false;

  try {
  resultVector->set_x(msgShape->Cells["EndX"]->Result[units] - msgShape->Cells["BeginX"]->Result[units]);
  resultVector->set_y(msgShape->Cells["EndY"]->Result[units] - msgShape->Cells["BeginY"]->Result[units]);
  } catch(_com_error err) {
    MessageBox(GetActiveWindow(), L"getMsgVector:: exception occurred", L"", MB_ICONERROR);
  }

  return true;
}

void CMessageSnapping::snap(Visio::IVShapePtr msgShape, double posX, double posY, MsgSnapType snapType)
{
  if(!msgShape)
    return;

  long scopeID = msgShape->Application->BeginUndoScope(_T("snap"));
#define _sp(x) CShapeUtils::getShapeCell(msgShape, x)

  //Step 1: Check snap type
  if(snapType < 0 || snapType > 2)
  {
    MessageBox(GetActiveWindow(), _T("Error occurred: Unknown snap type"), _T("Error"), MB_OK | MB_ICONERROR);
    return;
  }

  MscPoint msgVec;
  if(!getMsgVector(msgShape, &msgVec, (VisUnitCodes) 0))
    return;
  
  //Step 2: If message is only horizontal we can take as Y coord PinY and don't have to calculate oblique
  if(_sp("BeginY") == _sp("EndY"))
  {
    posY = _sp("PinY");
    snapType = MSSNAP_STRAIGHTEN;
  }
  
  //Step 3: Get all instances matching criteria
  std::vector<Visio::IVShapePtr> instShapes = getIntersectInstances(msgShape, posY, snapType);
  
  //Step 4: Get the closest instance on the left and on the right
  std::pair<Visio::IVShapePtr,Visio::IVShapePtr> closestInst;
  if(snapType == MSSNAP_PRESERVE_VERTICAL)
  {
    //left closest instance
    std::pair<Visio::IVShapePtr,Visio::IVShapePtr> closestInstLeft = getClosestInstancePair(msgShape, posX, _sp("BeginY"), instShapes, MSSNAP_STRAIGHTEN);             
    std::pair<Visio::IVShapePtr,Visio::IVShapePtr> closestInstRight = getClosestInstancePair(msgShape, posX, _sp("EndY"), instShapes, MSSNAP_STRAIGHTEN);       
    closestInst.first = ((_sp("BeginX") - _sp("EndX")) < 0) ? closestInstLeft.first : closestInstRight.first;
    closestInst.second =((_sp("BeginX") - _sp("EndX")) < 0) ? closestInstRight.second : closestInstLeft.second;
  }
  else
    closestInst = getClosestInstancePair(msgShape, posX, posY, instShapes, snapType);
  
  //Step 5: If message is left-oriented switch pointers
  if(CShapeUtils::getMsgDirection(msgShape) == MSG_LEFT)
    CShapeUtils::swapShape(closestInst.first, closestInst.second);

  //Step 6:Glue to them
  glueMsgToInstancesPair(msgShape, closestInst.first, closestInst.second, posY, snapType);
  
  //Step 7: Do some corrections if only one of instances intersect 
  //FIXME: Cut to functions
  if(closestInst.first ^ closestInst.second)
  {    
    //Step 7.1: Straighten messages when it's snapped only to one instance (must disable events, because CellChanged would be triggered)
    if(snapType == MSSNAP_STRAIGHTEN)
    {
      msgShape->Application->EventsEnabled = false;
      if(!closestInst.second)
        msgShape->Cells["EndY"]->FormulaU = stringize() << msgShape->Cells["BeginY"]->Result[""];
      else if(!closestInst.first)
        msgShape->Cells["BeginY"]->FormulaU = stringize() << msgShape->Cells["EndY"]->Result[""];
      msgShape->Application->EventsEnabled = true;
    }

    //Step 7.2: If message is snapped only to one instance, preserve its length
    msgShape->Application->EventsEnabled = false;
    if(closestInst.first) //LEFT point is snapped, adjust right point
    {
      msgShape->Cells["EndX"]->FormulaU = stringize() << msgShape->Cells["BeginX"]->Result[""] + msgVec.get_x();
      if(snapType != MSSNAP_STRAIGHTEN) msgShape->Cells["EndY"]->FormulaU = stringize() << msgShape->Cells["BeginY"]->Result[""] + msgVec.get_y();
    }

    if(closestInst.second) //RIGHT point is snapped, adjust left point
    {
      msgShape->Cells["BeginX"]->FormulaU = stringize() << msgShape->Cells["EndX"]->Result[""] - msgVec.get_x();
      if(snapType != MSSNAP_STRAIGHTEN) msgShape->Cells["BeginY"]->FormulaU = stringize() << msgShape->Cells["EndY"]->Result[""] - msgVec.get_y();
    }
    msgShape->Application->EventsEnabled = true;
  }

#undef _sp
   msgShape->Application->EndUndoScope(scopeID,true);
  
  //FIXME: When user add a new shape and hold ctrl key, snapping doesn't work correctly
}

std::vector<Visio::IVShapePtr> CMessageSnapping::getConnectedInstances(Visio::IVShapePtr msgShape)
{
  std::vector<Visio::IVShapePtr> instances;
  if(!msgShape)
    return instances;

  Visio::IVConnectsPtr conns = msgShape->Connects;
  for(int i=1; i<=conns->Count; i++)
  {
    Visio::IVConnectPtr conItem = conns->Item[i];
    Visio::IVShapePtr shape = conItem->ToCell->Shape;
    if(get_shape_type(shape) == ST_BMSC_COREGION)
      shape = shape->Connects->ToSheet; //NOTE: If we are connected to coregion, we treat it as we are connected to instance
    instances.push_back(shape);
  }
  return instances;
}

bool CMessageSnapping::resnap(Visio::IVShapePtr msgShape, Visio::IVShapesPtr shapes, double precision)
{
  if(!msgShape || !shapes || !isMessageShape(msgShape))
    return false;

  long scopeId = msgShape->Application->BeginUndoScope(_T("Re-snap"));

  MscPoint beginPoint(msgShape->Cells["BeginX"]->Result[""], msgShape->Cells["BeginY"]->Result[""]);
  MscPoint endPoint(msgShape->Cells["EndX"]->Result[""], msgShape->Cells["EndY"]->Result[""]);

  for(int i=1; i<=shapes->Count; i++)
  {
    Visio::IVShapePtr it = shapes->Item[i];
    TShapeType shapeType = get_shape_type(it);
    if(shapeType != ST_BMSC_INSTANCE)
      continue;

    MscPoint* bufferPoint = getIntersectionWithInstance(msgShape,it, (VisUnitCodes)0);
    if(bufferPoint)
    {
      if(pointsEqual(*bufferPoint, beginPoint, precision) && isPointOnInstancesLine(beginPoint, it, (VisUnitCodes)0))
        glueMsgToInstance(msgShape, it, "BeginX", beginPoint.get_y(), (VisUnitCodes)0);
      else if(pointsEqual(*bufferPoint, endPoint, precision) && isPointOnInstancesLine(endPoint, it, (VisUnitCodes)0))
        glueMsgToInstance(msgShape, it, "EndX", endPoint.get_y(), (VisUnitCodes)0);
    }
    delete bufferPoint;
  }
  msgShape->Application->EndUndoScope(scopeId,true);

  return true;
}

bool CMessageSnapping::pointsEqual(const MscPoint& point1, const MscPoint& point2, double precision)
{
  return (abs(point1.get_x() - point2.get_x()) < precision) && (abs(point1.get_y() - point2.get_y()) < precision);
}

Visio::IVShapePtr CMessageSnapping::getCoregionAt(Visio::IVShapePtr instShape, double yPos, VisUnitCodes units)
{
  if(get_shape_type(instShape) != ST_BMSC_INSTANCE)
    return NULL;

  Visio::IVConnectsPtr conns = instShape->FromConnects;

  for(int i = 1; i <= conns->Count; i++)
  {
    Visio::IVShapePtr shape = conns->Item[i]->FromSheet;
    if(get_shape_type(shape) != ST_BMSC_COREGION)
      continue;

    double beginY = CShapeUtils::getShapeCell(shape, "BeginY", units);
    double endY = CShapeUtils::getShapeCell(shape, "EndY", units);
    if(beginY < endY)
      std::swap(beginY, endY);
    if( (yPos <= beginY) && (yPos >= endY) )
      return shape;
  }

  return NULL;
}

Visio::IVShapePtr CMessageSnapping::snapEndPointToClosestInstance(Visio::IVShapePtr msgShape, _bstr_t endPointX, MsgDirection direction)
{
  if(!msgShape)
    return NULL;

  bool isBegin = _tcsicmp(endPointX,_T("BeginX")) == 0;
  //NOTE: If user snap message by himself (using red rectangle), we are done
  if(msgShape->GetCells(isBegin ? "BegTrigger" : "EndTrigger")->ResultInt[visNone][visTruncate] == 2)
    return NULL;

  MscPoint endPoint(CShapeUtils::getShapeCell(msgShape,endPointX),
                    CShapeUtils::getShapeCell(msgShape,isBegin ? "BeginY" : "EndY"));

  MsgJumpDirection closestInstDir = isBegin ? MSJUMP_LEFT : MSJUMP_RIGHT;
  if(direction == MSG_LEFT)
    closestInstDir = (MsgJumpDirection)!closestInstDir;

  std::vector<Visio::IVShapePtr> inst = CMessageSnapping::getIntersectInstances(msgShape, endPoint.get_y(), MSSNAP_STRAIGHTEN);
  Visio::IVShapePtr instance = CMessageJump::getClosestInstanceFromShape(endPoint, msgShape, inst, closestInstDir);

  CMessageSnapping::glueMsgToInstance(msgShape, instance, endPointX, endPoint.get_y());

  return instance;
}

void CMessageSnapping::setMsgLength(Visio::IVShapePtr msgShape, MscPoint msgVector, VisCellIndices fixedColumnX)
{
  if(!msgShape || !isMessageShape(msgShape))
    return;

  msgShape->Application->EventsEnabled = false;

  int sign = (fixedColumnX == vis1DBeginX) ? 1 : -1;
  try {
  msgShape->GetCellsSRC(visSectionObject,visRowXForm1D, fixedColumnX+(2*sign))->Formula = 
        stringize() << msgShape->GetCellsSRC(visSectionObject,
                                             visRowXForm1D,
                                             fixedColumnX)->Result[visMillimeters] + sign * msgVector.get_x() << " mm";
      
  msgShape->GetCellsSRC(visSectionObject,visRowXForm1D, (fixedColumnX + 1)+(2*sign))->Formula = 
        stringize() << msgShape->GetCellsSRC(visSectionObject,
                                             visRowXForm1D, 
                                             fixedColumnX+1)->Result[visMillimeters] + sign * msgVector.get_y() << " mm";
  }
  catch(_com_error err) {
    MessageBox(GetActiveWindow(), L"setMsgLength: exception occurred", L"Error", MB_OK);
  }
  msgShape->Application->EventsEnabled = true;
}

MscPoint* CMessageSnapping::getIntersectionWithCoregion(MscPoint beginPoint, MscPoint endPoint, 
                                                   Visio::IVShapePtr coregionShape, VisUnitCodes units)
{
  if(!coregionShape || get_shape_type(coregionShape) != ST_BMSC_COREGION)
    return NULL;

  double offsetX = CShapeUtils::getCoregionHeight(coregionShape, units);

#define _i(x) CShapeUtils::getShapeCell(coregionShape, x, units)
  MscPoint corBeginPoint(_i("BeginX"), _i("BeginY"));
  MscPoint corEndPoint(_i("EndX"), _i("EndY"));
#undef  _i

  if(corBeginPoint.get_y() < corEndPoint.get_y())
    std::swap(corBeginPoint, corEndPoint); //NOTE: No coregion can't be upside down
  //STEP 1: Get relative message position to coregion (left or right side)
  //FIXME: Doesn't corespond with mouse position on snapping - do it better
  if(beginPoint.get_x() > endPoint.get_x())
    std::swap(beginPoint, endPoint); //NOTE: Every message is treated as right-headed
  MscPoint* point = NULL;
  if(!(point = CPageUtils::getIntersectionPoint(endPoint, beginPoint, corEndPoint, corBeginPoint)))
    return NULL;
  offsetX *= (point->get_x() < endPoint.get_x()) ? 1 : -1; //FIXME: Deciding whether message should be on the right or on the left isn't accurate
  delete point;

  //STEP 2: Move coregion points at distance of coregion height
  MscPoint corNormalVec = CPageUtils::getNormalVec(corBeginPoint - corEndPoint, offsetX);
  
  //STEP 3: Compute intersection
  corBeginPoint = corBeginPoint + corNormalVec;
  corEndPoint = corEndPoint + corNormalVec;

  MscPoint * resultPoint = NULL;
  if(!(resultPoint = CPageUtils::getIntersectionPoint(endPoint, beginPoint, corEndPoint, corBeginPoint)))
    return NULL;
  //STEP 4: Check boundaries
  if((point->get_y() < corBeginPoint.get_y()) && (point->get_y() > corEndPoint.get_y()))
    return resultPoint;

  delete resultPoint;
  return NULL;
}

bool CMessageSnapping::coregionTreatment(Visio::IVShapePtr& instanceShape, MscPoint& instBegin, MscPoint& instEnd, 
                                         MsgDirection posToCoregion, bool &xPos, const double &yPos, VisUnitCodes units)
{
  if(!instanceShape || get_shape_type(instanceShape) != ST_BMSC_INSTANCE)
    return false;

  //Get all coregions the instance contains
  std::vector<Visio::IVShapePtr> coregs = CShapeUtils::getConnectedShapes(instanceShape, ST_BMSC_COREGION);
  if(!coregs.size())
    return true;

  bool inDeadZone = false;
  bool upside = false;
  for(u_int i = 0; i < coregs.size(); i++)
  {
    MscPoint corBeg(CShapeUtils::getShapeCell(coregs.at(i), "BeginX", units), CShapeUtils::getShapeCell(coregs.at(i), "BeginY", units));
    MscPoint corEnd(CShapeUtils::getShapeCell(coregs.at(i), "EndX", units), CShapeUtils::getShapeCell(coregs.at(i), "EndY", units));

    int corSide       = (posToCoregion == MSG_LEFT) ? -1 : 1;
    double corHeight  = CShapeUtils::getCoregionHeight(coregs.at(i), units) * corSide;

    if(corBeg.get_y() < corEnd.get_y())
    {
      upside = true;
      xPos = !xPos;
      std::swap(corBeg, corEnd); //NOTE: Coregion is never upside down
    }
    
    MscPoint normalVec = CPageUtils::getNormalVec(corBeg-corEnd, corHeight);
    MscPoint _corBeg = corBeg + normalVec;
    MscPoint _corEnd = corEnd + normalVec;

    if(yPos <= _corBeg.get_y() && yPos >= _corEnd.get_y())
    {
      instanceShape = coregs.at(i);
      instBegin = upside ? _corEnd : _corBeg;
      instEnd = upside? _corBeg : _corEnd;
      if(corSide == 1) xPos = !xPos;
      return true;
    }

    //Check the coregion dead zone (applies only to oblique coregions)
    bool side = (posToCoregion == MSG_RIGHT); 
    if(corBeg.get_x() > corEnd.get_x()) side = !side; //NOTE: Coregion is titled to the right
    if(side ? (yPos < _corEnd.get_y() && yPos > corEnd.get_y()) : (yPos < corBeg.get_y() && yPos > _corBeg.get_y()))
      inDeadZone = true;
  }
  return !inDeadZone;
}
// $Id: messageSnapping.cpp 952 2010-09-16 21:53:03Z mbezdeka $