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
 *
 */

#ifndef _CONVERT_PYTHON_H
#define _CONVERT_PYTHON_H

#include <Python.h>
#include <vector>
#include <list>
#include <map>
#include <iterator>
#include "data/msc.h"
#include "data/checker.h"
#include "data/pysc/export.h"

struct _cobject_id{
  PyObject *py;
  bool filled;
};
typedef struct _cobject_id cobject_id;

template <class Ptr>
class AdvCPtrIDMap
{
private:
  typedef std::map<Ptr, cobject_id> TPtrIDMapper;
  TPtrIDMapper m_mapper;

public:
  // this function assigns a unique identifier to Ptr objects
  PyObject *pget(Ptr message){
    typename std::map<Ptr, cobject_id>::iterator pos = m_mapper.find(message);
    if(pos != m_mapper.end())
      return (pos->second).py;

    return NULL;
  }

  Ptr get(PyObject *py){
    for(typename std::map<Ptr, cobject_id>::iterator it=m_mapper.begin() ; it != m_mapper.end();it++){
      if((*it).second.py == py)
        return (*it).first;
    }
    return Ptr(NULL);
  }

  void add(Ptr message, PyObject *pyo){
    cobject_id id = {pyo, false};
    m_mapper[message] = id;
  }

  bool is_filled(PyObject *message){
    for(typename std::map<Ptr, cobject_id>::iterator it=m_mapper.begin() ; it != m_mapper.end();it++){
      if((*it).second.py == message && !((*it).second.filled)){
        (*it).second.filled = true;
        return false;
      }
    }
    return true;
  }

  void clear(){
    for(typename std::map<Ptr, cobject_id>::iterator it=m_mapper.begin() ; it != m_mapper.end();it++){
      Py_CLEAR((*it).second.py);
      (*it).second.py = NULL;
    }
  }
  ~AdvCPtrIDMap(){
    clear();
    m_mapper.clear();
  }
};

struct _cobjects{
	AdvCPtrIDMap<MscPtr> msc;
	AdvCPtrIDMap<HMscNodePtr> node;
	AdvCPtrIDMap<InstancePtr> instance;
	AdvCPtrIDMap<EventAreaPtr> area;
	AdvCPtrIDMap<EventPtr> event;
	AdvCPtrIDMap<MscMessagePtr> message;
};
typedef struct _cobjects c_objects;

class SCPYCONV_EXPORT ConvPy{
public:
  c_objects pob;
  ConvPy(){
  };
  ~ConvPy();

  MscPtr convert_msc(PyObject *selected_msc);

protected:
  int typed_msc(PyObject *msc);

  std::list<PyObject *> m_printing;

  //! export a basic MSC drawing
  int convert_bmsc(PyObject *bmsc);
  //! export a HMSC drawing
  int convert_hmsc(PyObject *hmsc);

  void handle_event(PyObject *event, EventPtr cevent);

  InstancePtr create_instance(PyObject *inst);

  EventAreaPtr create_area(PyObject *area);

  EventPtr create_event(PyObject *event);

  MscMessagePtr create_message(PyObject *message);

  MscPtr create_msc(PyObject *msc);

  HMscNodePtr create_node(PyObject *node);
};

#endif /* _CONVERT_PYTHON_H */
