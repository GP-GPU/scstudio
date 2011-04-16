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
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: z120_save.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include <algorithm>
#include <vector>
#include <list>
#include <locale>
#include <map>
#include <ostream>
#include <iterator>

#include "data/Z120/z120.h"

// list of ITU-T Z.120 keywords that must not be used as names
static const char* keywords[] =
{
  "action",
  "after",
  "all",
  "alt",
  "as",
  "before",
  "begin",
  "bottom",
  "call",
  "comment",
  "concurrent",
  "condition",
  "connect",
  "create",
  "data",
  "decomposed",
  "def",
  "empty",
  "end",
  "endconcurrent",
  "endexpr",
  "endinstance",
  "endmethod",
  "endmsc",
  "endsuspension",
  "env",
  "equalpar",
  "escape",
  "exc",
  "external",
  "final",
  "finalized",
  "found",
  "from",
  "gate",
  "in",
  "inf",
  "inherits",
  "initial",
  "inline",
  "inst",
  "instance",
  "int_boundary",
  "label",
  "language",
  "loop",
  "lost",
  "method",
  "msc",
  "mscdocument",
  "msg",
  "nestable",
  "nonnestable",
  "offset",
  "opt",
  "origin",
  "otherwise",
  "out",
  "par",
  "parenthesis",
  "receive",
  "redefined",
  "reference",
  "related",
  "replyin",
  "replyout",
  "seq",
  "shared",
  "starttimer",
  "stop",
  "stoptimer",
  "suspension",
  "text",
  "time",
  "timeout",
  "timer",
  "to",
  "top",
  "undef",
  "using",
  "utilities",
  "variables",
  "via",
  "virtual",
  "when",
  "wildcards",
  NULL
};

/* String modifier to print Z.120 names in a correct character set.
 *
 * This is a last resort correction. Invalid characters are replaced by underscore.
 * More sophisticated transliteration should be done in the front-ends using platform
 * specific functions.
 *
 * Note: Naming conventions violated for 'code prettiness' reasons.
 */
#define VALID_NAME(name) __VALID_NAME(this, name)

class __VALID_NAME
{
public:
  __VALID_NAME(Z120* z120, const std::wstring &name)
    : m_z120(z120), m_name(name)
  { }

  friend std::ostream&
  operator<<(std::ostream& os, const __VALID_NAME& value)
  {
    static const char replacement = '_';
    bool was_replacement = false;

    std::string new_value;
    // performance optimalization: allocate buffer for all characters
    new_value.reserve(value.m_name.length());

    // step 1: character transliteration
    for(std::wstring::const_iterator pos = value.m_name.begin();
      pos != value.m_name.end(); pos++)
    {
      char stripped;

      if((*pos >= 'A' && *pos <= 'Z') || (*pos >= 'a' && *pos <= 'z')
        || (*pos >= '0' && *pos <= '9')
        || *pos == '_' || *pos == '.')
      {
        // print valid characters
        new_value.push_back((char)*pos);

        was_replacement = false;
      }
      else if((stripped = strip_diacritics(*pos)) != 0)
      {
        // use the transliteration table
        new_value.push_back(stripped);
        was_replacement = false;
      }
      else
      {
        // replace invalid characters by the 'replacement'
        // shrink multiple replacements into a single characters
        if(!was_replacement)
          new_value.push_back(replacement);

        was_replacement = true;
      }
    }

    std::string lowcase_value;
    lowcase_value.reserve(new_value.length());
    // transform new_value to lowercase for case insensitive comparison
    std::transform(new_value.begin(), new_value.end(),
      std::back_inserter(lowcase_value), ::tolower);
    // step 2: check for keywords
    for(const char** word = keywords; *word != NULL; word++)
    {
      if(lowcase_value == *word)
      {
        value.m_z120->print_keyword_warning(new_value);

        new_value.push_back(replacement);
        break;
      }
    }

    return os << new_value;
  }

private:
  Z120* m_z120;
  std::wstring m_name;
};

std::ostream&
operator<<(std::ostream& os, const VALID_CHARACTER_STRING& value)
{
  static const char replacement = '_';
  bool was_replacement = false;

  for(std::wstring::const_iterator pos = value.m_text.begin();
    pos != value.m_text.end(); pos++)
  {
    char stripped;

    if(*pos == '\'')
    {
      if((pos == value.m_text.begin() && *(pos+1) != '\'')
         || (pos == value.m_text.end() && *(pos-1) != '\'')
         || (*(pos+1) != '\'' && *(pos-1) != '\''))
      {
//TODO print warning that the comment is not valid
//        print_report(RS_WARNING, stringize() << L"Comment text \"" << TOWSTRING(value) << L"\" is not allowed");
          
          // double the apostrophe
          os << "''";
      }
      else
          os << "'";

      was_replacement = false;
    }
    else if(*pos == '\r')
    {
      // ignored characters
      was_replacement = false;
    }
    else if(*pos == '\n' || *pos == '\t')
    {
      // characters transformed to a space character
      os << " ";
      was_replacement = false;
    }
    else if(iswprint(*pos))
    {
      // other printable characters
      os << (char)*pos;
      was_replacement = false;
    }
    else if((stripped = strip_diacritics(*pos)) != 0)
    {
      // use the transliteration table
      os << stripped;
      was_replacement = false;
    }
    else
    {
      // replace invalid characters by the 'replacement'
      // shrink multiple replacements into a single characters
      if(!was_replacement)
        os << replacement;

      was_replacement = true;
    }
  }

  return os;
}

ExportFormatter::PreconditionList Z120::get_preconditions(MscPtr msc) const
{
  ExportFormatter::PreconditionList result;
  // no preconditions
  return result;
}

int Z120::save_msc(std::ostream& stream, const std::wstring &name,
  const MscPtr& selected_msc, const std::vector<MscPtr>& msc)
{
  m_warned_names.clear();

  int result = 0; // error count
  stream << "mscdocument " << VALID_NAME(name) << ";" << std::endl;

  std::set<std::wstring> printed;

  // list of MSC to be printed
  // new references may be added to m_printing by save_hmsc()
  m_printing.push_back(selected_msc);
  std::copy(msc.begin(), msc.end(), std::back_inserter(m_printing));

  for(std::list<MscPtr>::const_iterator pos = m_printing.begin();
    pos != m_printing.end(); pos++)
  {
    if(*pos == NULL)
      continue;

    // if not already generated
    if(printed.find((*pos)->get_label()) == printed.end())
    {
      result += save_msc(stream, *pos);
      printed.insert((*pos)->get_label());
    }
  }

  m_printing.clear();
  return 0;
}

int Z120::save_msc(std::ostream& stream, const MscPtr& msc)
{
  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);
  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);

  if(bmsc != NULL)
    return save_bmsc(stream, bmsc);
  else if(hmsc != NULL)
    return save_hmsc(stream, hmsc);
  else
    return 1; // unexpected pointer
}

void Z120::print_element_attributes(std::ostream& stream, const MscElementPtr& element)
{
  switch(element->get_marked())
  {
    case NONE: break;
    case MARKED: stream << "/* MARKED */" << std::endl; break;
    case ADDED: stream << "/* ADDED */" << std::endl; break;
    case REMOVED: stream << "/* REMOVED */" << std::endl; break;
    default: throw std::runtime_error("Error: unexpected behaviour");
  }

  std::set<std::string> attributes = element->get_attribute_names();
  if(!attributes.empty())
  {
    stream << "/* ATTRIBUTES: [";
    for(std::set<std::string>::const_iterator pos = attributes.begin();
      pos != attributes.end(); pos++)
    {
      stream << *pos;
    }
    stream << "] */" << std::endl;
  }
}

void Z120::print_event(std::ostream& stream, PtrIDMap<MscMessagePtr>& message_id_map,
  const EventPtr& event)
{
  print_element_attributes(stream, event);

  CompleteMessagePtr complete_message = event->get_complete_message();
  if(complete_message != NULL)
  {
    print_element_attributes(stream, complete_message);

    if(complete_message->get_send_event() == event)
      stream << "out " << VALID_NAME(complete_message->get_label())
        << "," << message_id_map.get_id(complete_message)
        << " to " << VALID_NAME(complete_message->get_receiver()->get_label());
    if(complete_message->get_receive_event() == event)
      stream << "in " << VALID_NAME(complete_message->get_label())
        << "," << message_id_map.get_id(complete_message)
        << " from " << VALID_NAME(complete_message->get_sender()->get_label());
  }

  IncompleteMessagePtr incomplete_message = event->get_incomplete_message();
  if(incomplete_message != NULL)
  {
    print_element_attributes(stream, incomplete_message);

    if(incomplete_message->is_lost())
      stream << "out " << VALID_NAME(incomplete_message->get_label())
        << "," << message_id_map.get_id(incomplete_message)
        << " to lost";
    if(incomplete_message->is_found())
      stream << "in " << VALID_NAME(incomplete_message->get_label())
        << "," << message_id_map.get_id(incomplete_message)
        << " from found";
  }
}

void print_absolut_time(std::ostream& stream, const MscTimeIntervalSetD& interval)
{
  const std::list< MscTimeInterval<double> > constraints = interval.get_set();
  
  for(std::list< MscTimeInterval<double> >::const_iterator it = constraints.begin();
    it != constraints.end(); it++)
  {
    if(it->get_begin_value() != it->get_end_value())
    {
      if (it->get_begin_closed())
        stream << "@[";
      else
        stream << "@(";

      stream << it->get_begin() << "," << it->get_end();

      if (it->get_end_closed())
        stream << "]";
      else
        stream << ")";
    }
    else
      stream << "[@" << it->get_begin() << "]";
  }
}

void Z120::print_time_relations(std::ostream& stream, PtrIDMap<EventPtr>& event_id_map,
  const EventPtr& event)
{
  size_t printed = 0;

  const TimeRelationEventPtrList& relations = event->get_time_relations();
  const MscTimeIntervalSetDList& absolut_times = event->get_absolut_times();
  // process all absolut time constraints
  for(MscTimeIntervalSetDList::const_iterator apos = absolut_times.begin();
        apos != absolut_times.end(); apos++)
  {
    if(!printed++)
      stream << "time ";
    else
      stream << ", ";

    //do not used the print function of MscTimeInterval due to [@3]
    print_absolut_time(stream, *apos);
  }

  // process all time relations
  for(TimeRelationEventPtrList::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    // for each "event a" we print the time relation to "event b"
    // we thus skip all "events b"
    if((*rpos)->get_event_a() != event)
      continue;

    // is it a first item being printed?
    if(!printed++)
      stream << "time";
    else
      stream << ",";

    print_element_attributes(stream, *rpos);

    stream << " e" << event_id_map.get_id((*rpos)->get_event_b());

    if((*rpos)->is_directed())
      stream << " origin";

    stream << " " << (*rpos)->get_label();
  }

  if(printed)
    stream << ";" << std::endl;
}

int Z120::save_bmsc(std::ostream& stream, const BMscPtr& bmsc)
{
  PtrIDMap<MscMessagePtr> message_id_map; // message instance identifiers
  PtrIDMap<EventPtr> event_id_map; // event identifiers

  print_element_attributes(stream, bmsc);
  stream << "msc " << VALID_NAME(bmsc->get_label()) << ";" << std::endl;

  // declare instances
  for(InstancePtrList::const_iterator ipos = bmsc->get_instances().begin();
    ipos != bmsc->get_instances().end(); ipos++)
  {
    stream << "inst " << VALID_NAME((*ipos)->get_label()) << ";" << std::endl;
  }

  // print global comments
  print_texts(stream, bmsc);

  // define instances
  for(InstancePtrList::const_iterator ipos = bmsc->get_instances().begin();
    ipos != bmsc->get_instances().end(); ipos++)
  {
    print_element_attributes(stream, *ipos);
    stream << VALID_NAME((*ipos)->get_label()) << ": instance;" << std::endl;

    // walk through event areas
    for(EventAreaPtr area = (*ipos)->get_first();
      area != NULL; area = area->get_next())
    {
      print_element_attributes(stream, area);

      StrictOrderAreaPtr strict_area = boost::dynamic_pointer_cast<StrictOrderArea>(area);
      if(strict_area != NULL)
      {
        // walk through events
        for(StrictEventPtr event = strict_area->get_first();
          event != NULL; event = event->get_successor())
        {
          // check if the label needs to be printed
          //  - label is required for time constrained events
          if(!event->get_time_relations().empty())
            stream << "label e" << event_id_map.get_id(event) << ";"  << std::endl;

          print_event(stream, message_id_map, event);
          print_comments(stream, event);
          stream << ";" << std::endl;
          print_time_relations(stream, event_id_map, event);
        }
      }

      CoregionAreaPtr coregion_area = boost::dynamic_pointer_cast<CoregionArea>(area);
      if(coregion_area != NULL)
      {
        stream << "concurrent;" << std::endl;
        // events to be processed; this is to avoid recursion
        std::list<CoregionEventPtr> event_stack;

        for(CoregionEventPVector::const_iterator mpos = coregion_area->get_minimal_events().begin();
          mpos != coregion_area->get_minimal_events().end(); mpos++)
        {
          // initialize the stack with events with no predecessors
          push_back_if_unique<CoregionEventPtr>(event_stack, *mpos);
        }

        // process all events in the stack
        for(std::list<CoregionEventPtr>::const_iterator epos = event_stack.begin();
          epos != event_stack.end(); epos++)
        {
          // check if the label needs to be printed
          //  - label is required for referenced coregion events
          //  - label is required for time constrained events
          if(!(*epos)->get_predecessors().empty() || !(*epos)->get_time_relations().empty())
            stream << "label e" << event_id_map.get_id(*epos) << ";"  << std::endl;

          print_event(stream, message_id_map, *epos);

          // process backward links
          for(CoregEventRelPtrVector::const_iterator ppos = (*epos)->get_predecessors().begin();
            ppos != (*epos)->get_predecessors().end(); ppos++)
          {
            print_element_attributes(stream, *ppos);
          }

          // process forward links
          for(CoregEventRelPtrVector::const_iterator spos = (*epos)->get_successors().begin();
            spos != (*epos)->get_successors().end(); spos++)
          {
            CoregionEventPtr successor = (*spos)->get_successor();

            // is it a first item being printed?
            if(spos == (*epos)->get_successors().begin())
              stream << " before";
            else
              stream << ",";

            print_element_attributes(stream, *spos);

            stream << " e" << event_id_map.get_id(successor);

            // add successors of this event to the stack
            // note: std::list<>::push_back doesn't invalidate iterators
            push_back_if_unique<CoregionEventPtr>(event_stack, successor);
          }

          print_comments(stream, *epos);
          stream << ";" << std::endl;
          print_time_relations(stream, event_id_map, *epos);
        }

        stream << "endconcurrent;" << std::endl;
      }
    }

    stream << "endinstance;" << std::endl;
  }

  stream << "endmsc;" << std::endl;
  return 0;
}

void Z120::print_time_relations1(std::ostream& stream,
  const ReferenceNodePtr& reference_node, TimeRelationRefNodePtrSet relations)
{
  size_t printed = 0;

  // process all time relations
  for(TimeRelationRefNodePtrSet::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    // phase 1: print only constraints applied to this single node
    if((*rpos)->get_ref_node_a() != (*rpos)->get_ref_node_b())
      continue;

    // is it a first item being printed?
    if(!printed++)
      stream << " time";
    else
      stream << ",";

    print_element_attributes(stream, *rpos);

    stream << " " << (*rpos)->get_label();
  }

  if(printed)
    stream << ";" << std::endl;
}

void Z120::print_time_relations2(std::ostream& stream,
  PtrIDMap<HMscNodePtr>& node_id_map, const std::string& title,
  const ReferenceNodePtr& reference_node, TimeRelationRefNodePtrSet relations)
{
  size_t printed = 0;

  // process all time relations
  for(TimeRelationRefNodePtrSet::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    // phase 2: constraints applied to this single node have already been printed
    if((*rpos)->get_ref_node_a() == (*rpos)->get_ref_node_b())
      continue;
    // for each "node a" we print the time relation to "node b"
    // we thus skip all "node b"
    if((*rpos)->get_ref_node_a() != reference_node)
      continue;

    // is it a first item being printed?
    if(!printed++)
      stream << " " << title;
    else
      stream << ",";

    print_element_attributes(stream, *rpos);

    if((*rpos)->get_ref_node_b() == NULL)
      throw std::invalid_argument("Disconnected time constraint.");

    if((*rpos)->is_bottom_node_b())
      stream << " bottom";
    else
      stream << " top";

    stream << " L" << node_id_map.get_id((*rpos)->get_ref_node_b());

    if((*rpos)->is_directed())
      stream << " origin";

    stream << " " << (*rpos)->get_label();
  }

  if(printed)
    stream << ";" << std::endl;
}

int Z120::save_hmsc(std::ostream& stream, const HMscPtr& hmsc)
{
  PtrIDMap<HMscNodePtr> node_id_map; // node identifiers
  // nodes to be processed; this is to avoid recursion
  std::list<HMscNodePtr> node_stack;

  print_element_attributes(stream, hmsc);
  stream << "msc " << VALID_NAME(hmsc->get_label()) << ";" << std::endl;
  // print global comments
  print_texts(stream, hmsc);

  // initialize the stack with the start node
  push_back_if_unique<HMscNodePtr>(node_stack, hmsc->get_start());

  // process all nodes in the stack
  for(std::list<HMscNodePtr>::const_iterator npos = node_stack.begin();
    npos != node_stack.end(); npos++)
  {
    if(*npos == NULL)
    {
      stream << "/* ERROR: BAD HMSC NODE */" << std::endl;
      continue;
    }

    print_element_attributes(stream, *npos);

    StartNodePtr start_node = boost::dynamic_pointer_cast<StartNode>(*npos);
    if(start_node != NULL)
    {
      stream << "initial";
    }

    ConditionNodePtr condition_node = boost::dynamic_pointer_cast<ConditionNode>(*npos);
    if(condition_node != NULL)
    {
      stream << "L" << node_id_map.get_id(*npos)
        << ": condition " << condition_node->get_label();
      // TODO: expressions may also appear as conditions
    }

    ConnectionNodePtr connection_node = boost::dynamic_pointer_cast<ConnectionNode>(*npos);
    if(connection_node != NULL)
    {
      stream << "L" << node_id_map.get_id(*npos)
        << ":";
    }

    ReferenceNodePtr reference_node = boost::dynamic_pointer_cast<ReferenceNode>(*npos);
    if(reference_node != NULL)
    {
      if(reference_node->get_msc() != NULL)
      {
        stream << "L" << node_id_map.get_id(*npos)
          << ": reference " << VALID_NAME(reference_node->get_msc()->get_label());
        m_printing.push_back(reference_node->get_msc());
      }
      else
        stream << "/* ERROR: BAD HMSC REFERENCE */" << std::endl;
    }

    EndNodePtr end_node = boost::dynamic_pointer_cast<EndNode>(*npos);
    if(end_node != NULL)
    {
      stream << "L" << node_id_map.get_id(*npos)
        << ": final";
    }

    if(reference_node != NULL)
    {
      const TimeRelationRefNodePtrSet& tops = reference_node->get_time_relations_top();
      const TimeRelationRefNodePtrSet& bottoms = reference_node->get_time_relations_bottom();

      // phase 1: print "time" constraints
      print_time_relations1(stream, reference_node, tops);
      // phase 2: print "top" and "bottom" constraints
      print_time_relations2(stream, node_id_map, "top", reference_node, tops);
      print_time_relations2(stream, node_id_map, "bottom", reference_node, bottoms);
    }

    // process backward links
    SuccessorNode *successor_node = dynamic_cast<SuccessorNode*>(npos->get());
    if(successor_node != NULL)
    {
      for(NodeRelationPtrVector::const_iterator ppos = successor_node->get_predecessors().begin();
        ppos != successor_node->get_predecessors().end(); ppos++)
      {
        print_element_attributes(stream, *ppos);
      }
    }

    // process forward links
    PredecessorNode *predecessor_node = dynamic_cast<PredecessorNode*>(npos->get());
    if(predecessor_node != NULL)
    {
      if(predecessor_node->get_successors().empty())
      {
        print_report(RS_WARNING, stringize()
          << L"Warning: HMSC node without successors violates the Z.120 standard.");
      }

      for(NodeRelationPtrVector::const_iterator spos = predecessor_node->get_successors().begin();
        spos != predecessor_node->get_successors().end(); spos++)
      {
        SuccessorNode *successor = (*spos)->get_successor();

        // is it a first item being printed?
        if(spos == predecessor_node->get_successors().begin())
          stream << " connect";
        else
          stream << ",";

        print_element_attributes(stream, *spos);

        HMscNode *successor_node = dynamic_cast<HMscNode*>(successor);
        stream << " L" << node_id_map.get_id(successor_node);

        // add successors of this node to the stack
        // note: std::list<>::push_back doesn't invalidate iterators
        push_back_if_unique<HMscNodePtr>(node_stack, successor_node);
      }
    }

    print_comments(stream, *npos);
    stream << ";" << std::endl;
  }

  stream << "endmsc;" << std::endl;
  return 0;
}

// $Id: z120_save.cpp 1029 2011-02-02 22:17:59Z madzin $
