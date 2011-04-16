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
 * Copyright (c) 2008-2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: z120.h 536 2009-12-18 12:21:45Z gotthardp $
 */

#ifndef _Z120_H
#define _Z120_H

#include "data/formatter.h"
#include "data/Z120/export.h"

struct s_Z120 { };

/* String modifier to print Z.120 character strings in a correct character set.
 */
class VALID_CHARACTER_STRING
{
public:
  VALID_CHARACTER_STRING(const std::wstring &text)
    : m_text(text)
  { }

  friend std::ostream&
  operator<<(std::ostream& os, const VALID_CHARACTER_STRING& value);

private:
  std::wstring m_text;
};

class SCZ120_EXPORT Z120 : public s_Z120, public Formatter

#ifdef HAVE_ANTLR
  , public ImportFormatter
#endif
  , public ExportFormatter
{
public:
  //! file extension used to distinguish this format
  // note: DLL in Windows cannot return pointers to static data
  virtual std::string get_extension() const
  { return "mpr"; }
  //! human readable description of this format
  virtual std::string get_description() const
  { return "Z.120 Textual Format"; }

#ifdef HAVE_ANTLR
  //! import MSC document
  virtual std::vector<MscPtr> load_msc(const std::string &filename);
  //! Returns a list of transformation for this format.
  virtual TransformationList get_transformations(MscPtr msc) const;
#endif
  //! Returns a list of preconditions for this format.
  virtual PreconditionList get_preconditions(MscPtr msc) const;
  //! export MSC document
  virtual int save_msc(std::ostream& stream, const std::wstring &name,
    const MscPtr& selected_msc, const std::vector<MscPtr>& msc = std::vector<MscPtr>());

  void print_keyword_warning(const std::string& name)
  {
    if(m_warned_names.find(name) == m_warned_names.end())
    {
      print_report(RS_WARNING, stringize()
        << L"Warning: Name '" << TOWSTRING(name) << "' is a reserved keyword.");
      m_warned_names.insert(name);
    }
  }
protected:
  int save_msc(std::ostream& stream, const MscPtr& msc);
  // note: insertion to m_printing must not invalidate iterators
  std::list<MscPtr> m_printing;

  //! export a basic MSC drawing
  int save_bmsc(std::ostream& stream, const BMscPtr& bmsc);
  //! export a HMSC drawing
  int save_hmsc(std::ostream& stream, const HMscPtr& hmsc);

  void print_element_attributes(std::ostream& stream,
    const MscElementPtr& element);

  // print global comments
  // note: used when the comment is attached to BMsc or HMsc
  template<class C>
  void print_texts(std::ostream& stream, const boost::intrusive_ptr<C>& commentable)
  {
    for(CommentPtrSet::const_iterator cpos = commentable->get_comments().begin();
      cpos != commentable->get_comments().end(); cpos++)
    {
      print_element_attributes(stream, *cpos);
      stream << "text '" << VALID_CHARACTER_STRING((*cpos)->get_text()) << "';" << std::endl;
    }
  }

  // print local comments
  // note: used when the comment is attached to Event, HMscNode, etc.
  template<class C>
  void print_comments(std::ostream& stream, const boost::intrusive_ptr<C>& commentable)
  {
    for(CommentPtrSet::const_iterator cpos = commentable->get_comments().begin();
      cpos != commentable->get_comments().end(); cpos++)
    {
      print_element_attributes(stream, *cpos);
      stream << " comment '" << VALID_CHARACTER_STRING((*cpos)->get_text()) << "'";
    }
  }

  void print_event(std::ostream& stream,
    PtrIDMap<MscMessagePtr>& message_id_map, const EventPtr& event);
  void print_time_relations(std::ostream& stream,
    PtrIDMap<EventPtr>& event_id_map, const EventPtr& event);
  void print_time_relations1(std::ostream& stream,
    const ReferenceNodePtr& reference_node, TimeRelationRefNodePtrSet relations);
  void print_time_relations2(std::ostream& stream,
    PtrIDMap<HMscNodePtr>& node_id_map, const std::string& title,
    const ReferenceNodePtr& reference_node, TimeRelationRefNodePtrSet relations);

  std::set<std::string> m_warned_names;
};

#endif /* _Z120_H */

// $Id: z120.h 536 2009-12-18 12:21:45Z gotthardp $
