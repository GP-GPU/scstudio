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
 * $Id: msc.cpp 951 2010-09-16 21:48:09Z gotthardp $
 */

#include <stack>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "data/msc.h"
void BMsc::remove_instances(const std::wstring &label)
{
  InstancePtrList::iterator it;

  it = m_instances.begin(); 
  while(it!= m_instances.end())
   if((*it)->get_label() == label)
      it = m_instances.erase(it);
   else it++;
}

void NodeRelation::set_successor(SuccessorNode* succ)
{
  m_successor = succ;
  succ->m_predecessors.push_back(this);
}

void NodeRelation::set_predecessor(PredecessorNode* pred)
{
  m_predecessor = pred;
  pred->m_successors.push_back(this);
}

void NodeRelation::set_line(const PolyLine& line)
{
  m_line = line;
}

const PolyLine& NodeRelation::get_line() const
{
  return m_line;
}

void SuccessorNode::remove_predecessor(const NodeRelationPtr& n)
{
  // m_predecessors.erase(n)
  // note: vector does not support the erase() function
  std::remove_if(m_predecessors.begin(), m_predecessors.end(),
    std::bind2nd(std::equal_to<NodeRelationPtr>(), n));

  // n->get_predecessor()->m_successors.erase(n)
  std::remove_if(n->get_predecessor()->m_successors.begin(), n->get_predecessor()->m_successors.end(),
    std::bind2nd(std::equal_to<NodeRelationPtr>(), n));
}

void  SuccessorNode::remove_predecessors()
{
  NodeRelationPtrVector::iterator i;
  for(i=m_predecessors.begin();i!=m_predecessors.end();i++)
  {
    (*i)->get_predecessor()->remove_successor(*i);
  }
  m_predecessors.clear();
}

////////////////////////////////////////////////////////////////////////////

void PredecessorNode::remove_successor(const NodeRelationPtr& n)
{
  // m_successors.erase(n)
  // note: vector does not support the erase() function
  std::remove_if(m_successors.begin(), m_successors.end(),
    std::bind2nd(std::equal_to<NodeRelationPtr>(), n));

  // n->get_successor()->m_predecessors.erase(n)
  std::remove_if(n->get_successor()->m_predecessors.begin(), n->get_successor()->m_predecessors.end(),
    std::bind2nd(std::equal_to<NodeRelationPtr>(), n));
}

void  PredecessorNode::remove_successors()
{
  NodeRelationPtrVector::iterator i;
  for(i=m_successors.begin();i!=m_successors.end();i++)
  {
    (*i)->get_successor()->remove_predecessor(*i);
  }
  m_successors.clear();
}

/////////////////////////////////////////////////////////////////

HMscNodePtr HMscNode::my_ptr()
{
  if(m_owner==NULL){
    HMscNodePtr a;
    return a;
  }
  return m_owner->find_ptr(this);
}

/////////////////////////////////////

void HMsc::add_node(HMscNodePtr node)
{
  m_nodes.insert(node);
  node->set_owner(this);
}

void HMsc::remove_node(HMscNode* node)
{
  HMscNodePtrSet::iterator i;
  for(i=m_nodes.begin();i!=m_nodes.end();i++)
  {
    if((*i).get()==node)
    {
      remove_node(*i);
      break;
    }
  }
}

void HMsc::remove_node(HMscNodePtr node)
{
  PredecessorNode* pred = dynamic_cast<PredecessorNode*>(node.get());
  if(pred)
  {
    pred->remove_successors();
  }
  SuccessorNode* suc = dynamic_cast<SuccessorNode*>(node.get());
  if(suc)
  {
    suc->remove_predecessors();
  }
  m_nodes.erase(node);
}

void ConditionNode::assign_label(const std::string& label)
{
  enum ParserState
  {
    KEYWORD,
    STATE_NAME,
    COMMA,
    PROBABILITY,
    PERCENT,
    NOTHING
  };

  ParserState state = KEYWORD;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  boost::char_separator<char> sep("\t\r\n ", ",%");
  tokenizer tokens(label, sep);

  for(tokenizer::iterator itoken = tokens.begin();
    itoken != tokens.end(); itoken++)
  {
    std::string lowcase_token;
    lowcase_token.reserve(itoken->length());
    // transform token to lowercase for case insensitive comparison
    std::transform(itoken->begin(), itoken->end(),
      std::back_inserter(lowcase_token), ::tolower);

    switch(state)
    {
    case KEYWORD:
      if(lowcase_token == "when")
      {
        m_type = GUARDING;
        state = STATE_NAME;
      }
      else if(lowcase_token == "for")
      {
        m_type = RANDOM;
        state = PROBABILITY;
      }
      else if(lowcase_token == "otherwise")
      {
        m_type = OTHERWISE;
        state = NOTHING;
      }
      else
      {
        m_type = SETTING;
        m_names.push_back(*itoken);
        state = COMMA;
      }

      break;

    case STATE_NAME:
      if(std::isalnum(lowcase_token[0]))
      {
        m_names.push_back(*itoken);
        state = COMMA;
      }
      else
        throw std::runtime_error("Bad condition: expecting state name");

      break;

    case COMMA:
      if(lowcase_token[0] == ',')
        state = STATE_NAME;
      else
        throw std::runtime_error("Bad condition: expecting next state name");

      break;

    case PROBABILITY:
      try
      {
        m_probability = boost::lexical_cast<double>(*itoken)/100.0;
        if(m_probability < 0 || m_probability > 1)
          throw std::runtime_error("Bad condition: wrong probability");
        state = PERCENT;
      }
      catch(boost::bad_lexical_cast&)
      {
        throw std::runtime_error("Bad condition: expecting probability");
      }

      break;

    case PERCENT:
      if(lowcase_token[0] == '%')
        state = NOTHING;
      else
        throw std::runtime_error("Bad condition: expecting % sign");

      break;

    case NOTHING:
      throw std::runtime_error("Bad condition: unexpected string");

    }
  }

  switch(state)
  {
  case KEYWORD:
    throw std::runtime_error("Bad condition: expecting keyword or state name");
  case STATE_NAME:
    throw std::runtime_error("Bad condition: expecting state name");
  case PROBABILITY:
    throw std::runtime_error("Bad condition: expecting probability");
  case PERCENT:
    throw std::runtime_error("Bad condition: expecting % sign");

  case COMMA:
  case NOTHING:
    break; // correct
  }
}

std::string ConditionNode::get_label() const
{
  std::stringstream text;

  switch(m_type)
  {
  case GUARDING:
    text << "when ";
    /* no break */
  case SETTING:
    for(std::vector<std::string>::const_iterator npos = m_names.begin();
      npos != m_names.end(); npos++)
    {
      if(npos != m_names.begin())
        text << ", ";

      text << *npos;
    }
    break;

  case RANDOM:
    text << "for " << m_probability*100.0 << "%";
    break;

  case OTHERWISE:
    text << "otherwise";
    break;
  }

  return text.str();
}

////////////////////////////////////

/*inline StrictOrderArea* StrictEvent::get_strict_order_area()
{
  return dynamic_cast<StrictOrderArea*>(m_area);
}*/

CoregionArea::~CoregionArea()
{
}

void CoregionArea::remove_event(CoregionEventPtr e)
{
  CoregEventRelPtrVector successors_rel = e->get_successors();
  CoregEventRelPtrVector predecessors_rel = e->get_predecessors();
  CoregionEvent* predecessor;

  for(unsigned int i = 0; i < predecessors_rel.size(); i++)
  {
    predecessor = predecessors_rel[i]->get_predecessor();
    
    //set relations among event's successors and predecessors
    for(unsigned j = 0; j < successors_rel.size(); j++)
    {
      predecessor->add_successor(successors_rel[j]);
    }

    //remove connection between event's predecessor and event
    predecessor->remove_successor(predecessors_rel[i]);
  }

  //remove event from maximal and minimal events
    this->remove_minimal_event(e.get());
    this->remove_maximal_event(e.get());

  //remove event
  if(this->m_events.find(e) != this->m_events.end())
    this->m_events.erase(this->m_events.find(e));
}

CoregionEvent::~CoregionEvent()
{
}

void CoregionEvent::add_successor(const CoregEventRelPtr& rel)
{
  m_successors.push_back(rel);
  rel->set_predecessor(this);
  get_coregion_area()->remove_maximal_event(this);

  rel->get_successor()->m_predecessors.push_back(rel);
  CoregionArea* successor_area = rel->get_successor()->get_coregion_area();
  if(get_coregion_area() != successor_area)
    throw std::invalid_argument("Successor not in the same area.");
  successor_area->remove_minimal_event(rel->get_successor());
}

void CoregionEvent::remove_successor(const CoregEventRelPtr& e)
{
  // m_successors.erase(e)
  // note: vector does not support the erase() function
  m_successors.erase(
    std::remove_if(m_successors.begin(), m_successors.end(),
      std::bind2nd(std::equal_to<CoregEventRelPtr>(), e)),
    m_successors.end());


  if(!has_successors())
    get_coregion_area()->add_maximal_event(this);

  // e->get_successor()->m_predecessors.erase(e)
  e->get_successor()->m_predecessors.erase(
    std::remove_if(e->get_successor()->m_predecessors.begin(), e->get_successor()->m_predecessors.end(),
      std::bind2nd(std::equal_to<CoregEventRelPtr>(), e)),
    e->get_successor()->m_predecessors.end());

  if(!e->get_successor()->has_predecessors())
    get_coregion_area()->add_minimal_event(e->get_successor());
}

CoregionArea* CoregionEvent::get_coregion_area() const
{
  return dynamic_cast<CoregionArea*>(m_area);
}

CoregEventRelPtr CoregionEvent::add_successor(CoregionEvent* e)
{
  CoregEventRelPtr rel = new CoregionEventRelation(this,e);
  add_successor(rel);
  return rel;
}

Instance* CompleteMessage::get_sender() const
{
  return m_send_event->get_instance();
}

void CompleteMessage::glue_send_event(const EventPtr& send_event)
{
  if(send_event.get() != m_send_event)
  {
    // unglue, if already glued
    if(m_send_event != NULL)
      m_send_event->set_message(NULL);

    if(send_event != NULL)
      send_event->set_message(this);

    m_send_event = send_event.get();
  }
}

Instance* CompleteMessage::get_receiver() const
{
  return m_receive_event->get_instance();
}

void CompleteMessage::glue_receive_event(const EventPtr& receive_event)
{
  if(receive_event != m_receive_event)
  {
    // unglue, if already glued
    if(m_receive_event != NULL)
      m_receive_event->set_message(NULL);

    if(receive_event != NULL)
      receive_event->set_message(this);

    m_receive_event = receive_event.get();
  }
}

CompleteMessage::CompleteMessage(Event* sender, Event* receiver, MscMessage* original):
MscMessage(original),m_send_event(sender),m_receive_event(receiver)
{
}

void BMsc::add_instance(InstancePtr& i)
{
  m_instances.push_back(i);
  i->set_bmsc(this);
}

void Instance::add_area(EventAreaPtr area)
{
  if(!is_empty())
  {
    m_last->set_next(area);
  }
  else
  {
    m_first = area;
  }
  m_last = area;
  area->set_instance(this);
}

bool Instance::any_event(EventAreaPtr area)
{
  if(!area)
    return false;
  if(area->is_empty())
    return any_event(area->get_next());
  else
    return true;
}

/////////////////////////////////////////////////////////////////////////////

MscMessage::MscMessage(const std::wstring& label):
MscElementTmpl<MscMessage>(),m_label(label)
{
}

MscMessage::MscMessage(MscMessage* original):
MscElementTmpl<MscMessage>(original)
{
  if(original)
  {
    m_label = original->get_label();
  }
}

MscMessage::~MscMessage()
{
}

/////////////////////////////////////////////////////////////////////////////

IncompleteMessage::IncompleteMessage(IncompleteMessage* original):MscMessage(original)
{
  m_dot_position = original->m_dot_position;
  m_type = original->m_type;
}

void IncompleteMessage::glue_event(const EventPtr& event)
{
  if(event != m_event)
  {
    // unglue, if already glued
    if(m_event != NULL)
      m_event->set_message(NULL);

    if(event != NULL)
      event->set_message(this);

    m_event = event.get();
  }
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _TIME_H_
TimeRelation::TimeRelation(const MscTimeIntervalSetD& set):
  MscElementTmpl<TimeRelation>(),
  m_directed(false), m_interval_set(set), m_measurement(), m_width(0)
{
}

TimeRelation::TimeRelation(const std::string& value):
  MscElementTmpl<TimeRelation>(),
  m_directed(false), m_interval_set(), m_measurement(), m_width(0)
{
  assign_label(value);
}

TimeRelation::TimeRelation(TimeRelation* original):
  MscElementTmpl<TimeRelation>(original)
{
  if(original)
  {
    m_directed = original->is_directed();
    m_interval_set = original->m_interval_set;
    m_measurement = original->m_measurement;
    m_width = original->get_width();
  }
}

TimeRelation::~TimeRelation()
{
}

void TimeRelation::assign_label(const std::string& label)
{
  enum ParserState
  {
    AWAIT_FIRST,
    INTERVAL,
    AWAIT_NEXT,
    CONJUNCTION,
    MEASUREMENT_FIRST,
    MEASUREMENT_NEXT,
    AWAIT_END
  };

  ParserState state = AWAIT_FIRST;

  std::string interval;
  std::string measurement;

  for(std::string::const_iterator pos = label.begin();
    pos != label.end(); pos++)
  {
    switch(state)
    {
    case AWAIT_FIRST:
      if(*pos == '[' || *pos == '(')
      {
        state = INTERVAL;
        interval.push_back(*pos);
      }
      else if(*pos == '&')
        state = MEASUREMENT_FIRST;
      else if(isspace(*pos))
        break;
      else
        throw MscIntervalStringConversionError("expecting interval or measurement");

      break;

    case INTERVAL:
      interval.push_back(*pos);

      if(*pos == ']' || *pos == ')')
        state = AWAIT_NEXT;

      break;

    case AWAIT_NEXT:
      if(*pos == '+')
      {
        state = CONJUNCTION;
        interval.push_back(*pos);
      }
      else if(*pos == '&')
        state = MEASUREMENT_FIRST;
      else if(isspace(*pos))
        break;
      else
        throw MscIntervalStringConversionError("expecting interval conjunction or measurement");

      break;

    case CONJUNCTION:
      if(*pos == '[' || *pos == '(')
      {
        state = INTERVAL;
        interval.push_back(*pos);
      }
      else if(isspace(*pos))
        break;
      else
        throw MscIntervalStringConversionError("expecting interval");

      break;

    case MEASUREMENT_FIRST:
      if(isalpha(*pos))
      {
        state = MEASUREMENT_NEXT;
        measurement.push_back(*pos);
      }
      else
        throw MscIntervalStringConversionError("invalid measurement");

      break;

    case MEASUREMENT_NEXT:
      if(isalnum(*pos))
        measurement.push_back(*pos);
      else if(isspace(*pos))
        state = AWAIT_END;
      else
        throw MscIntervalStringConversionError("invalid measurement name");

      break;

    case AWAIT_END:
      if(!isspace(*pos))
        throw MscIntervalStringConversionError("unexpected character");

      break;
    }
  }

  switch(state)
  {
  case AWAIT_FIRST:
    throw MscIntervalStringConversionError("empty time interval");

  case INTERVAL:
    throw MscIntervalStringConversionError("unterminated time interval");

  case AWAIT_NEXT:
    break; // correct

  case CONJUNCTION:
    throw MscIntervalStringConversionError("expecting interval conjunction");

  case MEASUREMENT_FIRST:
    throw MscIntervalStringConversionError("expecting measurement");

  case MEASUREMENT_NEXT:
  case AWAIT_END:
    break; // correct
  }

  if(!interval.empty())
    m_interval_set.assign(interval);
  else
    m_interval_set.set_infinity();

  m_measurement.assign(measurement);
}

std::string TimeRelation::get_label() const
{
  std::stringstream text;
  // extract textual representation of the constraint
  if(!m_measurement.empty())
  {
    // don't print infinite intervals for measurements
    if(!m_interval_set.is_infinity())
      text << m_interval_set << " ";

    text << "&" << m_measurement;
  }
  else
    text << m_interval_set;

  return text.str();
}

const MscTimeIntervalSetD& TimeRelation::get_interval_set() const
{
  return m_interval_set;
}

MscTimeIntervalSetD& TimeRelation::get_interval_set()
{
  return m_interval_set;
}

void TimeRelation::set_interval_set(const MscTimeIntervalSetD& set)
{
  m_interval_set = set;
}

TimeRelationEvent::TimeRelationEvent(TimeRelationEvent *original):
  TimeRelation(original), m_event_origin(NULL), m_event(NULL)
{
}

TimeRelationRefNode::TimeRelationRefNode(TimeRelationRefNode *original):
  TimeRelation(original), m_node_origin(NULL), m_node(NULL)
{
}

#endif

// $Id: msc.cpp 951 2010-09-16 21:48:09Z gotthardp $
