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
 * $Id: engmann.cpp 438 2009-10-25 15:32:55Z gotthardp $
 */

#include <string>
#include <iostream>
#include <fstream>
#include <string.h>

#include "data/engmann/engmann.h"
#include "data/msc.h"

struct TToken
{
  enum TType
  {
    T_WORD,
    T_COLON,
    T_EOF
  }
  type;

  std::wstring text;

  TToken(TType t)
    : type(t) { }
  TToken(TType t, const std::wstring& s)
    : type(t), text(s) { }
};

static void skip_whitespace(std::istream& stream)
{
  while(stream.good())
  {
    int ch = stream.get();
    if(!isspace(ch) || ch == '\n')
    {
      stream.putback(ch);
      break;
    }
  }
}

static void trim(std::wstring& str)
{
  std::wstring::size_type pos = str.find_last_not_of(' ');
  if(pos != std::wstring::npos)
  {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != std::wstring::npos)
      str.erase(0, pos);
  }
  else
    str.erase(str.begin(), str.end());
}

static std::wstring read_line(std::istream& stream)
{
  std::wstring result;
  // skip leading whitespace
  skip_whitespace(stream);
  // read the text
  int ch;
  while(stream.good() && (ch = stream.get()) != '\n')
    result.push_back(ch);

  // remove leading and trailing whitespace
  trim(result);

  return result;
}

static void skip_line(std::istream& stream)
{
  // skip this line
  while(stream.good() && stream.get() != '\n')
  { }
}

static int is_word_char(int ch)
{
  return !isspace(ch) &&
    ch != ':' && ch != '#';
}

static TToken get_token(std::istream& stream)
{
  while(stream.good())
  {
    char ch = stream.get();
    if(is_word_char(ch))
    {
      std::wstring word;
      do
      {
        word.push_back(ch);
        if(!stream.good())
          return TToken(TToken::T_WORD, word);

        ch = stream.get();
      }
      while(is_word_char(ch));

      stream.putback(ch);
      return TToken(TToken::T_WORD, word);
    }
    else if(ch == ':')
      return TToken(TToken::T_COLON);
    else if(ch == '#')
      skip_line(stream);
  }

  return TToken(TToken::T_EOF);
}

typedef std::map<std::wstring,EventAreaPtr> InstanceAreaMap;
static EventAreaPtr get_instance_area(const InstanceAreaMap& instances, const std::wstring name)
{
  InstanceAreaMap::const_iterator pos = instances.find(name);
  if(pos == instances.end())
    return NULL;

  return pos->second;
}

std::vector<MscPtr> Engmann::load_msc(const std::string &filename)
{
  std::vector<MscPtr> result;

  InstanceAreaMap instances;

  static const Coordinate x_step = 30; // distance between instances [mm]
  Coordinate current_x = 10;

  static const Coordinate y_step = 7.5; // distance between messages [mm]
  Coordinate current_y = 7.5;

  std::ifstream stream;
  stream.open(filename.c_str(), std::ifstream::in);
  if(!stream.good())
  {
    print_report(RS_ERROR,
      stringize() << "Cannot open file '" << TOWSTRING(filename) << "'.");
    return result;
  }

  BMscPtr bmsc = new BMsc();
  while(stream.good())
  {
    TToken token = get_token(stream);
    if(token.type == TToken::T_WORD)
    {
      TToken next_token = get_token(stream);
      // title | node
      if(next_token.type == TToken::T_COLON)
      {
        if(wcscmp(token.text.c_str(), L"title") == 0)
        {
          bmsc->set_label(read_line(stream));
        }
        else if(wcscmp(token.text.c_str(), L"node") == 0)
        {
          TToken ip_address = get_token(stream);
          TToken node_name = get_token(stream);

          // create new instance
          InstancePtr inst = new Instance(node_name.text);
          inst->set_width(10);
          inst->set_line_begin(MscPoint(current_x,5));
          bmsc->add_instance(inst);

          EventAreaPtr area = new StrictOrderArea();
          inst->add_area(area);

          instances[node_name.text] = area;
          current_x += x_step;
        }
        else
        {
          // error
          skip_line(stream);
        }
      }
      // message
      else if(next_token.type == TToken::T_WORD)
      {
        std::wstring message_label = read_line(stream);

        EventAreaPtr from_area = get_instance_area(instances, token.text);
        EventAreaPtr to_area = get_instance_area(instances, next_token.text);
        if(from_area != NULL && to_area != NULL)
        {
          CompleteMessagePtr message = new CompleteMessage(message_label);

          EventPtr from_event = from_area->add_event();
          from_event->set_position(MscPoint(0,current_y));

          EventPtr to_event = to_area->add_event();
          to_event->set_position(MscPoint(0,current_y));

          message->glue_events(from_event, to_event);
        }
        current_y += y_step;
      }
      else
      {
        // error
        skip_line(stream);
      }
    }
    else
    {
      // error
      skip_line(stream);
    }
  }

  stream.close();

  // set length of the instance lines
  for(InstanceAreaMap::const_iterator pos = instances.begin();
    pos != instances.end(); pos++)
  {
    InstancePtr inst = pos->second->get_instance();
    Coordinate my_x = inst->get_line_begin().get_x();
    Coordinate my_y = inst->get_line_begin().get_y();

    inst->set_line_end(MscPoint(my_x,my_y+current_y));
  }

  result.push_back(bmsc);
  return result;
}

ImportFormatter::TransformationList Engmann::get_transformations(MscPtr msc) const
{
  ImportFormatter::TransformationList result;
  result.push_back(L"Beautify");
  return result;
}

// $Id: engmann.cpp 438 2009-10-25 15:32:55Z gotthardp $
