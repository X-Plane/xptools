#!/bin/sh
bison -ydv vrml.y
mv y.tab.c vrml.tab.cpp
mv y.tab.h vrml.tab.h
flex -I vrml.l
mv lex.yy.c vrml.yy.cpp
