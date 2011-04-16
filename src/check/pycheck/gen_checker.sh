#! /usr/bin/env bash

function usage(){
	echo "You did not pass the argument"
	echo "Argument: word in following form:[A-Z][a-z]*, i.e. Deadlock but not DeAdlock nor deadlock"
	exit 1
}

function tolower(){
	echo $1 | tr [:upper:] [:lower:]
}

function toupper(){
	echo $1 | tr [:lower:] [:upper:]
}

function end(){
	echo "$2"
	return $1
}

function genhcpp(){
	test -z "$1" && usage
	TEST="$(tolower $1)_checker"
	FILE="${TEST}_visio"
	DEFINE="$(toupper $FILE)_H"
	CLASS="PyH${1}Checker"
	PROPERTY="PyH${1}Free"
	DOC=""
	CHM="true"

	echo "__HMSCCHECKER__"
	echo "TEST:     $TEST"
	echo "FILE:     $FILE"
	echo "DEFINE:   $DEFINE"
	echo "CLASS:    $CLASS"
	echo "PROPERTY: $PROPERTY"
	echo "DOC:      $DOC"

	#exit 0

	# .h file
	if test -f "${FILE}.h"
	then
		echo "Header file exists ($1)"
		return 1
	fi
	echo -e "/*
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
 *
 */

#ifndef $DEFINE
#define $DEFINE

#include <Python.h>
#include <vector>
#include <list>
#include <map>
#include \"data/msc.h\"
#include \"data/checker.h\"
#include \"data/pysc/py_conv.h\"
#include \"check/pycheck/export.h\"

class ${CLASS};

typedef boost::shared_ptr<${CLASS}> ${CLASS}Ptr;

class SCPYCHECK_EXPORT ${CLASS}: public Checker, public HMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static ${CLASS}Ptr m_instance;

public:

  ${CLASS}(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L\"${PROPERTY}\"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L\"${DOC}\"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfies the property.
   */
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes();
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return ${CHM};
  }
  
  ~${CLASS}(){}
  
  static ${CLASS}Ptr instance()
  {
    if(!m_instance.get())
      m_instance = ${CLASS}Ptr(new ${CLASS}());
    return m_instance;
  }

};" > "${FILE}.h"

	# .cpp file
	if test -f "${FILE}.cpp"
	then
		echo "Cpp file exists ($1)"
		return 1
	fi
	echo -e "/*
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

#include \"check/pycheck/${FILE}.h\"

${CLASS}Ptr ${CLASS}::m_instance;

std::list<HMscPtr> ${CLASS}::check(HMscPtr hmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv(\"pycheck.${TEST}\");
  }
  catch(int e){
    std::cout << \"Cannot initialize checker\" << std::endl;
    throw 15;
  }
  std::list<HMscPtr> ret = exp->checkHMsc(hmsc, chm);
  delete exp;
  return ret;
}

void ${CLASS}::cleanup_attributes()
{
}

Checker::PreconditionList ${CLASS}::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}" > "${FILE}.cpp"


	#TEST="$(tolower $1)_bchecker"
	#FILE="${TEST}_visio"
	#DEFINE="$(toupper $FILE)_H"
	CLASS="PyB${1}Checker"
	PROPERTY="PyB${1}Free"
	DOC=""
	CHM="true"

	echo "__BMSCCHECKER__"
	echo "TEST:     $TEST"
	echo "FILE:     $FILE"
	echo "DEFINE:   $DEFINE"
	echo "CLASS:    $CLASS"
	echo "PROPERTY: $PROPERTY"
	echo "DOC:      $DOC"

	#exit 0

	# .h file
	test ! -f "${FILE}.h" && end 1 "Header file does not exist"
	echo -e "
class ${CLASS};

typedef boost::shared_ptr<${CLASS}> ${CLASS}Ptr;

class SCPYCHECK_EXPORT ${CLASS}: public Checker, public BMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static ${CLASS}Ptr m_instance;

public:

  ${CLASS}(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L\"${PROPERTY}\"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L\"${DOC}\"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfies the property.
   */
  std::list<BMscPtr> check(BMscPtr hmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes();
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return ${CHM};
  }
  
  ~${CLASS}(){}
  
  static ${CLASS}Ptr instance()
  {
    if(!m_instance.get())
      m_instance = ${CLASS}Ptr(new ${CLASS}());
    return m_instance;
  }

};

#endif" >> "${FILE}.h"

	# .cpp file
	test ! -f "${FILE}.cpp" && end 1 "Cpp file does not exist"
	echo -e "

${CLASS}Ptr ${CLASS}::m_instance;

std::list<BMscPtr> ${CLASS}::check(BMscPtr bmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv(\"pycheck.${TEST}\");
  }
  catch(int e){
    std::cout << \"Cannot initialize checker\" << std::endl;
    throw 15;
  }
  std::list<BMscPtr> ret = exp->checkBMsc(bmsc, chm);
  delete exp;
  return ret;
}

void ${CLASS}::cleanup_attributes()
{
}

Checker::PreconditionList ${CLASS}::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}" >> "${FILE}.cpp"
}
