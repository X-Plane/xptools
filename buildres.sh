#!/bin/bash

# objcopy won't produce output for 0 byte (i.e. empty) files

wd=$PWD;

if [ "$1" = "" ]
then
  echo "usage: 'buildres.sh <path to resources> <outfile> <architecture>'"
  exit 1;
fi

if [ "$2" = "" ]
then
  echo "usage: 'buildres.sh <path to resources> <outfile> <architecture>'"
  exit 1;
fi

if [ "$3" = "" ]
then
  echo "usage: 'buildres.sh <path to resources> <outfile> <architecture>'"
  exit 1;
fi

rm -f $2
find $1 -wholename "*.git*" -prune -o -type f \( -iname "*" \) -print |
while read sourcefile
do
  fn=$(basename "$sourcefile")
  dn=$(dirname "$sourcefile")
  if [ "$3" = "i386" ]
  then
    echo "processing: $fn (i386)"
    cd "$dn" && objcopy -I binary -O elf32-i386 -B i386 "$fn" "${fn}.intr" && cd "$wd"
  fi
  if [ "$3" = "x86_64" ]
  then
    echo "processing: $fn (x86_64)"
    cd "$dn" && objcopy -I binary -O elf64-x86-64 -B i386:x86-64 "$fn" "${fn}.intr" && cd "$wd"
  fi
done
if [ "$3" = "i386" ]
then
  echo "linking: $1/$2 (i386)"
  find $1 -wholename "*.git*" -prune -o -type f \( -iname "*.intr" \) -print | awk -F '\n' '{printf "\"%s\" ",$1}' | xargs ld -melf_i386 -z nostdlib -r -o $2
fi
if [ "$3" = "x86_64" ]
then
  echo "linking: $1/$2 (x86_64)"
  find $1 -wholename "*.git*" -prune -o -type f \( -iname "*.intr" \) -print | awk -F '\n' '{printf "\"%s\" ",$1}' | xargs ld -melf_x86_64 -z nostdlib -r -o $2
fi

echo "removing intermediate files"
find $1 -wholename "*.git*" -prune -o -type f \( -iname "*.intr" \) -print | awk -F '\n' '{printf "\"%s\" ",$1}' | xargs rm -f
exit 0;
