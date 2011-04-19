/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 3.0, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 */

#include "data/pysc/py_conv.h"

#define ERR -7
#ifdef MUTE
// Do not print any output
#define DPRINT(a)
#else
// Debug mode, default
#define DPRINT(a) std::cout << a << std::endl
#endif
#define ERRNULL(a) if((a) == NULL){ DPRINT("Error creating " << #a);if(PyErr_Occurred()) PyErr_Print(); throw ERR;}

PyObject *mktuple(const std::wstring& a){
  PyObject *t = PyTuple_New(1);
  PyTuple_SetItem(t, 0, PyUnicode_FromWideChar(a.c_str(), a.length()));
  return t;
}

int PyConv::init(const char *module){
  if(!Py_IsInitialized())
    Py_Initialize();
  DPRINT("Importing module " << module);
  DPRINT("Current PATH: " << Py_GetPath());
  pob.name = PyString_FromString(module);
  if(pob.name == NULL){
    DPRINT(module << " cannot be converted to PyString");
    return 0;
  }
  pob.module = PyImport_Import(pob.name);
  if(pob.module == NULL){
    DPRINT(module << " cannot be imported");
    DPRINT("You must first install it (by default using setup.py installation file).");
    if(PyErr_Occurred())
      PyErr_Print();
    return 0;
  }
  pob.pDict = PyModule_GetDict(pob.module);
  if(pob.pDict == NULL){
    DPRINT("Cannot extract dictionary from " << module);
    return 0;
  }
  pob.funcBMsc = PyDict_GetItemString(pob.pDict, "checkBMsc");
  if(pob.funcBMsc == NULL){
    DPRINT("WARNING: checkBMsc is not implemented in module " << module);
//    return 0;
  }
  pob.funcHMsc = PyDict_GetItemString(pob.pDict, "checkHMsc");
  if(pob.funcHMsc == NULL){
    DPRINT("WARNING: checkBMsc is not implemented in module " << module);
//    return 0;
  }
  return 1;
}

int PyConv::reinit(const char *module){
  DPRINT("Removing old module");
  Py_XDECREF(pob.name);
  Py_XDECREF(pob.module);
  DPRINT("Importing module " << module);
  DPRINT("Current PATH: " << Py_GetPath());
  pob.name = PyString_FromString(module);
  if(pob.name == NULL){
    DPRINT(module << " cannot be converted to PyString");
    return 0;
  }
  pob.module = PyImport_Import(pob.name);
  if(pob.module == NULL){
    DPRINT(module << " cannot be imported");
    DPRINT("You must first install it (by default using setup.py installation file).");
    if(PyErr_Occurred())
      PyErr_Print();
    return 0;
  }
  pob.pDict = PyModule_GetDict(pob.module);
  if(pob.pDict == NULL){
    DPRINT("Cannot extract dictionary from " << module);
    return 0;
  }
  pob.funcBMsc = PyDict_GetItemString(pob.pDict, "checkBMsc");
  if(pob.funcBMsc == NULL){
    DPRINT("WARNING: checkBMsc is not implemented in module " << module);
//    return 0;
  }
  pob.funcHMsc = PyDict_GetItemString(pob.pDict, "checkHMsc");
  if(pob.funcHMsc == NULL){
    DPRINT("WARNING: checkBMsc is not implemented in module " << module);
//    return 0;
  }
  return 1;
}

int PyConv::check(const MscPtr& msc, const ChannelMapperPtr& chm){
  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);
  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);

  if(bmsc != NULL){
    std::list<BMscPtr> blist;
    blist = checkBMsc(bmsc, chm);
    return blist.size();
  }
  if(hmsc != NULL){
    std::list<HMscPtr> hlist;
    hlist = checkHMsc(hmsc, chm);
    return hlist.size();
  }
  DPRINT("Given Msc is neither HMsc nor BMsc");
  return 0;
}

std::list<BMscPtr> PyConv::checkBMsc(const BMscPtr& bmsc, const ChannelMapperPtr& chm){
  std::list<BMscPtr> blist;
  if(!bmsc.get()){
    DPRINT("Can't get BMsc");
    throw 0;
  }
  PyObject *pchm = create_chm(chm);
  if(pchm == NULL){
    DPRINT("WARNING: Cannot create corresponding Channel Mapper, asssuming Channel Mapper is not necessary");
    pchm = Py_None;
  }

  if(pob.funcBMsc == NULL){
    DPRINT("Input type: BMsc, but no checkBMsc");
    throw 0;
  }
  if(PyCallable_Check(pob.funcBMsc)){
    PyObject *tuple = PyTuple_New(2);
    ERRNULL(tuple);
    PyObject *pbmsc = pob.msc.get(bmsc);
    if(!pbmsc)
      pbmsc = convert_msc(bmsc);
    PyTuple_SetItem(tuple, 0, pbmsc);
    PyTuple_SetItem(tuple, 1, pchm);
    DPRINT("Calling checkBMsc");
    PyObject *presult = PyObject_CallObject(pob.funcBMsc, tuple);
    ERRNULL(presult);
    if(PyErr_Occurred()){
      DPRINT("Error occurred, Traceback:");
      PyErr_Print();
    }
    if(presult == NULL){
      DPRINT("Can't get result from checker");
      throw 0;
    }
    for(int i = 0;i < PyList_Size(presult);i++){
      BMscPtr bret = boost::dynamic_pointer_cast<BMsc>(pob.msc.cget(PyList_GetItem(presult, i)));
      blist.push_back(bret);
    }
  }
  else
    DPRINT("Function checkBMsc is not callable");
  return blist;
}

std::list<HMscPtr> PyConv::checkHMsc(const HMscPtr& hmsc, const ChannelMapperPtr& chm){
  std::list<HMscPtr> hlist;
  if(!hmsc.get()){
    DPRINT("Can't get HMsc");
    throw 0;
  }
  PyObject *pchm = create_chm(chm);
  if(pchm == NULL){
    DPRINT("WARNING: Cannot create corresponding Channel Mapper, asssuming Channel Mapper is not necessary");
    pchm = Py_None;
  }

  if(pob.funcHMsc == NULL){
    DPRINT("Input type: HMsc, but no checkHMsc");
    throw 0;
  }
  if(PyCallable_Check(pob.funcHMsc)){
    PyObject *tuple = PyTuple_New(2);
    ERRNULL(tuple);
    PyObject *phmsc = pob.msc.get(hmsc);
    if(!phmsc)
      phmsc = convert_msc(hmsc);
    PyTuple_SetItem(tuple, 0, phmsc);
    PyTuple_SetItem(tuple, 1, pchm);
    DPRINT("Calling checkHMsc");
    PyObject *presult = PyObject_CallObject(pob.funcHMsc, tuple);
    ERRNULL(presult);
    if(PyErr_Occurred()){
      DPRINT("Error occurred, Traceback:");
      PyErr_Print();
    }
    if(presult == NULL){
      DPRINT("Can't get result from checker");
      throw 0;
    }
    for(int i = 0;i < PyList_Size(presult);i++){
      HMscPtr hret = boost::dynamic_pointer_cast<HMsc>(pob.msc.cget(PyList_GetItem(presult, i)));
      hlist.push_back(hret);
    }
  }
  else
    DPRINT("Function checkHMsc is not callable");
  return hlist;
}

PyObject *PyConv::convert_msc(const MscPtr& selected_msc, const std::vector<MscPtr>& msc){
  std::set<std::wstring> printed;

  // list of MSC to be printed
  // new references may be added to m_printing by save_hmsc()
  m_printing.push_back(selected_msc);
  std::copy(msc.begin(), msc.end(), std::back_inserter(m_printing));

  for(std::list<MscPtr>::const_iterator pos = m_printing.begin();
    pos != m_printing.end(); pos++){
    if(*pos == NULL)
      continue;

    // if not already generated
    if(printed.find((*pos)->get_label()) == printed.end()){
      typed_msc(*pos);
      printed.insert((*pos)->get_label());
    }
  }

  m_printing.clear();
  return pob.msc.get(selected_msc);
}

int PyConv::typed_msc(const MscPtr& msc){
  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);
  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);

  if(bmsc != NULL)
    return convert_bmsc(bmsc);
  else if(hmsc != NULL)
    return convert_hmsc(hmsc);
  else
    return 1; // unexpected pointer
}

PyObject *PyConv::create_instance(const InstancePtr& inst){
  PyObject *pinst = pob.instance.get(inst);
  if(pinst)
    return pinst;
  pinst = PyObject_CallObject(PyDict_GetItemString(pob.pDict, "Instance"), mktuple(inst->get_label()));
  if(pinst == NULL){
    DPRINT("Instance is not callable object");
    return pinst;
  }
  DPRINT("DEBUG: Instance created(" << inst << ")");
  pob.instance.add(inst, pinst);
  return pinst;
}

const char *PyConv::get_area_type(const EventAreaPtr& area){
  if(boost::dynamic_pointer_cast<StrictOrderArea>(area))
    return "StrictOrderArea";
  if(boost::dynamic_pointer_cast<CoregionArea>(area))
    return "CoregionArea";
  return "";
}

PyObject *PyConv::create_area(const EventAreaPtr& area, const char *optype){
  const char *type = optype;
  if(type[0] == '\0'){
    type = get_area_type(area);
    if(type[0] == '\0'){
      DPRINT("Cannot get type of the area");
      return NULL;
    }
  }

  PyObject *parea = pob.area.get(area);
  if(parea)
    return parea;
  parea = PyObject_CallObject(PyDict_GetItemString(pob.pDict, type), PyTuple_New(0));
  if(parea == NULL){
    DPRINT(type << " is not callable object");
    return parea;
  }
  DPRINT("DEBUG: Area created(" << area << ")");
  pob.area.add(area, parea);
  return parea;
}

const char *PyConv::get_event_type(const EventPtr& area){
  if(boost::dynamic_pointer_cast<StrictEvent>(area))
    return "StrictEvent";
  if(boost::dynamic_pointer_cast<CoregionEvent>(area))
    return "CoregionEvent";
  return "";
}

PyObject *PyConv::create_event(const EventPtr& event, const char *optype){
  const char *type = optype;
  if(type[0] == '\0'){
    type = get_event_type(event);
    if(type[0] == '\0'){
      DPRINT("Cannot get type of the event");
      return NULL;
    }
  }

  PyObject *pevent = pob.event.get(event);
  if(pevent)
    return pevent;
  pevent = PyObject_CallObject(PyDict_GetItemString(pob.pDict, type), PyTuple_New(0));
  if(pevent == NULL){
    DPRINT(type << " is not callable object");
    return pevent;
  }
  DPRINT("DEBUG: Event created(" << event << ")");
  pob.event.add(event, pevent);
  return pevent;
}

const char *PyConv::get_message_type(const MscMessagePtr& message){
  if(boost::dynamic_pointer_cast<CompleteMessage>(message))
    return "CompleteMessage";
  if(boost::dynamic_pointer_cast<IncompleteMessage>(message))
    return "IncompleteMessage";
  return "";
}

PyObject *PyConv::create_message(const MscMessagePtr& message, const char *optype){
  const char *type = optype;
  if(type[0] == '\0'){
    type = get_message_type(message);
    if(type[0] == '\0'){
      DPRINT("Cannot get type of the message");
      return NULL;
    }
  }

  PyObject *pmessage = pob.message.get(message);
  if(pmessage)
    return pmessage;
  pmessage = PyObject_CallObject(PyDict_GetItemString(pob.pDict, type), mktuple(message->get_label()));
  if(pmessage == NULL){
    DPRINT(type << " is not callable object");
    return pmessage;
  }
  DPRINT("DEBUG: Message created(" << message << ")");
  pob.message.add(message, pmessage);
  return pmessage;
}

void PyConv::handle_event(const EventPtr& event, PyObject *pevent){
  CompleteMessagePtr complete_message = event->get_complete_message();
  PyObject *pmessage = NULL;
  if(complete_message != NULL){
    pmessage = create_message(complete_message, "CompleteMessage");
    ERRNULL(pmessage);
    if(!pob.message.is_filled(complete_message)){
      PyObject *tuple = PyTuple_New(2);
      ERRNULL(tuple);
      PyObject *psend = create_event(complete_message->get_send_event());
      ERRNULL(psend);
      PyTuple_SetItem(tuple, 0, psend);
      PyObject *preceive = create_event(complete_message->get_receive_event());
      ERRNULL(preceive);
      PyTuple_SetItem(tuple, 1, preceive);
      PyObject_SetAttrString(pmessage, "events", tuple);
    }
  }

  IncompleteMessagePtr incomplete_message = event->get_incomplete_message();
  if(incomplete_message != NULL){
    pmessage = create_message(incomplete_message, "IncompleteMessage");
    ERRNULL(pmessage);
    if(!pob.message.is_filled(incomplete_message)){
      PyObject_SetAttrString(pmessage, "event", pevent);
      if(incomplete_message->is_lost())
        PyObject_SetAttrString(pmessage, "type", PyUnicode_FromString("lost"));
      if(incomplete_message->is_found())
        PyObject_SetAttrString(pmessage, "type", PyUnicode_FromString("found"));
      PyObject *tuple = PyTuple_New(2);
      ERRNULL(tuple);
      PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(incomplete_message->get_dot_position().get_x()));
      PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(incomplete_message->get_dot_position().get_y()));
      PyObject_SetAttrString(pmessage, "dot_position", tuple);
    }
  }
}

int PyConv::convert_bmsc(const BMscPtr& bmsc){
  DPRINT("Converting BMsc");
  PyObject *pbmsc = create_msc(bmsc, "BMsc");
  ERRNULL(pbmsc);

  for(InstancePtrList::const_iterator ipos = bmsc->get_instances().begin();
    ipos != bmsc->get_instances().end(); ipos++){
    PyObject *pinst = create_instance(*ipos);
    ERRNULL(pinst);
    PyObject_SetAttrString(pbmsc, "instance", pinst);
    PyObject *tuple = PyTuple_New(2);
    ERRNULL(tuple);
    PyTuple_SetItem(tuple, 0, PyFloat_FromDouble((*ipos)->get_line_begin().get_x()));
    PyTuple_SetItem(tuple, 1, PyFloat_FromDouble((*ipos)->get_line_begin().get_y()));
    PyObject_SetAttrString(pinst, "line_begin", tuple);
    tuple = PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, PyFloat_FromDouble((*ipos)->get_line_end().get_x()));
    PyTuple_SetItem(tuple, 1, PyFloat_FromDouble((*ipos)->get_line_end().get_y()));
    PyObject_SetAttrString(pinst, "line_end", tuple);
    PyObject_SetAttrString(pinst, "width", PyFloat_FromDouble((*ipos)->get_width()));
    if((*ipos)->get_form() == LINE)
      PyObject_SetAttrString(pinst, "form", PyUnicode_FromString("line"));
    else
      PyObject_SetAttrString(pinst, "form", PyUnicode_FromString("column"));
    // walk through event areas
    for(EventAreaPtr area = (*ipos)->get_first();
      area != NULL; area = area->get_next()){
      PyObject *parea = NULL;

      StrictOrderAreaPtr strict_area = boost::dynamic_pointer_cast<StrictOrderArea>(area);
      if(strict_area != NULL){
        parea = create_area(area, "StrictOrderArea");
        ERRNULL(parea);
        PyObject_SetAttrString(pinst, "area", parea);
        
        // walk through events
        for(StrictEventPtr event = strict_area->get_first();
          event != NULL; event = event->get_successor()){
          PyObject *pevent = create_event(event, "StrictEvent");
          ERRNULL(pevent);
          PyObject_SetAttrString(parea, "event", pevent);
          if(event->get_successor() != NULL){
            PyObject *psucc = create_event(event->get_successor(), "StrictEvent");
            ERRNULL(psucc);
            PyObject_SetAttrString(pevent, "successor", psucc);
          }
          tuple = PyTuple_New(2);
          ERRNULL(tuple);
          PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(event->get_position().get_x()));
          PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(event->get_position().get_y()));
          PyObject_SetAttrString(pevent, "position", tuple);
          handle_event(event, pevent);
        }
      }

      CoregionAreaPtr coregion_area = boost::dynamic_pointer_cast<CoregionArea>(area);
      if(coregion_area != NULL){
        parea = create_area(area, "CoregionArea");
        ERRNULL(parea);
        PyObject_SetAttrString(pinst, "area", parea);
        if(coregion_area->get_form() == LINE)
          PyObject_SetAttrString(parea, "form", PyUnicode_FromString("line"));
        else
          PyObject_SetAttrString(parea, "form", PyUnicode_FromString("column"));
        // Minimal events
        // events to be processed; this is to avoid recursion
        std::list<CoregionEventPtr> event_stack;

        for(CoregionEventPVector::const_iterator mpos = coregion_area->get_minimal_events().begin();
          mpos != coregion_area->get_minimal_events().end(); mpos++){
          // initialize the stack with events with no predecessors
          push_back_if_unique<CoregionEventPtr>(event_stack, *mpos);
        }

        // process all events in the stack
        for(std::list<CoregionEventPtr>::const_iterator epos = event_stack.begin();
          epos != event_stack.end(); epos++){
          PyObject *pevent = create_event(*epos, "CoregionEvent");
          ERRNULL(pevent);
          PyObject_SetAttrString(parea, "minimal_event", pevent);
          tuple = PyTuple_New(2);
          ERRNULL(tuple);
          PyTuple_SetItem(tuple, 0, PyFloat_FromDouble((*epos)->get_position().get_x()));
          PyTuple_SetItem(tuple, 1, PyFloat_FromDouble((*epos)->get_position().get_y()));
          PyObject_SetAttrString(pevent, "position", tuple);
          handle_event(*epos, pevent);

          for(CoregEventRelPtrVector::const_iterator spos = (*epos)->get_successors().begin();
            spos != (*epos)->get_successors().end(); spos++){
            CoregionEventPtr successor = (*spos)->get_successor();
            CoregionEventPtr predecessor = (*spos)->get_predecessor();

            tuple = PyTuple_New(2);
            PyObject *ppred = create_event(predecessor, "CoregionEvent");
            ERRNULL(ppred);
            PyObject *psucc = create_event(successor, "CoregionEvent");
            ERRNULL(psucc);
            PyObject *psuccarea = create_area(successor->get_area(), "CoregionArea");
            ERRNULL(psuccarea);
            PyObject_SetAttrString(psucc, "area", psuccarea);
            PyTuple_SetItem(tuple, 0, ppred);
            PyTuple_SetItem(tuple, 1, psucc);
            PyObject_SetAttrString(pevent, "successor", PyObject_CallObject(PyDict_GetItemString(pob.pDict, "CoregionEventRelation"), tuple));

            // add successors of this event to the stack
            // note: std::list<>::push_back doesn't invalidate iterators
            push_back_if_unique<CoregionEventPtr>(event_stack, successor);
          }
        }


        // Maximal events
        event_stack.clear();

        for(CoregionEventPVector::const_iterator mpos = coregion_area->get_maximal_events().begin();
          mpos != coregion_area->get_maximal_events().end(); mpos++){
          // initialize the stack with events with no predecessors
          push_back_if_unique<CoregionEventPtr>(event_stack, *mpos);
        }

        // process all events in the stack
        for(std::list<CoregionEventPtr>::const_iterator epos = event_stack.begin();
          epos != event_stack.end(); epos++){
          PyObject *pevent = create_event(*epos, "CoregionEvent");
          ERRNULL(pevent);
          PyObject_SetAttrString(parea, "maximal_event", pevent);
          handle_event(*epos, pevent);

          for(CoregEventRelPtrVector::const_iterator spos = (*epos)->get_successors().begin();
            spos != (*epos)->get_successors().end(); spos++){
            CoregionEventPtr successor = (*spos)->get_successor();
            CoregionEventPtr predecessor = (*spos)->get_predecessor();

            tuple = PyTuple_New(2);
            ERRNULL(tuple);
            PyObject *psucc = create_event(successor, "CoregionEvent");
            ERRNULL(psucc);
            PyObject *ppred = create_event(predecessor, "CoregionEvent");
            ERRNULL(ppred);
            PyTuple_SetItem(tuple, 0, ppred);
            PyTuple_SetItem(tuple, 1, psucc);
            PyObject_SetAttrString(pevent, "successor", PyObject_CallObject(PyDict_GetItemString(pob.pDict, "CoregionEventRelation"), tuple));

            // add successors of this event to the stack
            // note: std::list<>::push_back doesn't invalidate iterators
            push_back_if_unique<CoregionEventPtr>(event_stack, successor);
          }
        }
      }
    }
  }
  return 0;
}

const char *PyConv::get_msc_type(const MscPtr& msc){
  if(boost::dynamic_pointer_cast<HMsc>(msc))
    return "HMsc";
  if(boost::dynamic_pointer_cast<BMsc>(msc))
    return "BMsc";
  return "";
}

PyObject *PyConv::create_msc(const MscPtr& msc, const char * optype){
  const char *type = optype;
  if(type[0] == '\0'){
    type = get_msc_type(msc);
    if(type[0] == '\0'){
      DPRINT("Cannot get type of the msc");
      return NULL;
    }
  }

  PyObject *pmsc = pob.msc.get(msc);
  if(pmsc)
    return pmsc;
  pmsc = PyObject_CallObject(PyDict_GetItemString(pob.pDict, type), mktuple(msc->get_label()));
  if(pmsc == NULL){
    DPRINT(type << " is not callable object");
    return pmsc;
  }
  DPRINT("DEBUG: Msc created(" << msc << ")");
  pob.msc.add(msc, pmsc);
  return pmsc;
}

const char *PyConv::get_node_type(const HMscNodePtr& node){
  if(boost::dynamic_pointer_cast<StartNode>(node))
    return "StartNode";
  if(boost::dynamic_pointer_cast<ConditionNode>(node))
    return "ConditionNode";
  if(boost::dynamic_pointer_cast<ConnectionNode>(node))
    return "ConnectionNode";
  if(boost::dynamic_pointer_cast<ReferenceNode>(node))
    return "ReferenceNode";
  if(boost::dynamic_pointer_cast<EndNode>(node))
    return "EndNode";
  return "";
}

PyObject *PyConv::create_node(const HMscNodePtr& node, const char* optype)
{
  const char* type = optype;
  if(type[0] == '\0'){
    type = get_node_type(node);
    if(type[0] == '\0'){
      DPRINT("Cannot get type of the node");
      return NULL;
    }
  }

  PyObject *pnode = pob.node.get(node);
  if(pnode)
    return pnode;
  pnode = PyObject_CallObject(PyDict_GetItemString(pob.pDict, type), PyTuple_New(0));
  if(pnode == NULL){
    DPRINT(type << " is not callable object");
    return pnode;
  }
  DPRINT("DEBUG: Node created(" << node << ")");
  pob.node.add(node, pnode);
  return pnode;
}

int PyConv::convert_hmsc(const HMscPtr& hmsc){
  DPRINT("Converting HMsc");
  PyObject *phmsc = create_msc(hmsc, "HMsc");
  ERRNULL(phmsc);
  // nodes to be processed; this is to avoid recursion
  std::list<HMscNodePtr> node_stack;

  // initialize the stack with the start node
  push_back_if_unique<HMscNodePtr>(node_stack, hmsc->get_start());

  // Data declaration -> first iteration
  // process all nodes in the stack
  for(std::list<HMscNodePtr>::const_iterator npos = node_stack.begin();
    npos != node_stack.end(); npos++){
    if(*npos == NULL){
      DPRINT("# ERROR: BAD HMSC NODE");
      continue;
    }

    PyObject *pnode = NULL;
    StartNodePtr start_node = boost::dynamic_pointer_cast<StartNode>(*npos);
    if(start_node != NULL){
      pnode = create_node(*npos, "StartNode");
      ERRNULL(pnode);
      PyObject_SetAttrString(phmsc, "start", pnode);
      pob.node.add(*npos, PyObject_CallObject(PyDict_GetItemString(pob.pDict, "Node"), mktuple(L"Start")));
    }

    ConditionNodePtr condition_node = boost::dynamic_pointer_cast<ConditionNode>(*npos);
    if(condition_node != NULL){
      pnode = create_node(*npos, "ConditionNode");
      ERRNULL(pnode);
      PyObject_SetAttrString(phmsc, "node", pnode);
      PyObject_SetAttrString(pnode, "label", PyUnicode_FromString(condition_node->get_label().c_str()));
    }

    ConnectionNodePtr connection_node = boost::dynamic_pointer_cast<ConnectionNode>(*npos);
    if(connection_node != NULL){
      pnode = create_node(*npos, "ConnectionNode");
      ERRNULL(pnode);
      PyObject_SetAttrString(pob.msc.get(hmsc), "node", pnode);
    }

    ReferenceNodePtr reference_node = boost::dynamic_pointer_cast<ReferenceNode>(*npos);
    if(reference_node != NULL){
      pnode = create_node(*npos, "ReferenceNode");
      ERRNULL(pnode);
      PyObject_SetAttrString(phmsc, "node", pnode);
      if(reference_node->get_msc() != NULL){
        PyObject *pmsc = create_msc(reference_node->get_msc());
        ERRNULL(pmsc);
        PyObject_SetAttrString(pnode, "msc", pmsc);
        m_printing.push_back(reference_node->get_msc());
      }
    }

    EndNodePtr end_node = boost::dynamic_pointer_cast<EndNode>(*npos);
    if(end_node != NULL){
      pnode = create_node(*npos, "EndNode");
      ERRNULL(pnode);
      PyObject_SetAttrString(phmsc, "node", pnode);
    }

/* Not reachable
    if(pnode == NULL){
      DPRINT("Unknown node, can not be retyped");
      continue;
    }
*/
    PyObject *tuple = PyTuple_New(2);
    ERRNULL(tuple);
    PyTuple_SetItem(tuple, 0, PyFloat_FromDouble((*npos)->get_position().get_x()));
    PyTuple_SetItem(tuple, 1, PyFloat_FromDouble((*npos)->get_position().get_y()));
    PyObject_SetAttrString(pnode, "position", tuple);

    PredecessorNode *predecessor_node = dynamic_cast<PredecessorNode*>(npos->get());
    if(predecessor_node != NULL){
      for(NodeRelationPtrVector::const_iterator spos = predecessor_node->get_successors().begin();
        spos != predecessor_node->get_successors().end(); spos++){

        SuccessorNode *successor = (*spos)->get_successor();

        HMscNode *successor_node = dynamic_cast<HMscNode*>(successor);

        PyObject *psucc = create_node(successor_node);
        ERRNULL(psucc);
        PyObject_SetAttrString(pnode, "successor", psucc);

        // add successors of this node to the stack
        // note: std::list<>::push_back doesn't invalidate iterators
        push_back_if_unique<HMscNodePtr>(node_stack, successor_node);
      }
    }
  }
  return 0;
}

const char *PyConv::get_chm_type(const ChannelMapperPtr& chm){
  if(boost::dynamic_pointer_cast<SRChannelMapper>(chm))
    return "SRChannelMapper";
  if(boost::dynamic_pointer_cast<SRMChannelMapper>(chm))
    return "SRMChannelMapper";
  return "";
}

PyObject *PyConv::create_chm(const ChannelMapperPtr& chm){
  const char *type = get_chm_type(chm);
  if(type[0] == '\0'){
    DPRINT("Cannot get type of channel mapper");
    return NULL;
  }

  PyObject *pchm = PyObject_CallObject(PyDict_GetItemString(pob.pDict, type), PyTuple_New(0));
  if(pchm == NULL)
    DPRINT(type << " is not callable object");
  return pchm;
}

PyConv::~PyConv(){
    pob.msc.clear();
    pob.node.clear();
    pob.instance.clear();
    pob.area.clear();
    pob.event.clear();
    pob.message.clear();
    // DECREF only these two, all the other PyObjects are borrowed references
    Py_XDECREF(pob.name);
    Py_XDECREF(pob.module);
}
