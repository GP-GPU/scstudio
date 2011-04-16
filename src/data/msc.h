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
 * Copyright (c) 2008 Jindra Babica <babica@mail.muni.cz>
 *
 * $Id: msc.h 1037 2011-02-09 19:47:01Z madzin $
 */

#ifndef _MSC_H
#define _MSC_H

#include <list>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <vector>
#include <stdexcept>
#include <cmath>

// we use boost::intrusive_ptr as we need to construct the xx_ptr<T> from T*
// see http://www.boost.org/doc/libs/1_37_0/libs/smart_ptr/smart_ptr.htm
#include <boost/intrusive_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION > 103500
#include <boost/interprocess/detail/atomic.hpp>
#endif

#include "data/msc_types.h"
#include "data/time.h"
#include "data/export.h"

class MscElement;
class Comment;
class Commentable;
class HMscNode;
class InnerNode;
class ReferenceNode;
class ConditionNode;
class ConnectionNode;
class EndNode;
class StartNode;
class Instance;
class Event;
class StrictEvent;
class CoregionEvent;
class Msc;
class BMsc;
class HMsc;
class MscMessage;
class EventArea;
class StrictOrderArea;
class CoregionArea;
class SuccessorNode;
class PredecessorNode;
#ifdef _TIME_H_
class TimeRelation;
class TimeRelationEvent;
class TimeRelationRefNode;
#endif


typedef boost::intrusive_ptr<MscElement> MscElementPtr;

typedef boost::intrusive_ptr<Comment> CommentPtr;
typedef std::set<CommentPtr> CommentPtrSet;
typedef boost::intrusive_ptr<Commentable> CommentablePtr;

typedef boost::intrusive_ptr<Msc> MscPtr;

typedef boost::intrusive_ptr<BMsc> BMscPtr;

typedef boost::intrusive_ptr<HMsc> HMscPtr;

typedef boost::intrusive_ptr<HMscNode> HMscNodePtr;
typedef std::set<HMscNodePtr> HMscNodePtrSet;

typedef boost::intrusive_ptr<ReferenceNode> ReferenceNodePtr;
typedef std::list<ReferenceNodePtr> ReferenceNodePtrList;

typedef std::set<InnerNode*> InnerNodePSet;
typedef std::set<ReferenceNode*> ReferenceNodePSet;
typedef std::queue<InnerNode*> InnerNodePQueue;

typedef boost::intrusive_ptr<ConditionNode> ConditionNodePtr;

typedef boost::intrusive_ptr<ConnectionNode> ConnectionNodePtr;

typedef boost::intrusive_ptr<StartNode> StartNodePtr;

typedef boost::intrusive_ptr<EndNode> EndNodePtr;

typedef boost::intrusive_ptr<Instance> InstancePtr;
typedef std::list<InstancePtr> InstancePtrList;

typedef boost::intrusive_ptr<Event> EventPtr;
typedef std::list<EventPtr> EventPtrList;

typedef boost::intrusive_ptr<StrictEvent> StrictEventPtr;
typedef std::list<StrictEventPtr> StrictEventPtrList;

typedef boost::intrusive_ptr<CoregionEvent> CoregionEventPtr;
typedef std::list<CoregionEventPtr> CoregionEventPtrList;
typedef std::set<CoregionEventPtr> CoregionEventPtrSet;
typedef std::vector<CoregionEvent*> CoregionEventPVector;
typedef std::list<CoregionEvent*> CoregionEventPList;
typedef std::queue<CoregionEvent*> CoregionEventPQueue;

typedef boost::intrusive_ptr<MscMessage> MscMessagePtr;

typedef boost::intrusive_ptr<EventArea> EventAreaPtr;
typedef std::list<EventAreaPtr> EventAreaPtrList;

typedef boost::intrusive_ptr<StrictOrderArea> StrictOrderAreaPtr;

typedef boost::intrusive_ptr<CoregionArea> CoregionAreaPtr;

typedef void* AttributeP;
typedef std::map<std::string,AttributeP> AttributePMap;

typedef std::set<HMscNodePtr> HMscNodePtrSet;

#ifdef _TIME_H_

typedef boost::intrusive_ptr<TimeRelation> TimeRelationPtr;
typedef boost::intrusive_ptr<TimeRelationEvent> TimeRelationEventPtr;
typedef std::list<TimeRelationEventPtr> TimeRelationEventPtrList;
typedef boost::intrusive_ptr<TimeRelationRefNode> TimeRelationRefNodePtr;
typedef std::set<TimeRelationRefNodePtr> TimeRelationRefNodePtrSet;
typedef MscTimeIntervalSet<double> MscTimeIntervalSetD;
typedef std::list<MscTimeIntervalSetD> MscTimeIntervalSetDList;

#endif

enum MarkType {NONE, MARKED, ADDED, REMOVED};

/**
 * \brief Common basic abstract class for all elements of MSC
 */
class SCMSC_EXPORT MscElement
{
  /**
   * \brief Atributes of MscElement which may be dynamically created.
   *
   * It is neccessary to ensure to remove any attribute if the attribute is no
   * more required. These attributes are mostly used in checking algorithms.
   *
   * @warning individual attributes are of type AttributeP, i.e. type control
   * is left up to piece of code using these attributes
   */
  AttributePMap m_attributes;

  //! Determines whether this element is in some way interesting for creator.
  enum MarkType m_marked;

protected:

  MscElement()
  {
    m_marked = NONE;
    m_counter = 0;
  }

public:

  /**
   * \brief Finds out whether attribute with specified name is set or not.
   *
   * Keep in mind that this method hasn't got constant complexity but O(log n)
   * where n=number of dynamic attributes set to this element.
   *
   * @param name - name of attribute
   * @returns true if attribute is set false otherwise
   */
  bool is_attribute_set(const std::string& name) const
  {
      return (m_attributes.find(name)!=m_attributes.end());
  }

  /**
   * \brief Returns dynamic attribute of MscElement.
   *
   * If attribute is not set, value of def is copied into its value and is
   * returned. T is type of attribute.
   *
   * Keep in mind that this method hasn't got constant complexity but O(log n)
   * where n=number of dynamic attributes set to this element.
   *
   * @param name - attribute name
   * @param def - default value of attribute
   */
  template<class T>
  T& get_attribute(const std::string& name, const T& def)
  {
    AttributePMap::iterator i = m_attributes.find(name);
    T* a;
    if(i!=m_attributes.end())
    {
      a = static_cast<T*>(i->second);
    }else{
      //insert attribute with default value
      a = new T;
      *a = def;
      m_attributes[name] = a;
    }
    return *a;
  }

  /**
   * \brief Returns dynamic attribute of MscElement.
   *
   * If attribute is not set, value of def is copied into its value and is
   * returned. T is type of attribute.
   *
   * Keep in mind that this method hasn't got constant complexity but O(log n)
   * where n=number of dynamic attributes set to this element.
   *
   * @param name - attribute name
   * @param def - default value of attribute
   * @param status - set to true if attribute was just created, to false
   * otherwise
   */
  template<class T>
  T& get_attribute(const std::string& name, const T& def, bool& status)
  {
    AttributePMap::iterator i = m_attributes.find(name);
    T* a;
    if(i!=m_attributes.end())
    {
      a = static_cast<T*>(i->second);
      status = false;
    }else{
      //insert attribute with default value
      a = new T;
      *a = def;
      m_attributes[name] = a;
      status = true;
    }
    return *a;
  }

  /**
   * \brief Sets dynamic attribute of MscElement.
   *
   * Value of def is copied as value of this attribute. T is type of attribute.
   *
   * Keep in mind that this method hasn't got constant complexity but O(log n)
   * where n=number of dynamic attributes set to this element.
   *
   * @param name - attribute name
   * @param val - value of attribute to be set
   */
  template<class T>
  void set_attribute(const std::string& name, const T& val)
  {
    T& a = get_attribute<T>(name,val);
    a = val;
  }

  /**
   * \brief Removes attribute of MscElement.
   *
   * Keep in mind that this method hasn't got constant complexity but O(log n)
   * where n=number of dynamic attributes set to this element.
   *
   * Removes attribute with specified name from m_attributes map and deletes it
   * from memory too. T is type of attribute.
   *
   * @param name - attribute name
   */
  template<class T>
  void remove_attribute(const std::string& name)
  {
    AttributePMap::iterator i = m_attributes.find(name);
    if(i!=m_attributes.end())
    {
      delete static_cast<T*>(i->second);
      m_attributes.erase(i);
    }
  }

  /**
   * \brief List all attributes of MscElement.
   */
  std::set<std::string> get_attribute_names() const
  {
    std::set<std::string> result;

    for(AttributePMap::const_iterator i = m_attributes.begin();
      i != m_attributes.end(); i++)
    {
      result.insert(i->first);
    }

    return result;
  }

  enum MarkType get_marked() const
  {
    return m_marked;
  }

  void set_marked(enum MarkType marked=MARKED)
  {
    m_marked = marked;
  }

  /**
   * \brief See MscElementTmpl for details about attribute original
   */
  virtual MscElement* get_general_original() const =0;

  /**
   * \brief See MscElementTmpl for details about attribute original
   *
   * Implementation of this method can throw bad cast exception
   * in case original is not of appropriate type
   */
  virtual void set_general_original(MscElement* original)=0;

  virtual ~MscElement()
  {
  }

private:
  //! Number of references to this object.
  mutable volatile boost::uint32_t m_counter;

  // see http://www.boost.org/doc/libs/1_37_0/libs/smart_ptr/intrusive_ptr.html
  friend void intrusive_ptr_add_ref(const MscElement *ptr);
  friend void intrusive_ptr_release(const MscElement *ptr);
};

inline void intrusive_ptr_add_ref(const MscElement *ptr)
{
  if(ptr != NULL)
#if BOOST_VERSION > 103500
    boost::interprocess::detail::atomic_inc32(&ptr->m_counter);
#else
    ++ptr->m_counter;
#endif
}

inline void intrusive_ptr_release(const MscElement *ptr)
{
  if(ptr != NULL)
  {
#if BOOST_VERSION > 103500
    // note, atomic_dec32() returns the old value pointed to by mem
    if(boost::interprocess::detail::atomic_dec32(&ptr->m_counter) == 1)
#else
    if(ptr->m_counter-- == 1)
#endif
      // the last pointer is being released, delete the objet
      delete ptr;
  }
}

template <class T>
class /*SCMSC_EXPORT*/ MscElementTmpl : public MscElement
{
protected:

  //! Visual style of the shape.
  int m_visual_style;

  /**
   * \brief Pointer to original version of MscElement.
   *
   * Checking algorithms returns as their result violating example of BMsc or
   * path in HMsc. To show exact violating example they must often change the
   * original version of BMsc or HMsc. However, it is neccessary to keep
   * original unmodified version, therefore they must create copy of the
   * structure that violated some property.
   *
   * Moreover it is neccessary to keep relationship between original MscElement
   * and the newly created one. Therefore algorithms keep this relation in this
   * attribute.
   */
  boost::intrusive_ptr<T> m_original;

  MscElementTmpl<T>() : MscElement()
  {
    m_visual_style = 0;
    m_original = NULL;
  }

  /**
   * \brief Creates MscElement referencing the original one.
   *
   * Used to create counter example with reference to original element which
   * should be accessible from the new one.
   */
  MscElementTmpl<T>(T* original) : MscElement()
  {
    m_visual_style = original->m_visual_style;
    m_original = original;
    this->set_marked(original->get_marked());
  }

public:

  int get_visual_style() const
  {
    return m_visual_style;
  }

  void set_visual_style(int visual_style)
  {
    m_visual_style = visual_style;
  }

  MscElement* get_general_original() const
  {
    return m_original.get();
  }

  void set_general_original(MscElement* original)
  {
    T* orig = dynamic_cast<T*>(original);
    if(orig || !original)
    {
      set_original(orig);
    }
    else
    {
      throw std::invalid_argument("Attribute m_original is not of desired type");
    }
  }

  /**
   * Getter for m_original.
   */
  T* get_original() const
  {
    return m_original.get();
  }

  /**
   * Setter for m_original.
   */
  void set_original(T* e)
  {
    m_original = e;
  }

  /**
   * Setter for m_original.
   */
  void set_original(boost::intrusive_ptr<T>& e)
  {
    m_original = e;
  }

  virtual ~MscElementTmpl()
  {
  }
};

/**
 * \brief Represents a text block or a text comment in MSC.
 */
class SCMSC_EXPORT Comment : public MscElementTmpl<Comment>
{
protected:
  std::wstring m_text;

  MscPoint m_position;
  Coordinate m_width;

public:
  Comment(const std::wstring& text=L"") :
    m_text(text), m_position(), m_width(20)
  {
  }

  void set_text(const std::wstring& text)
  {
    m_text = text;
  }

  const std::wstring& get_text() const
  {
    return m_text;
  }

  void set_position(const MscPoint& position)
  {
    m_position = position;
  }

  const MscPoint& get_position() const
  {
    return m_position;
  }

  void set_width(Coordinate width)
  {
    m_width = width;
  }

  Coordinate get_width() const
  {
    return m_width;
  }
};

/**
 * \brief Represents a class that may reference text comments.
 */
class SCMSC_EXPORT Commentable
{
protected:
  CommentPtrSet m_comments;

public:
  virtual ~Commentable()
  {
  }

  void add_comment(CommentPtr comment)
  {
    std::pair<CommentPtrSet::iterator, bool> res =
      m_comments.insert(comment);

    if(!res.second)
      throw std::invalid_argument("Comment already connected");
  }

  const CommentPtrSet& get_comments() const
  {
    return m_comments;
  }
};

// An empty structure used by extern "C" pointers to Msc
struct s_Msc { };

/**
 * \brief Represents virtual base class for BMsc and HMsc.
 */
class SCMSC_EXPORT Msc :
  public s_Msc, public MscElementTmpl<Msc>, public Commentable
{
protected:

  /**
   * \brief Label of MSC.
   */
  std::wstring m_label;

  Msc(const std::wstring& label=L""):m_label(label)
  {
  }

  Msc(Msc* original):MscElementTmpl<Msc>(original)
  {
    if(original)
    {
      m_label = original->get_label();
    }
  }

public:

  virtual ~Msc()
  {
  }

  /**
   * Getter for m_label
   */
  void set_label(const std::wstring& label)
  {
    m_label = label;
  }

  /**
   * Setter for m_label
   */
  const std::wstring& get_label() const
  {
    return m_label;
  }

};

/**
 * \brief Represents Basic MSC.
 */
class SCMSC_EXPORT BMsc:public Msc
{

  /**
   * Processes' instances which are contained in BMsc.
   */
  InstancePtrList m_instances;

public:

  /**
   * Initializes m_label
   */
  BMsc(const std::wstring& label=L""):Msc(label)
  {
  }

  BMsc(BMsc* original):Msc(original)
  {
  }

  virtual ~BMsc()
  {
  }

  /**
   * Adds instance.
   */
  void add_instance(InstancePtr& i);

  /**
   * Getter for m_instances.
   */
  const InstancePtrList& get_instances() const
  {
    return m_instances;
  }
  /**
   * Removes all the instances with a specified name.
   */
  void remove_instances(const std::wstring &label);
};

/**
 * \brief Base abstract class for node of HMsc
 */
class SCMSC_EXPORT HMscNode:
  public MscElementTmpl<HMscNode>, public Commentable
{
  MscPoint m_position;

protected:

  HMsc* m_owner;

  HMscNode() : MscElementTmpl<HMscNode>(),
    m_position(),
    m_owner(NULL)
  {
  }

  HMscNode(HMscNode* original) : MscElementTmpl<HMscNode>(original),
    m_position(original->get_position()),
    m_owner(NULL)
  {
  }

public:

  HMsc* get_owner() const
  {
    return m_owner;
  }

  void set_owner(HMsc* owner)
  {
    m_owner = owner;
  }

  const MscPoint& get_position() const
  {
    return m_position;
  }

  void set_position(const MscPoint& position)
  {
    m_position = position;
  }

  HMscNodePtr my_ptr();

  virtual ~HMscNode()
  {
  }

};

class SCMSC_EXPORT NodeRelation:public MscElementTmpl<NodeRelation>
{
protected:

  PolyLine m_line;

  SuccessorNode* m_successor;
  PredecessorNode* m_predecessor;

public:

  NodeRelation() : MscElementTmpl<NodeRelation>()
  {
    m_successor = NULL;
    m_predecessor = NULL;
  }

  SuccessorNode* get_successor() const
  {
    return m_successor;
  }

  PredecessorNode* get_predecessor() const
  {
    return m_predecessor;
  }

  /**
   * Set the successor.
   * Connects to a given SuccessorNode and re-configures its back pointer.
   */
  void set_successor(SuccessorNode* succ);

  /**
   * Set the predecessor.
   * Connects to a given PredecessorNode and re-configures its back pointer.
   */
  void set_predecessor(PredecessorNode* pred);

  void set_line(const PolyLine& line);

  const PolyLine& get_line() const;
};

typedef boost::intrusive_ptr<NodeRelation> NodeRelationPtr;
typedef std::vector<NodeRelationPtr> NodeRelationPtrVector;

class SCMSC_EXPORT SuccessorNode
{
protected:

  //! Predecessors of the SuccessorNode.
  // note: vector is used to reduce overhead and preserve the insertion order
  NodeRelationPtrVector m_predecessors;

  public:

  SuccessorNode()
  {
  }

  virtual ~SuccessorNode()
  {
  }

  /**
   * Getter for m_predecessors.
   */
  const NodeRelationPtrVector& get_predecessors() const
  {
    return m_predecessors;
  }

  /**
   * Adds a PredecessorNode.
   * This function automatically creates a new NodeRelation link
   * to the PredecessorNode.
   */
  NodeRelationPtr add_predecessor(PredecessorNode* pred)
  {
    NodeRelationPtr n(new NodeRelation());
    n->set_predecessor(pred);
    n->set_successor(this);
    return n;
  }

  void remove_predecessor(const NodeRelationPtr& n);

  void remove_predecessors();

  /**
   * Returns true iff node has any successors
   */
  bool has_predecessors() const
  {
    return m_predecessors.size()!=0;
  }

  friend class NodeRelation;
  friend class PredecessorNode;
};

class SCMSC_EXPORT PredecessorNode
{
protected:

  //! Succesors of the PredecessorNode.
  // note: vector is used to reduce overhead and preserve the insertion order
  NodeRelationPtrVector m_successors;

public:

  PredecessorNode()
  {
  }

  virtual ~PredecessorNode()
  {
  }

  /**
   * Getter for m_successors.
   */
  const NodeRelationPtrVector& get_successors() const
  {
    return m_successors;
  }

  /**
   * Adds a SuccessorNode.
   * This function automatically creates a new NodeRelation link
   * to the SuccessorNode.
   */
  NodeRelationPtr add_successor(SuccessorNode* succ)
  {
    NodeRelationPtr n(new NodeRelation());
    n->set_predecessor(this);
    n->set_successor(succ);
    return n;
  }

  /**
   * Removes successor.
   */
  void remove_successor(const NodeRelationPtr& n);

  void remove_successors();

  /**
   * Returns true iff node has any successors
   */
  bool has_successors() const
  {
    return m_successors.size()!=0;
  }

  friend class NodeRelation;
  friend class SuccessorNode;
};

/**
 * \brief End node of HMsc.
 *
 * Represents end node as specified in Z.120
 */
class SCMSC_EXPORT EndNode:
  public HMscNode, public SuccessorNode
{
public:

  EndNode() : HMscNode(), SuccessorNode()
  {}

  EndNode(EndNode* original) : HMscNode(original), SuccessorNode()
  {}

  virtual ~EndNode()
  {
  }
};

/**
 * \brief Start node of HMsc.
 *
 * According to ITU-T standard, each HMsc has got exactly
 * one start node.
 */
class SCMSC_EXPORT StartNode:
  public HMscNode, public PredecessorNode
{
public:

  StartNode() : HMscNode(), PredecessorNode()
  {
  }

  StartNode(StartNode* original) : HMscNode(original), PredecessorNode()
  {
  }

  virtual ~StartNode()
  {
  }
};

/**
 * \brief Represents High-level MSC.
 */
class SCMSC_EXPORT HMsc:public Msc
{
protected:
  /**
   * \brief Start node of HMsc.
   *
   * Mandatory element in HMsc, not included in m_nodes.
   */
  StartNodePtr m_start;

  /**
   * Set of all HMscNodes included in this HMsc
   */
  HMscNodePtrSet m_nodes;

public:

  HMsc(const std::wstring& label=L""):Msc(label)
  {
  }

  HMsc(HMsc* original):Msc(original)
  {
  }

  virtual ~HMsc()
  {
  }

  /**
   * Getter for m_start.
   */
  const StartNodePtr& get_start() const
  {
    return m_start;
  }

  void set_start(const StartNodePtr& start)
  {
    m_start = start;
    m_start->set_owner(this);
  }

  const HMscNodePtrSet& get_nodes() const
  {
    return m_nodes;
  }

  void add_node(HMscNodePtr node);

  void remove_node(HMscNode* node);

  void remove_node(HMscNodePtr node);

  HMscNodePtr find_ptr(HMscNode* node) const
  {
    HMscNodePtrSet::const_iterator n;
    for(n=m_nodes.begin();n!=m_nodes.end();n++)
    {
      if((*n).get()==node) return *n;
    }
    HMscNodePtr a;
    return a;
  }

};

/**
 * \brief HMscNode which references either BMsc or HMsc.
 *
 * Represents reference node in Z.120.
 */
class SCMSC_EXPORT ReferenceNode:
  public HMscNode, public PredecessorNode, public SuccessorNode
{
private:

  /**
   * MSC which is contained in this node.
   */
  MscPtr m_msc;

#ifdef _TIME_H_
  TimeRelationRefNodePtrSet m_time_relation_set_top;
  TimeRelationRefNodePtrSet m_time_relation_set_bottom;

#endif

public:

  ReferenceNode() : HMscNode(), PredecessorNode(), SuccessorNode()
  {
  }

  /**
   * Initializes m_original
   */
  ReferenceNode(ReferenceNode* original) : HMscNode(original), PredecessorNode(), SuccessorNode()
  {
  }

  virtual ~ReferenceNode()
  {
  }


  /**
   * Getter for m_msc.
   */
  const MscPtr& get_msc() const
  {
    return m_msc;
  }

  /**
   * Setter for m_msc.
   */
  void set_msc(const MscPtr& msc)
  {
    m_msc = msc;
  }

  /**
   * \brief Tries to overcast m_msc into HMscPtr.
   *
   * If not successfull undefined HMscPtr is returned.
   */
  HMscPtr get_hmsc() const
  {
    return boost::dynamic_pointer_cast<HMsc>(m_msc);
  }

  /**
   * \brief Tries to overcast m_msc into BMscPtr.
   *
   * If not successfull undefined BMscPtr is returned.
   */
  BMscPtr get_bmsc() const
  {
    return boost::dynamic_pointer_cast<BMsc>(m_msc);
  }

#ifdef _TIME_H_

//TODO ask Ondra why I cannot do this
//   void empty_time_relations(){
//     TimeRelationRefNodePtrSet::iterator it;
//     for(it=m_time_relation_set_top.begin();it!=m_time_relation_set_top.end();it++){
//       it->get()->clear_interval_set();
//     }
//     for(it=m_time_relation_set_bottom.begin();it!=m_time_relation_set_bottom.end();it++){
//       it->get()->clear_interval_set();
//     }
//   }

  void add_time_relation_top(TimeRelationRefNodePtr relation)
  {
    std::pair<TimeRelationRefNodePtrSet::iterator, bool> res =
      m_time_relation_set_top.insert(relation);

    if(!res.second)
      throw std::invalid_argument("Time relation already connected");
  }

  bool is_time_relation_top(const TimeRelationRefNodePtr& relation) const
  {
    return m_time_relation_set_top.find(relation) != m_time_relation_set_top.end();
  }

  void remove_time_relation_top(TimeRelationRefNodePtr relation)
  {
    m_time_relation_set_top.erase(m_time_relation_set_top.find(relation));
  }

  void add_time_relation_bottom(TimeRelationRefNodePtr relation)
  {
    std::pair<TimeRelationRefNodePtrSet::iterator, bool> res =
      m_time_relation_set_bottom.insert(relation);

    if(!res.second)
      throw std::invalid_argument("Time relation already connected");
  }

  bool is_time_relation_bottom(const TimeRelationRefNodePtr& relation) const
  {
    return m_time_relation_set_bottom.find(relation) != m_time_relation_set_bottom.end();
  }

  void remove_time_relation_bottom(TimeRelationRefNodePtr relation)
  {
    m_time_relation_set_bottom.erase(m_time_relation_set_bottom.find(relation));
  }

  const TimeRelationRefNodePtrSet& get_time_relations_top() const
  {
    return m_time_relation_set_top;
  }

  TimeRelationRefNodePtrSet& get_time_relations_top()
  {
    return m_time_relation_set_top;
  }

  const TimeRelationRefNodePtrSet& get_time_relations_bottom() const
  {
    return m_time_relation_set_bottom;
  }

  TimeRelationRefNodePtrSet& get_time_relations_bottom()
  {
    return m_time_relation_set_bottom;
  }
#endif

};

/**
 * \brief HMSC condition node.
 *
 * Represents an HMSC condition.
 */
class SCMSC_EXPORT ConditionNode:
  public HMscNode, public PredecessorNode, public SuccessorNode
{
public:
  ConditionNode():HMscNode(),PredecessorNode(),SuccessorNode(),
    m_type(OTHERWISE), m_probability(1.0)
  {
  }

  ConditionNode(const std::string& label):HMscNode(),PredecessorNode(),SuccessorNode(),
    m_type(OTHERWISE), m_probability(1.0)
  {
    assign_label(label);
  }

  /**
   * ConditionNode is allowed to reference via m_original different types of
   * HMscNode.
   */
  ConditionNode(ConditionNode* original):HMscNode(original),PredecessorNode(),SuccessorNode()
  {
    if(original)
    {
      m_type = original->m_type;
      m_names = original->m_names;
      m_probability = original->m_probability;
    }
  }

  virtual ~ConditionNode()
  {
  }

  void assign_label(const std::string& label);
  std::string get_label() const;

  enum ConditionType
  {
    SETTING,  // <state1>, <state2>
    GUARDING, // when <state1>, <state2>
    RANDOM,   // for 50%
    OTHERWISE
  };

  //! Setter of m_type
  void set_type(ConditionType type)
  {
    m_type = type;
  }

  //! Getter of m_type
  ConditionType get_type() const
  {
    return m_type;
  }

  //! Setter of m_names
  void add_name(const std::string& name)
  {
    m_names.push_back(name);
  }

  //! Getter of m_states
  const std::vector<std::string>& get_names() const
  {
    return m_names;
  }

  //! Setter of m_probability
  void set_probability(double probability)
  {
    m_probability = probability;
  }

  //! Getter of m_probability
  double get_probability() const
  {
    return m_probability;
  }

private:
  ConditionType m_type;

  std::vector<std::string> m_names;
  //! probability (0-1) this condition is applied
  double m_probability;
};

/**
 * \brief Represents connection node, empty visual node)
 *
 * Corresponds to ITU-T's connection node.
 */
class SCMSC_EXPORT ConnectionNode:
  public HMscNode, public PredecessorNode, public SuccessorNode
{
public:

  ConnectionNode() : HMscNode(), PredecessorNode(), SuccessorNode()
  {
  }

  /**
   * ConnectionNode is allowed to reference via m_original different types of
   * HMscNode.
   */
  ConnectionNode(HMscNode* original) : HMscNode(original), PredecessorNode(), SuccessorNode()
  {
  }

  virtual ~ConnectionNode()
  {
  }
};

/**
 * \brief Represents process (vertical line) in Basic MSC.
 *
 * There are EventAreas at Instance organized as bidirectional linked list.
 * There isn't used std::list or something similar because it is necessary
 * to have next EventArea accessible from the previous one.
 */
class SCMSC_EXPORT Instance:public MscElementTmpl<Instance>
{

protected:

  /**
   * Label of instance -- name of concrete instance
   */
  std::wstring m_label;

  /**
   * Kind of instance -- name of particular type of instance
   */
  std::wstring m_kind;

  /**
   * Form of instance axis (line/column)
   */
  InstanceAxisForm m_form;

  /**
   * EventAreas which occure at instance as first one.
   */
  EventAreaPtr m_first;

  /**
   * EventAreas which occure at instance as last one.
   */
  EventAreaPtr m_last;

  /**
   * BMsc which this instance belongs to
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  BMsc* m_bmsc;

  MscPoint m_line_begin;
  MscPoint m_line_end;

  Size m_width;

  bool any_event(EventAreaPtr area);

public:

  /**
   * @param label - label of instance
   */
  Instance(const std::wstring& label, const std::wstring& kind=L""):
    MscElementTmpl<Instance>(),
    m_label(label),
    m_kind(kind),
    m_width(10)
  {
  }

  Instance(Instance* original) : MscElementTmpl<Instance>(original),
    m_label(original->get_label()),
    m_kind(original->get_label()),
    m_form(original->get_form()),
    m_line_begin(original->get_line_begin()),
    m_line_end(original->get_line_end()),
    m_width(original->get_width())
  {
  }

  virtual ~Instance()
  {
  }

  InstanceAxisForm get_form() const
  {
    return m_form;
  }

  /**
   * Set the first EventArea
   */
  void set_first(const EventAreaPtr& a)
  {
    m_first = a;
  }

  /**
   * Getter for m_first.
   */
  const EventAreaPtr& get_first() const
  {
    return m_first;
  }

  /**
   * Getter for m_label.
   */
  const std::wstring& get_label() const
  {
    return m_label;
  }

 void set_label(const std::wstring& label)
  {
    m_label = label;
  }

  void set_last(const EventAreaPtr& a)
  {
    m_last = a;
  }

  const EventAreaPtr& get_last() const
  {
    return m_last;
  }

  /**
   * Getter for m_bmsc
   */
  BMsc* get_bmsc() const
  {
    return m_bmsc;
  }

  void set_bmsc(BMsc* bmsc)
  {
    m_bmsc = bmsc;
  }

  const MscPoint& get_line_begin() const
  {
    return m_line_begin;
  }

  void set_line_begin(const MscPoint& line_begin)
  {
    m_line_begin = line_begin;
  }

  const MscPoint& get_line_end() const
  {
    return m_line_end;
  }

  void set_line_end(const MscPoint& line_end)
  {
    m_line_end = line_end;
  }

  Size get_height() const
  {
    Size dx = m_line_end.get_x() - m_line_begin.get_x();
    Size dy = m_line_end.get_y() - m_line_begin.get_y();
    return sqrt(dx*dx + dy*dy);
  }

  Size get_width() const
  {
    return m_width;
  }

  void set_width(const Size& width)
  {
    m_width = width;
  }

  void add_area(EventAreaPtr area);

  bool is_empty() const
  {
    return !m_last.get();
  }

  /** \brief Returns true iff the instance contains at least one event
   */
  bool has_events()
  {
    return any_event(this->get_first());
  }

};


/**
 * \brief Message sent by Instances.
 */
class SCMSC_EXPORT MscMessage:public MscElementTmpl<MscMessage>
{
  /**
   * Label of message.
   */
  std::wstring m_label;

public:

  /**
   * @param sender - sending instance
   * @param receiver - receiving instance
   * @param label - label of message
   */
  MscMessage(const std::wstring& label=L"");

  MscMessage(MscMessage* original);

  virtual ~MscMessage();

  const std::wstring& get_label() const
  {
    return m_label;
  }

  void set_label(const std::wstring& label)
  {
    m_label = label;
  }

  /**
   * Determines if the message is correctly glued to events.
   */
  virtual bool is_glued() const = 0;
};

class SCMSC_EXPORT CompleteMessage:public MscMessage
{

  /**
   * Sender of message.
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  Event* m_send_event;

  /**
   * Receiver of message.
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  Event* m_receive_event;

public:

  /**
   * @param label - label of message
   */
  CompleteMessage(const std::wstring& label=L"")
    : MscMessage(label)
  {
    m_send_event = NULL;
    m_receive_event = NULL;
  }

  CompleteMessage(Event* sender, Event* receiver, MscMessage* original);

  /**
   * Retrieves instance of m_send_event
   */
  Instance* get_sender() const;

  /**
   * Getter for m_send_event.
   */
  Event* get_send_event() const
  {
    return m_send_event;
  }

  /**
   * Connects to a given send event and re-configures its back-pointers.
   * To un-glue, use NULL as the parameter.
   */
  void glue_send_event(const EventPtr& send_event);

  /**
   * Retrieves instance of m_receive_event
   */
  Instance* get_receiver() const;

  /**
   * Getter for m_receive_event.
   */
  Event* get_receive_event() const
  {
    return m_receive_event;
  }

  /**
   * Connects to a given receive event and re-configures its back-pointers.
   * To un-glue, use NULL as the parameter.
   */
  void glue_receive_event(const EventPtr& receive_event);

  /**
   * Connects to given send and receive events.
   * Does a sequential execution of glue_send_event() and glue_receive_event().
   */
  void glue_events(const EventPtr& send_event, const EventPtr& receive_event)
  {
    glue_send_event(send_event);
    glue_receive_event(receive_event);
  }

  virtual bool is_glued() const
  {
    return m_send_event != NULL && m_receive_event != NULL;
  }
};

typedef boost::intrusive_ptr<CompleteMessage> CompleteMessagePtr;

typedef enum
{
  LOST,
  FOUND
} IncompleteMsgType;

class SCMSC_EXPORT IncompleteMessage: public MscMessage
{

protected:

  std::wstring m_instance_label;

  IncompleteMsgType m_type;

  MscPoint m_dot_position;

  /**
   * Peer event.
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  Event* m_event;

public:

  IncompleteMessage(const IncompleteMsgType& type=LOST, const std::wstring& label=L"",
    const std::wstring& instance_label=L"", const MscPoint& dot_position=MscPoint()):
  MscMessage(label),m_instance_label(instance_label),m_type(type),
    m_dot_position(dot_position)
  {
    m_event = NULL;
  }

  IncompleteMessage(IncompleteMessage* original);

  const MscPoint& get_dot_position() const
  {
    return m_dot_position;
  }

  void set_dot_position(const MscPoint& dot_position)
  {
    m_dot_position = dot_position;
  }

  const std::wstring& get_instance_label() const
  {
    return m_instance_label;
  }

  const IncompleteMsgType& get_type() const
  {
    return m_type;
  }

  bool is_lost() const
  {
    return m_type==LOST;
  }

  bool is_found() const
  {
    return m_type==FOUND;
  }

  void glue_event(const EventPtr& event);

  Event* get_event()
  {
    return m_event;
  }

  virtual bool is_glued() const
  {
    return m_event != NULL;
  }
};

typedef boost::intrusive_ptr<IncompleteMessage> IncompleteMessagePtr;

/**
 * \brief Event which occurs in EventArea.
 */
class SCMSC_EXPORT Event:
  public MscElementTmpl<Event>, public Commentable
{
protected:

  /**
   * Relative position of the event to the respective instance/area.
   */
  MscPoint m_position;

  /**
   * Label of message whose this is send or receive event.
   */
  MscMessagePtr m_message;


#ifdef _TIME_H_
 TimeRelationEventPtrList m_time_relations;
 MscTimeIntervalSetDList m_absolut_time;
#endif

  Event() : MscElementTmpl<Event>(), Commentable(),
   m_position(),
   m_message()
  {
  }

  /**
   * @param original - original Event of this Event
   */
  Event(Event* original) : MscElementTmpl<Event>(original), Commentable(),
    m_position(original->m_position),
    m_message()
  {
  }

  CompleteMessage* get_complete() const
  {
    return dynamic_cast<CompleteMessage*>(m_message.get());
  }

  IncompleteMessage* get_incomplete() const
  {
    return dynamic_cast<IncompleteMessage*>(m_message.get());
  }

public:

  virtual ~Event()
  {
  }

  void set_message(const MscMessagePtr& message)
  {
    m_message = message;
  }

  /**
   * Getter for m_message.
   */
  const MscMessagePtr& get_message() const
  {
    return m_message;
  }

#ifdef _TIME_H_
  void add_time_relation(const TimeRelationEventPtr& relation)
  {
    m_time_relations.push_back(relation);
  }

  void remove_time_relation(const TimeRelationEventPtr& relation)
  {
    TimeRelationEventPtrList::iterator pos =
      std::find(m_time_relations.begin(), m_time_relations.end(), relation);

    if(pos != m_time_relations.end())
      m_time_relations.erase(pos);
  }

  void set_time_relations(const TimeRelationEventPtrList& list)
  {
    m_time_relations=list;
  }

  const TimeRelationEventPtrList& get_time_relations() const
  {
    return m_time_relations;
  }

  void clear_time_relations()
  {
    m_time_relations.clear();
  }

//TODO ask Ondra why I cannot do this
//   void empty_time_relations()
//   {
//     TimeRelationEventPtrList::iterator it;
//     for(it = m_time_relations.begin();it!=m_time_relations.end();it++){
//       it->get()->clear_interval_set();
//     }
//   }


  void add_absolut_time(const MscTimeIntervalSetD& absolut_time)
  {
    m_absolut_time.push_back(absolut_time);
  }

  void remove_absolut_time(const MscTimeIntervalSetD& absolut_time)
  {
    MscTimeIntervalSetDList::iterator pos = 
      std::find(m_absolut_time.begin(), m_absolut_time.end(), absolut_time);

    if(pos != m_absolut_time.end())
      m_absolut_time.erase(pos);
  }

  void set_absolut_time(const MscTimeIntervalSetDList& list)
  {
    m_absolut_time = list;
  }

  const MscTimeIntervalSetDList& get_absolut_times() const 
  {
    return m_absolut_time;
  }

  void clear_absolut_times()
  {
    m_absolut_time.clear();
  }
#endif

  /**
   * Return instance of CompleteMessage in case m_message contains the instance,
   * undefined NULL otherwise
   */
  CompleteMessagePtr get_complete_message() const
  {
    return boost::dynamic_pointer_cast<CompleteMessage>(m_message);
  }

  /**
   * Return instance of CompleteMessage in case m_message contains the instance,
   * undefined NULL otherwise
   */
  IncompleteMessagePtr get_incomplete_message() const
  {
    return boost::dynamic_pointer_cast<IncompleteMessage>(m_message);
  }

  /**
   * Retrives matching event of this Event in case this Event has complete
   * message, otherwise it returns NULL
   */
  Event* get_matching_event() const
  {
    if(is_matched())
    {
	      CompleteMessage* complete = get_complete();
	      return (is_send()?complete->get_receive_event():complete->get_send_event());
    }
    return NULL;
  }

  /**
   * True iff is sending event.
   */
  bool is_send() const
  {
    if(is_matched())
    {
      CompleteMessage* complete = get_complete();
      return complete->get_send_event()==this;
    }
    else
    {
      IncompleteMessage* incomplete = get_incomplete();
      return incomplete->is_lost();
    }
  }

  /**
   * True iff is receiving event.
   */
  bool is_receive() const
  {
    return !is_send();
  }

  /**
   * Returns true if m_message is complete.
   */
  int is_matched() const
  {
    return get_complete() != NULL;
  }

  /**
   * Returns Instance which this event occures at
   */
  virtual Instance* get_instance() const=0;

  /**
   * Returns EventArea which this event occures at
   */
  virtual EventArea* get_general_area() const=0;

  const std::wstring& get_receiver_label() const
  {
    if(is_matched())
    {
      CompleteMessage* complete = get_complete();
      return complete->get_receiver()->get_label();
    }
    else
    {
      if(is_send())
      {
        IncompleteMessage* incomplete = get_incomplete();
        return incomplete->get_instance_label();
      }
      else
      {
        return get_instance()->get_label();
      }
    }
  }

  const std::wstring& get_sender_label() const
  {
    if(is_matched())
    {
      CompleteMessage* complete = get_complete();
      return complete->get_sender()->get_label();
    }
    else
    {
      if(is_receive())
      {
        IncompleteMessage* incomplete = get_incomplete();
        return incomplete->get_instance_label();
      }
      else
      {
        return get_instance()->get_label();
      }
    }
  }

  void set_position(const MscPoint& position)
  {
    m_position = position;
  }

  const MscPoint& get_position() const
  {
    return m_position;
  }
};



template <class TArea>
class EventTmpl:public Event
{
protected:

  /**
   * EventArea which this Event occures in.
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  TArea* m_area;

  EventTmpl() : Event(),
    m_area(NULL)
  {
  }

  /**
   * @param original - original Event of this Event
   */
  EventTmpl(Event* original) : Event(original),
    m_area(NULL)
  {
  }

public:

  /**
   * Returns EventArea which this Event occures in
   */
  TArea* get_area() const
  {
    return m_area;
  }

  void set_area(TArea* area)
  {
    m_area = area;
  }

  Instance* get_instance() const
  {
    if(m_area) return m_area->get_instance();
      else return NULL;
  }

  EventArea* get_general_area() const
  {
    return m_area;
  }

};

/**
 * \brief Event occurring in StrictOrderArea
 */
class SCMSC_EXPORT StrictEvent: public EventTmpl<StrictOrderArea>
{

protected:

  /**
   * Successor of this event
   */
  StrictEventPtr m_successor;

  /**
   * Predecessor of this Event
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  StrictEvent* m_predecessor;

public:

  StrictEvent() : EventTmpl<StrictOrderArea>()
  {
    m_predecessor = NULL;
  }

  /**
   * See Event constructor for details
   */
  StrictEvent(Event* original) : EventTmpl<StrictOrderArea>(original)
  {
    m_predecessor = NULL;
  }

  virtual ~StrictEvent()
  {
  }

  /**
   * Getter for m_successor.
   */
  const StrictEventPtr& get_successor() const
  {
    return m_successor;
  }

  /**
   * Getter for predecessor.
   */
  StrictEvent* get_predecessor() const
  {
    return m_predecessor;
  }

  /**
   * Setter for m_successor.
   */
  void set_successor(const StrictEventPtr& successor)
  {
    if(!is_last())
    {
      m_successor->m_predecessor = successor.get();
    }
    successor->m_predecessor = this;
    successor->m_successor = m_successor;
    m_successor = successor;
  }

  void set_predecessor(const StrictEventPtr& predecessor)
  {
    if(!is_first())
    {
      m_predecessor->m_successor = predecessor.get();
    }
    predecessor->m_successor = this;
    predecessor->m_predecessor = m_predecessor;
    m_predecessor = predecessor.get();
  }

  int is_first() const
  {
    return m_predecessor==NULL;
  }

  int is_last() const
  {
    return m_successor==NULL;
  }

};

class SCMSC_EXPORT CoregionEventRelation:public MscElementTmpl<CoregionEventRelation>
{
private:

  CoregionEvent* m_predecessor;

  CoregionEvent* m_successor;

  PolyLine m_line;

public:

  CoregionEventRelation(CoregionEvent* predecessor, CoregionEvent* successor,
    const PolyLine& line = PolyLine()):MscElementTmpl<CoregionEventRelation>()
  {
    m_predecessor = predecessor;
    m_successor = successor;
    m_line = line;
  }

  CoregionEvent* get_predecessor() const
  {
    return m_predecessor;
  }

  CoregionEvent* get_successor() const
  {
    return m_successor;
  }

  void set_predecessor(CoregionEvent* predecessor)
  {
    m_predecessor = predecessor;
  }

  void set_successor(CoregionEvent* successor)
  {
    m_successor = successor;
  }
};

typedef boost::intrusive_ptr<CoregionEventRelation> CoregEventRelPtr;
typedef std::vector<CoregEventRelPtr> CoregEventRelPtrVector;


/**
 * \brief Event occurring in CoregionArea
 */
class SCMSC_EXPORT CoregionEvent: public EventTmpl<CoregionArea>
{
  //! Successors of this event.
  // note: vector is used to reduce overhead and preserve the insertion order
  CoregEventRelPtrVector m_successors;

  //! Predecessors of this event.
  CoregEventRelPtrVector m_predecessors;

public:

  CoregionEvent() : EventTmpl<CoregionArea>()
  {
  }

  /**
   * See Event constructor for details
   */
  CoregionEvent(Event* original) : EventTmpl<CoregionArea>(original)
  {
  }

  virtual ~CoregionEvent();

  /**
   * Getter for m_successors
   */
  const CoregEventRelPtrVector& get_successors() const
  {
      return m_successors;
  }

  /**
   * Getter for m_predecessors
   */
  const CoregEventRelPtrVector& get_predecessors() const
  {
      return m_predecessors;
  }

  /**
   * Inserts successors
   */
  void add_successor(const CoregEventRelPtr& rel);

  /**
   * Removes successor
   */
  void remove_successor(const CoregEventRelPtr& e);

  void add_predecessor(const CoregEventRelPtr& e);

  void remove_predecessor(const CoregEventRelPtr& e);

  bool has_successors() const
  {
    return m_successors.size()!=0;
  }

  bool has_predecessors() const
  {
    return m_predecessors.size()!=0;
  }

  /**
   * Getter for m_area
   */
  CoregionArea* get_coregion_area() const;

  bool is_minimal() const
  {
    return m_predecessors.size()==0;
  }

  bool is_maximal() const
  {
    return m_successors.size()==0;
  }

  CoregEventRelPtr add_successor(CoregionEvent* e);

};

/**
 * \brief General base area which contains events.
 *
 * For details about EventArea organization at Instance see Instance.
 */
class SCMSC_EXPORT EventArea:public MscElementTmpl<EventArea>
{
protected:

  /**
   * Following EventArea in Instance.
   */
  EventAreaPtr m_next;

  /**
   * Previous EventArea
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  EventArea* m_previous;

  Coordinate m_begin_height;
  Coordinate m_end_height;

  Size m_width;

  /**
   * Instance which EventArea occures on
   *
   * @warning boost::intrusive_ptr mustn't be used because of possible cyclic dependency
   */
  Instance* m_instance;

public:

  EventArea() : MscElementTmpl<EventArea>(),
    m_next(),
    m_previous(NULL),
    m_begin_height(0),
    m_end_height(0),
    m_width(10),
    m_instance(NULL)
  {
  }

  /**
   * Constructor.
   */
  EventArea(EventArea* original) : MscElementTmpl<EventArea>(original),
    m_next(),
    m_previous(NULL),
    m_begin_height(original->m_begin_height),
    m_end_height(original->m_end_height),
    m_width(original->m_width),
    m_instance(NULL)
  {
  }

  virtual ~EventArea(){}

  /**
   * Set EventArea's successor
   */
  void set_next(const EventAreaPtr& n)
  {
      m_next = n;
      n->m_previous = this;
  }

  /**
   * Getter for m_next
   */
  const EventAreaPtr& get_next() const
  {
    return m_next;
  }

  /**
   * Getter for m_instance
   */
  Instance* get_instance() const
  {
    return m_instance;
  }

  void set_instance(Instance* instance)
  {
    m_instance = instance;
  }

  /**
   * Getter for m_previous
   */
  EventArea* get_previous() const
  {
    return m_previous;
  }

  /**
   * Returns true iff doesn't contain any Events.
   */
  virtual bool is_empty() const = 0;

  /**
   * Return true iff this Area is first at instance
   */
  bool is_first() const
  {
    return m_previous==NULL;
  }

  const Coordinate& get_begin_height() const
  {
    return m_begin_height;
  }

  void set_begin_height(const Coordinate& begin_height)
  {
    m_begin_height = begin_height;
  }

  const Coordinate& get_end_height() const
  {
    return m_end_height;
  }

  void set_end_height(const Coordinate& end_height)
  {
    m_end_height = end_height;
  }

  Size get_height() const
  {
    return fabs(m_end_height - m_begin_height);
  }

  Size get_width() const
  {
    return m_width;
  }

  void set_width(const Size& width)
  {
    m_width = width;
  }

  /**
   * Adds event into this area. The added event is returned.
   */
  virtual EventPtr add_event()=0;
};

/**
 * \brief EventArea whose events are ordered linearly as they follow each other.
 *
 * Events in this area are organized as bidirectional linked lists with
 * successors and predecessors defined directly in elements of this list
 * (not using std::list).
 */
class SCMSC_EXPORT StrictOrderArea:public EventArea
{

  /**
   * First of Events which occure in this area.
   */
  StrictEventPtr m_first;

  /**
   * Last of Events which occure in this area.
   */
  StrictEventPtr m_last;

public:

  StrictOrderArea() : EventArea()
  {
  }

  StrictOrderArea(StrictOrderArea* original) : EventArea(original)
  {
  }

  virtual ~StrictOrderArea()
  {
  }

  /**
   * Getter for m_first.
   */
  const StrictEventPtr& get_first() const
  {
    return m_first;
  }

  /**
   * Setter for m_first
   *
   * TODO: throw exception if first parameter has predecessor
   */
  void set_first(const StrictEventPtr& first)
  {
    if(!is_empty())
      m_first->set_predecessor(first);
    m_first = first;
    if(!m_last.get())
      m_last = m_first;
  }

  /**
   * Getter for m_first.
   */
  const StrictEventPtr& get_last() const
  {
    return m_last;
  }

  /**
   * Setter for m_last
   *
   * TODO: throw exception if last parameter has successor
   */
  void set_last(const StrictEventPtr& last)
  {
    if(!m_last.get())
    {
      m_last = last;
      m_first = last;
    }
    else
    {
      m_last->set_successor(last);
      m_last = last;
    }
    last->set_area(this);
  }

  bool is_empty() const
  {
    return m_first.get()==NULL;
  }

  virtual EventPtr add_event()
  {
    return add_event(new StrictEvent());
  }

  StrictEventPtr add_event(const StrictEventPtr& e)
  {
    StrictEventPtr ev = e;
    if(e == NULL)
      ev = new StrictEvent();

    set_last(ev);
    return ev;
  }
};

/**
 * \brief EventArea whose events are ordered like directed acyclic graph.
 *
 * Graph is organized by list of successors of particular element (CoregionEvent).
 */
class SCMSC_EXPORT CoregionArea:public EventArea
{

protected:

  /**
   * Forma of drawn coregion
   */
  InstanceAxisForm m_form;

  //! Events which aren't successors of any other events.
  // note: vector is used to reduce overhead and preserve the insertion order
  CoregionEventPVector m_minimal_events;

  //! Events which aren't successors of any other events.
  CoregionEventPVector m_maximal_events;

  /**
   * Contains all events of this area
   */
  CoregionEventPtrSet m_events;

public:

  CoregionArea() : EventArea(),
    m_form(COLUMN)
  {
  }

  /**
   * See EventArea constructor for details
   */
  CoregionArea(CoregionArea* original) : EventArea(original)
  {
    m_form = original->get_form();
  }

  /**
   * Removes all referenced CoregionEvents
   */
  virtual ~CoregionArea();

  /**
   * Getter for m_minimal_events.
   */
  const CoregionEventPVector& get_minimal_events() const
  {
    return m_minimal_events;
  }

  /**
   * Getter for m_maximal_events.
   */
  const CoregionEventPVector& get_maximal_events() const
  {
    return m_maximal_events;
  }

  /**
   * Adds minimal event.
   *
   * TODO: throw exception if e has predecessors
   */
  void add_minimal_event(CoregionEvent* e)
  {
    m_minimal_events.push_back(e);
  }

  /**
   * Removes minimal event.
   */
  void remove_minimal_event(CoregionEvent* e)
  {
    m_minimal_events.erase(
      std::remove_if(m_minimal_events.begin(), m_minimal_events.end(),
        std::bind2nd(std::equal_to<CoregionEvent*>(), e)),
      m_minimal_events.end());
  }

  /**
   * Adds maximal event.
   *
   * TODO: throw exception if e has successors
   */
  void add_maximal_event(CoregionEvent* e)
  {
    m_maximal_events.push_back(e);
  }

  /**
   * Removes maximal event.
   */
  void remove_maximal_event(CoregionEvent* e)
  {
    m_maximal_events.erase(
      std::remove_if(m_maximal_events.begin(), m_maximal_events.end(),
        std::bind2nd(std::equal_to<CoregionEvent*>(), e)),
      m_maximal_events.end());
  }

  bool is_empty() const
  {
    return m_minimal_events.size()==0;
  }

  const CoregionEventPtrSet& get_events() const
  {
    return m_events;
  }

  virtual EventPtr add_event()
  {
    return add_event(new CoregionEvent());
  }

  CoregionEventPtr add_event(CoregionEventPtr e)
  {
    if(e == NULL)
      e = new CoregionEvent();

    m_events.insert(e);

    if(e->is_minimal())
      add_minimal_event(e.get());

    if(e->is_maximal())
      add_maximal_event(e.get());

    e->set_area(this);
    return e;
  }

  void remove_event(CoregionEventPtr e);

  InstanceAxisForm get_form() const
  {
    return m_form;
  }

};

#ifdef _TIME_H_
class SCMSC_EXPORT TimeRelation:public MscElementTmpl<TimeRelation>
{
protected:
  bool m_directed; //! indicates directed constraint, see "origin" keyword
  MscTimeIntervalSet<double> m_interval_set;
  std::string m_measurement; //! measurement variable name
  Size m_width; //! length of the guide line

public:
  TimeRelation(const MscTimeIntervalSetD& set);
  TimeRelation(const std::string& set);
  TimeRelation(TimeRelation *original);
  virtual ~TimeRelation();

  void assign_label(const std::string& label);
  std::string get_label() const;

  bool is_directed() const
  {
    return m_directed;
  }

  void set_directed(bool directed)
  {
    m_directed = directed;
  }

  void clear_interval_set(){
    m_interval_set.clear();
  }

  const MscTimeIntervalSetD& get_interval_set() const;
  MscTimeIntervalSetD& get_interval_set();
  void set_interval_set(const MscTimeIntervalSetD& set);

  const std::string& get_measurement() const
  {
    return m_measurement;
  }

  void set_measurement(std::string& measurement)
  {
    m_measurement = measurement;
  }

  Size get_width() const
  {
    return m_width;
  }

  void set_width(const Size& width)
  {
    m_width = width;
  }
};

class SCMSC_EXPORT TimeRelationEvent:public TimeRelation
{
private:
  Event* m_event_origin;
  Event* m_event;

public:

  TimeRelationEvent(MscTimeIntervalSet<double> set):
    TimeRelation(set), m_event_origin(NULL), m_event(NULL)
  {
  }

  TimeRelationEvent(const std::string& set):
    TimeRelation(set), m_event_origin(NULL), m_event(NULL)
  {
  }

  TimeRelationEvent(TimeRelationEvent *original);

  virtual ~TimeRelationEvent()
  {
  }

  Event* get_event_a() const
  {
    return m_event_origin;
  }

  Event* get_event_b() const
  {
    return m_event;
  }

  void glue_event_a(Event* a)
  {
    // unglue, if already glued
    if(m_event_origin)
      m_event_origin->remove_time_relation(this);

    m_event_origin = a;
    m_event_origin->add_time_relation(this);
  }

  void glue_event_b(Event* b)
  {
    // unglue, if already glued
    if(m_event)
      m_event->remove_time_relation(this);

    m_event = b;
    m_event->add_time_relation(this);
  }

  void glue_events(Event* a, Event* b)
  {
    glue_event_a(a);
    glue_event_b(b);
  }

  bool is_glued() const
  {
    return m_event_origin != NULL && m_event != NULL;
  }
};

class SCMSC_EXPORT TimeRelationRefNode:public TimeRelation
{
private:
  ReferenceNode* m_node_origin;
  ReferenceNode* m_node;

public:

  TimeRelationRefNode(MscTimeIntervalSet<double> set):
    TimeRelation(set), m_node_origin(NULL), m_node(NULL)
  {
  }

  TimeRelationRefNode(const std::string& set):
    TimeRelation(set), m_node_origin(NULL), m_node(NULL)
  {
  }

  TimeRelationRefNode(TimeRelationRefNode *original);

  ReferenceNode* get_ref_node_a() const
  {
    return m_node_origin;
  }

  bool is_top_node_a()
  {
    return m_node_origin->is_time_relation_top(this);
  }

  bool is_bottom_node_a()
  {
    return m_node_origin->is_time_relation_bottom(this);
  }

  ReferenceNode* get_ref_node_b() const
  {
    return m_node;
  }

  bool is_top_node_b()
  {
    return m_node->is_time_relation_top(this);
  }

  bool is_bottom_node_b()
  {
    return m_node->is_time_relation_bottom(this);
  }

  void glue_ref_node_a(bool bottom, ReferenceNode* a)
  {
    // unglue, if already glued
    if(m_node_origin)
    {
      if(is_bottom_node_a())
        m_node_origin->remove_time_relation_bottom(this);
      else
        m_node_origin->remove_time_relation_top(this);
    }

    m_node_origin = a;

    if(bottom)
      m_node_origin->add_time_relation_bottom(this);
    else
      m_node_origin->add_time_relation_top(this);
  }

  void glue_ref_node_b(bool bottom, ReferenceNode* b)
  {
    // unglue, if already glued
    if(m_node)
    {
      if(is_bottom_node_b())
        m_node->remove_time_relation_bottom(this);
      else
        m_node->remove_time_relation_top(this);
    }

    m_node = b;

    if(bottom)
      m_node->add_time_relation_bottom(this);
    else
      m_node->add_time_relation_top(this);
  }

  void glue_ref_nodes(bool bottom_a, ReferenceNode* a, bool bottom_b, ReferenceNode* b)
  {
    glue_ref_node_a(bottom_a, a);
    glue_ref_node_b(bottom_b, b);
  }

  ~TimeRelationRefNode()
  {
  }
};

#endif

#endif /* _MSC_H */

// $Id: msc.h 1037 2011-02-09 19:47:01Z madzin $
