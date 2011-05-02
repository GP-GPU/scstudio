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

#ifndef _PYTHON_CONVERT_H
#define _PYTHON_CONVERT_H

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
	PyObject *name;
	PyObject *module;
	PyObject *pDict;
	PyObject *funcBMsc;
	PyObject *funcHMsc;
};
typedef struct _objects py_objects;

typedef boost::intrusive_ptr<PyObject> PyObjectPtr;

class SCPYCONV_EXPORT PyConv{
public:
  py_objects pob;
  PyConv(const char *module){
    if(!init(module))
      throw -1;
  };
  ~PyConv();

  int init(const char *module);
  int reinit(const char *module);
  int check(const MscPtr& msc, const ChannelMapperPtr& chm);
  std::list<BMscPtr> checkBMsc(const BMscPtr& bmsc, const ChannelMapperPtr& chm);
  std::list<HMscPtr> checkHMsc(const HMscPtr& hmsc, const ChannelMapperPtr& chm);
  PyObject* convert_msc(const MscPtr& selected_msc, const std::vector<MscPtr>& msc = std::vector<MscPtr>());

protected:
  int typed_msc(const MscPtr& msc);
  // note: insertion to m_printing must not invalidate iterators
  std::list<MscPtr> m_printing;

  //! export a basic MSC drawing
  int convert_bmsc(const BMscPtr& bmsc);
  //! export a HMSC drawing
  int convert_hmsc(const HMscPtr& hmsc);

  void handle_event(const EventPtr& event, PyObject *pevent);

  PyObject *create_instance(const InstancePtr& inst);

  const char *get_area_type(const EventAreaPtr& msc);
  PyObject *create_area(const EventAreaPtr& area, const char *optype = "");

  const char *get_event_type(const EventPtr& msc);
  PyObject *create_event(const EventPtr& event, const char *optype = "");

  const char *get_message_type(const MscMessagePtr& message);
  PyObject *create_message(const MscMessagePtr& message, const char *optype = "");

  const char *get_msc_type(const MscPtr& msc);
  PyObject *create_msc(const MscPtr& msc, const char * type = "");

  const char *get_node_type(const HMscNodePtr& node);
  PyObject *create_node(const HMscNodePtr& node, const char *type = "");

  const char *get_chm_type(const ChannelMapperPtr& chm);
  PyObject *create_chm(const ChannelMapperPtr& chm);
};

#endif /* _PYTHON_CONVERT_H */
