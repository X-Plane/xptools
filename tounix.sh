#!/bin/bash

# Janos says, we don't use this script when having a workflow containing git and cvs
# as it causes to much trouble when merging from one scm to another

# usage: tounix [path]

# when no path is given the current working directory is used.
# the script recurses over the given (or current) path (except .git directories)
# and modifies all *.c, *.cp, *.cpp, *.h which are regular files and contain text
# only. it won't process files which names contain colons or backslashes
# (quotes and spaces in file or directory names will work)

#find $1 -wholename "*.git*" -prune  -o -type f \( -iname "Makefile" -o -iname "*.cpp" -iname "*.cxx" -o -iname "*.c" -o -iname "*.h" \) -print0 | xargs -0 file | grep -Z -i text | awk -F ':' '{printf "%s\n",$1}' |
#while read sourcefile
#do
#  fn=$(basename "$sourcefile")
#  echo "processing: $fn"

 # format sorce code to ansi style. ben doesn't like ansi style :-(
  #astyle --style=ansi -V -n "$sourcefile"

 # removing trailing whitespaces - script is buggered
  #sed -i -e 's/[[:space:]]*$//' "$sourcefile"

 # normalize line endings
  #dos2unix "$sourcefile"

 # normalize file mode. ben says CVS doesn't care, janos says git cares :-)
  #chmod 0644 "$sourcefile"
#done

#find $1 -wholename "*.git*" -prune  -o -type f \( -iname "*.sh" \) -print0 | xargs -0 file | grep -Z -i text | awk -F ':' '{printf "%s\n",$1}' |
#while read sourcefile
#do
#  fn=$(basename "$sourcefile")
#  echo "processing: $fn"
#  chmod 0744 "$sourcefile"
#done


