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

struct _object_id{
  PyObject *py;
  bool filled;
};
typedef struct _object_id object_id;

template <class Ptr>
class AdvPtrIDMap
{
private:
  typedef std::map<Ptr, object_id> TPtrIDMapper;
  TPtrIDMapper m_mapper;

public:
  // this function assigns a unique identifier to Ptr objects
  PyObject *pget(const Ptr& message){
    typename std::map<Ptr, object_id>::iterator pos = m_mapper.find(message);
    if(pos != m_mapper.end())
      return (pos->second).py;

    return NULL;
  }

  const Ptr& get(PyObject *py){
    for(typename std::map<Ptr, object_id>::iterator it=m_mapper.begin() ; it != m_mapper.end();it++){
      if((*it).second.py == py)
        return (*it).first;
    }
    throw 0;
    //return (Ptr)NULL;
  }

  void add(const Ptr& message, PyObject *pyo){
    object_id id = {pyo, false};
    m_mapper[message] = id;
  }

  bool is_filled(const Ptr& message){
    typename std::map<Ptr, object_id>::iterator pos = m_mapper.find(message);
    if(pos != m_mapper.end() && !((pos->second).filled)){
      (pos->second).filled = true;
      return false;
    }
    return true;
  }

  void clear(){
    for(typename std::map<Ptr, object_id>::iterator it=m_mapper.begin() ; it != m_mapper.end();it++){
      Py_CLEAR((*it).second.py);
      (*it).second.py = NULL;
    }
  }
  ~AdvPtrIDMap(){
    clear();
    m_mapper.clear();
  }
};

struct _objects{
	AdvPtrIDMap<MscPtr> msc;
	AdvPtrIDMap<HMscNodePtr> node;
	AdvPtrIDMap<InstancePtr> instance;
	AdvPtrIDMap<EventAreaPtr> area;
	AdvPtrIDMap<EventPtr> event;
	AdvPtrIDMap<MscMessagePtr> message;
};
typedef struct _objects py_objects;

typedef boost::intrusive_ptr<PyObject> PyObjectPtr;

class SCPYCONV_EXPORT ConvPy{
public:
  py_objects pob;
  ConvPy(){
  };
  ~ConvPy();

  //int check(const MscPtr& msc, const ChannelMapperPtr& chm);
  //std::list<BMscPtr> checkBMsc(const BMscPtr& bmsc, const ChannelMapperPtr& chm);
  //std::list<HMscPtr> checkHMsc(const HMscPtr& hmsc, const ChannelMapperPtr& chm);
  MscPtr convert_msc(PyObject *selected_msc);

protected:
  int typed_msc(PyObject *msc);
  // note: insertion to m_printing must not invalidate iterators
  std::list<MscPtr> m_printing;

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
