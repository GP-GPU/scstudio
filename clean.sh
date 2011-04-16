#! /usr/bin/env bash

function clean(){
	test -z "$1" && return 1
	pushd "$1"
	rm -rf CMakeFiles .svn/ CMakeCache.txt *.pyc Makefile CTestTestfile.cmake cmake_install.cmake
	for i in *
	do
		test -d "$i" && clean "$i"
	done
	popd
}

clean "$1"
rm -rf "$1/build/"
rm -rf "$1/dist/"	
