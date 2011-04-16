#! /usr/bin/env bash

. ./gen_checker.sh || exit 2
. ./checker_list || exit 3

function count(){
	RET=1
	for i in $CHECK
	do
		RET=$(($RET+2))
	done
	echo $RET
}

# Generate module.cpp
echo "Generating module.cpp"
echo -e "/*
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
 */
" > module.cpp

for i in $CHECK
do
	echo "#include \"check/pycheck/$(tolower ${i})_checker_visio.h\"" >> module.cpp
done

echo -e "
// module initialization function
// note: the Visio add-on searches for a function of this name
extern \"C\" SCPYCHECK_EXPORT
Checker** init_checkers()
{
  Checker **result = new Checker* [$(count)];
" >> module.cpp
CNT=0
for i in $CHECK
do
	# Generate .h and .cpp checker files
	echo "Generating .h and .cpp files for $i"
	genhcpp "$i"
	echo "  result[${CNT}] = new PyB${i}Checker();" >> module.cpp
	CNT=$(($CNT+1))
	echo "  result[${CNT}] = new PyH${i}Checker();" >> module.cpp
	CNT=$(($CNT+1))
done

echo "  result[${CNT}] = NULL;" >> module.cpp
echo "  return result;" >> module.cpp
echo "}" >> module.cpp


#Generate CMakeLists.txt
echo "Generating CMakeLists.txt"
echo -e "ADD_LIBRARY(scpycheck SHARED
  export.h
  module.cpp" > CMakeLists.txt

for i in $CHECK
do
	echo "  $(tolower ${i})_checker_visio.h" >> CMakeLists.txt
	echo "  $(tolower ${i})_checker_visio.cpp" >> CMakeLists.txt
done

# Following might be useful once:
#ADD_CUSTOM_TARGET(PyCheck
#  SOURCES deadlock_checker.py livelock_checker.py acyclic_checker.py fifo_checker.py)


echo -e ")

TARGET_LINK_LIBRARIES(scpycheck
  scpyconv
  scmsc
  \${PYTHON_LIBRARIES}
  \${PARSER_LIBRARIES}
)

INCLUDE_DIRECTORIES(\${PYTHON_INCLUDE_PATH})

INSTALL(TARGETS scpycheck
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)" >> CMakeLists.txt
