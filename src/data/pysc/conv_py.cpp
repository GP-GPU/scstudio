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
  pob.name = PyUnicode_FromString(module);
  if(pob.name == NULL){
    DPRINT(module << " cannot be converted to PyUnicode");
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
  return init(module);
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

MscPtr ConvPy::convert_msc(PyObject *selected_msc){
  std::set<std::wstring> printed;

  // list of MSC to be printed
  // new references may be added to m_printing by save_hmsc()

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

std::wstring get_label(PyObject *py){
  PyObject *plabel = PyObject_GetAttrString(py, "label");
  Py_ssize_t psize = PyUnicode_GetSize(plabel);
  wchar_t *wret = new wchar_t [psize + 1];
  PyUnicode_AsWideChar((PyUnicodeObject *)plabel, wret, psize);
  wret[psize] = '\0';
  std::wstring clabel(wret);
  return clabel;
}

int ConvPy::typed_msc(PyObject *msc){
  if(PyObject_GetAttrString(msc, "BMsc") == Py_True)
    return convert_bmsc(bmsc);
  else if(PyObject_GetAttrString(msc, "HMsc") == Py_True)
    return convert_hmsc(hmsc);
  else
    return 1; // unexpected pointer
}

InstancePtr ConvPy::create_instance(PyObject *inst){
  InstancePtr cinst = pob.instance.get(inst);
  if(cinst)
    return cinst;
  cinst = InstancePtr(new Instance(get_label(inst)));
  if(cinst == NULL){
    DPRINT("Instance is not callable object");
    return cinst;
  }
  DPRINT("DEBUG: Instance created(" << cinst << ")");
  pob.instance.add(cinst, inst);
  return cinst;
}

const char *PyConv::get_area_type(PyObject *area){
  if(PyObject_GetAttrString(boost::dynamic_pointer_cast<StrictOrderArea>(area)))
    return "StrictOrderArea";
  if(boost::dynamic_pointer_cast<CoregionArea>(area))
    return "CoregionArea";
  return "";
}

EventAreaPtr ConvPy::create_area(PyObject *area){
  EventAreaPtr carea = pob.area.get(area);
  if(carea)
    return carea;
  if(PyObject_GetAttrString(area, "StrictOrderArea") == Py_True){
    StrictOrderAreaPtr sarea = StrictOrderAreaPtr(new StrictOrderArea());
    pob.area.add(sarea, area);
    return sarea;
  }
  if(PyObject_GetAttrString(area, "CoregionArea") == Py_True){
    CoregionAreaPtr aarea = CoregionAreaPtr(new CoregionArea());
    pob.area.add(aarea, area);
    return aarea;
  }
  return carea;
}

const char *PyConv::get_event_type(const EventPtr& area){
  if(boost::dynamic_pointer_cast<StrictEvent>(area))
    return "StrictEvent";
  if(boost::dynamic_pointer_cast<CoregionEvent>(area))
    return "CoregionEvent";
  return "";
}

EventPtr ConvPy::create_event(PyObject *event){
  EventPtr cevent = pob.event.get(event);
  if(cevent)
    return cevent;
  if(PyObject_GetAttrString(event, "StrictEvent") == Py_True){
    StrictEventPtr sevent = StrictEventPtr(new StrictEvent());
    pob.event.add(sevent, event);
    return sevent;
  }
  if(PyObject_GetAttrString(event, "CoregionEvent") == Py_True){
    CoregionEventPtr aevent = CoregionEventPtr(new CoregionEvent());
    pob.event.add(aevent, event);
    return aevent;
  }
  DPRINT("DEBUG: Event created(" << event << ")");
  return cevent;
}

const char *PyConv::get_message_type(const MscMessagePtr& message){
  if(boost::dynamic_pointer_cast<CompleteMessage>(message))
    return "CompleteMessage";
  if(boost::dynamic_pointer_cast<IncompleteMessage>(message))
    return "IncompleteMessage";
  return "";
}

MscMessagePtr ConvPy::create_message(PyObject *message){
  MscMessagePtr cmessage = pob.message.get(message);
  if(cmessage)
    return cmessage;
  if(PyObject_GetAttrString(message, "CompleteMessage") == Py_True){
    CompleteMessagePtr pmessage = CompleteMessagePtr(new CompleteMessage(get_label(message)));
    pob.message.add(pmessage, message);
    return pmessage;
  }
  if(PyObject_GetAttrString(message, "IncompleteMessage") == Py_True){
    IncompleteMessagePtr imessage = IncompleteMessagePtr(new IncompleteMessage(get_label(message)));
    pob.message.add(imessage, message);
    return imessage;
  }
  DPRINT("DEBUG: Message created(" << message << ")");
  return cmessage;
}

void ConvPy::handle_event(PyObject *event, EventPtr cevent){
  PyObject *complete_message = PyObject_GetAttrString(event, "complete_message");
  MscMessagePtr cmessage = NULL;
  if(complete_message != Py_None){
    cmessage = create_message(complete_message, "CompleteMessage");
    ERRNULL(cmessage);
    if(!pob.message.is_filled(complete_message)){
      EventPtr csend = create_event(PyObject_GetAttrString(complete_message, "send_event"));
      ERRNULL(csend);
      EventPtr creceive = create_event(PyObject_GetAttrString(complete_message, "receive_event"));
      ERRNULL(creceive);
      cmessage->glue_events(csend, creceive);
    }
  }

  PyObject *incomplete_message = PyObject_GetAttrString(event, "incomplete_message");
  if(incomplete_message != Py_None){
    cmessage = create_message(incomplete_message, "IncompleteMessage");
    ERRNULL(cmessage);
    if(!pob.message.is_filled(incomplete_message)){
      cmessage->glue_event(cevent);
      // This has to be accessed/created when new object is created
      if(PyObject_GetAttrString(incomplete_message, "is_lost") == Py_True)
        PyObject_SetAttrString(pmessage, "type", PyUnicode_FromString("lost"));
      if(PyObject_GetAttrString(incomplete_message, "is_found") == Py_True)
        PyObject_SetAttrString(pmessage, "type", PyUnicode_FromString("found"));
      MscPoint pnt;

      PyObject *tuple = PyObject_GetAttrString(incomplete_message, "dot_position");
      if(tuple == Py_None){
        //Problem
	return;
      }
      double a = PyFloat_AsDouble(PyTuple_GetItem(tuple, 0));
      double b = PyFloat_AsDouble(PyTuple_GetItem(tuple, 1));
      MscPoint pnt(a, b);
      cmessage->set_dot_position(pnt);
    }
  }
}

int ConvPy::convert_bmsc(PyObject *bmsc){
  DPRINT("Converting BMsc");
  BMscPtr cbmsc = create_msc(bmsc);
  ERRNULL(cbmsc);

  PyObject *linst = PyObject_GetAttrString(bmsc, "linstances");
  for(int ipos = 0;ipos < PyList_Size(linst); ipos++){
    PyObject *inst = PyList_GetItem(linst, ipos);
    InstancePtr cinst = create_instance(inst);
    ERRNULL(cinst);
    cbmsc->add_instance(cinst);
    PyObject *tuple = PyObject_GetAttrString(inst, "line_begin");
    if(tuple != Py_None){
      MscPoint mpnt(PyFloat_AsDouble(PyTuple_GetItem(tuple, 0)), PyFloat_AsDouble(PyTuple_GetItem(tuple, 1)));
      cinst->set_line_begin(mpnt);
    }
    tuple = PyObject_GetAttrString(inst, "line_end");
    if(tuple != Py_None){
      MscPoint mpnt(PyFloat_AsDouble(PyTuple_GetItem(tuple, 0)), PyFloat_AsDouble(PyTuple_GetItem(tuple, 1)));
      cinst->set_line_end(mpnt);
    }
    cinst->set_width(PyFloat_AsDouble(PyObject_GetAttrString(inst, "width")));
    // No form, might need update
/*    if(PyUnicode_AsWideChar(PyObject_GetAttrString(inst, "form")) == L"line")
      cinst->set_form(PyObject_SetAttrString(pinst, "form", PyUnicode_FromString("line"));
    else
      PyObject_SetAttrString(pinst, "form", PyUnicode_FromString("column"));*/
    // walk through event areas
    PyObject *larea = PyObject_GetAttrString(inst, "lareas");
    for(int apos = 0;apos < PyList_Size(larea);apos++){
      PyObject *area = PyList_GetItem(larea, apos);
      EventAreaPtr carea;

      if(PyObject_GetAttrString(area, "StrictOrderArea") == PyTrue){
        carea = create_area(area);
        ERRNULL(carea);
	cinst->add_area(carea);
        
        // walk through events
	PyObject *levent = PyObject_GetAttrString(area, "levents");
        for(int epos = 0;epos < PyList_Size(levent);epos++){
	  PyObject *event = PyList_GetItem(levent, epos);
          StrictEventPtr cevent = create_event(event);
          ERRNULL(cevent);
	  carea->add_event(cevent);
          if(PyObject_GetAttrString(event, "successor") != PyNone){
	    StrictEventPtr csucc = create_event(PyObject_GetAttrString(event, "successor"));
            ERRNULL(csucc);
	    cevent->set_successor(csucc);
          }
          tuple = PyObject_GetAttrString(event, "position");
	  if(tuple != Py_None){
	    MscPoint mpnt(PyFloat_AsDouble(PyTuple_GetItem(tuple, 0)), PyFloat_AsDouble(PyTuple_GetItem(tuple, 1)));
	    cevent->set_position(mpnt);
	  }
          handle_event(event, cevent);
        }
      }

      if(PyObject_GetAttrString(area, "CoregionArea") == PyTrue){
        carea = create_area(area);
        ERRNULL(carea);
	cinst->add_area(carea);
        // Not now, can't set form...
	/*if(coregion_area->get_form() == LINE)
          PyObject_SetAttrString(parea, "form", PyUnicode_FromString("line"));
        else
          PyObject_SetAttrString(parea, "form", PyUnicode_FromString("column"));*/
        // Minimal events
        // events to be processed; this is to avoid recursion

        // process all events in the stack
	PyObject *levent = PyObject_GetAttrString(area, "lminevents");
        for(int epos = 0;epos < PyList_Size(levent);epos++){
	  PyObject *event = PyList_GetItem(levent, epos);
          CoregionEventPtr cevent = create_event(event);
          ERRNULL(cevent);
          carea->add_minimal_event(cevent);
          tuple = PyObject_GetAttrString(event, "position");
          if(tuple != PyNone){
	    MscPoint mpnt(PyFloat_AsDouble(PyTuple_GetItem(tuple, 0)), PyFloat_AsDouble(PyTuple_GetItem(tuple, 1)));
	    cevent->set_position(mpnt);
	  }
          handle_event(event, cevent);

          PyObject *lcorel = PyObject_GetAttrString(event, "lcoregionrelations");
          for(int spos = 0;spos < PyList_Size(lcorel);spos++){
	    PyObject *corel = PyList_GetItem(lcorel, spos);
	    PyObject *successor = PyObject_GetAttrString(corel, "successor");
	    PyObject *predecessor = PyObject_GetAttrString(corel, "predecessor");

            EventPtr cpred = create_event(predecessor);
            ERRNULL(cpred);
            EventPtr csucc = create_event(successor);
            ERRNULL(csucc);
	    CoregionAreaPtr csuccarea = create_area(PyObject_GetAttrString(successor, "area"));
            ERRNULL(csuccarea);
	    csucc->set_area(csuccarea);
	    CoregEventRelPtr ccorevrel = CoregEventRelPtr(new CoregionEventRelation(cpred, csucc));
	    cevent->add_successor(ccorevrel);

            // add successors of this event to the stack
            // note: std::list<>::push_back doesn't invalidate iterators
          }
        }


        // Maximal events
	PyObject *levent = PyObject_GetAttrString(area, "lmaxevents");
	for(int epos = 0;epos < PyList_Size(levent);epos++){
	  PyObject *event = PyList_GetItem(levent, epos);
          CoregionEventPtr cevent = create_event(event);
          ERRNULL(cevent);
	  carea->add_maximal_event(cevent);
          handle_event(*epos, pevent);

          PyObject *lcorel = PyObject_GetAttrString(event, "lcoregionrelations");
          for(int spos = 0;spos < PyList_Size(lcorel);spos++){
	    PyObject *corel = PyList_GetItem(lcorel, spos);
	    PyObject *successor = PyObject_GetAttrString(corel, "successor");
	    PyObject *predecessor = PyObject_GetAttrString(corel, "predecessor");

            EventPtr cpred = create_event(predecessor);
	    ERRNULL(cpred);
	    EventPtr csucc = create_event(successor);
	    ERRNULL(csucc);
	    CoregionAreaPtr csuccarea = create_area(PyObject_GetAttrString(successor, "area"));
            ERRNULL(csuccarea);
	    csucc->set_area(csuccarea);
	    CoregEventRelPtr ccorevrel = CoregEventRelPtr(new CoregionEventRelation(cpred, csucc));
	    cevent->add_successor(ccorevrel);
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

MscPtr ConvPy::create_msc(PyObject *msc){
  MscPtr cmsc = pob.msc.get(msc);
  if(cmsc)
    return cmsc;
  if(PyObject_GetAttrString(msc, "HMsc") == Py_True){
    HMscPtr hmsc = HMscPtr(new HMsc(get_label(msc)));
    pob.msc.add(hmsc, msc);
    return hmsc;
  }
  if(PyObject_GetAttrString(msc, "BMsc") == Py_True){
    BMscPtr bmsc = BMscPtr(new BMsc(get_label(msc)));
    pob.msc.add(bmsc, msc);
    return bmsc;
  }
  DPRINT("DEBUG: Msc created(" << msc << ")");
  return cmsc;
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

HMscNodePtr ConvPy::create_node(PyObject *node)
{
  // Add labels, forgot to do that
  HMscNodePtr cnode = pob.node.get(node);
  if(cnode)
    return cnode;
  if(PyObject_GetAttrString(node, "StartNode") == Py_True){
    StartNodePtr snode = StartNodePtr(new StartNode());
    pob.node.add(snode, node);
    return snode;
  }
  if(PyObject_GetAttrString(node, "ConditionNode") == Py_True){
    ConditionNodePtr cdnode = ConditionNodePtr(new ConditionNode());
    pob.node.add(cdnode, node);
    return cdnode;
  }
  if(PyObject_GetAttrString(node, "ConnectionNode") == Py_True){
    ConnectionNodePtr cnode = ConnectionNodePtr(new ConnectionNode());
    pob.node.add(cnode, node);
    return cnode;
  }
  if(PyObject_GetAttrString(node, "ReferenceNode") == Py_True){
    ReferenceNodePtr rnode = ReferenceNodePtr(new ReferenceNode());
    pob.node.add(rnode, node);
    return rnode;
  }
  if(PyObject_GetAttrString(node, "EndNode") == Py_True){
    EndNodePtr enode = EndNodePtr(new EndNode());
    pob.node.add(enode, node);
    return enode;
  }
  DPRINT("DEBUG: Node created(" << node << ")");
  return cnode;
}

int PyConv::convert_hmsc(PyObject *hmsc){
  DPRINT("Converting HMsc");
  MscPtr chmsc = create_msc(hmsc);
  ERRNULL(chmsc);
  // nodes to be processed; this is to avoid recursion
  std::list<HMscNodePtr> node_stack;

  // initialize the stack with the start node
  push_back_if_unique<HMscNodePtr>(node_stack, hmsc->get_start());

  // Data declaration -> first iteration
  // process all nodes in the stack
  PyObject *lnode = PyObject_GetAttrString(hmsc, "lnodes");
  for(int npos = 0;npos < PyList_Size(lnode);npos++){
    PyObject *node = PyList_GetItem(lnode, npos);
    if(node == NULL){
      DPRINT("# ERROR: BAD HMSC NODE");
      continue;
    }

    HMscNodePtr cnode = create_node(node);
    ERRNULL(cnode);
    StartNodePtr start_node = boost::dynamic_pointer_cast<StartNode>(cnode);
    if(start_node != NULL){
      chmsc->set_start(start_node);
    }

    ConditionNodePtr condition_node = boost::dynamic_pointer_cast<ConditionNode>(cnode);
    if(condition_node != NULL){
      chmsc->add_node(condition_node);
      condition_node->assing_label(get_label(node)); // Only string, not wstring, change it
    }

    ConnectionNodePtr connection_node = boost::dynamic_pointer_cast<ConnectionNode>(cnode);
    if(connection_node != NULL){
      chmsc->add_node(connection_node);
    }

    ReferenceNodePtr reference_node = boost::dynamic_pointer_cast<ReferenceNode>(cnode);
    if(reference_node != NULL){
      chmsc->add_node(reference_node);
      if(PyObject_GetAttrString(node, "msc") != PyNone){
        MscPtr cmsc = create_msc(PyObject_GetAttrString(node, "msc"));
        ERRNULL(cmsc);
	cnode->set_msc(cmsc);
        m_printing.push_back(reference_node->get_msc()); // Deal with this
      }
    }

    EndNodePtr end_node = boost::dynamic_pointer_cast<EndNode>(cnode);
    if(end_node != NULL){
      chmsc->add_node(end_node);
    }

    PyObject *tuple = PyObject_GetAttrString(node, "position");
    if(tuple != PyNone){
      MscPoint mpnt(PyFloat_AsDouble(PyTuple_GetItem(tuple, 0)), PyFloat_AsDouble(PyTuple_GetItem(tuple, 1)));
      cnode->set_position(mpnt);
    }

    // This might be a little problematic
    // Hopefully just a little, very little
    // Leaving for next wake day
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
