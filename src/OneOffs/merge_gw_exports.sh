#!/bin/bash

from1="Apts_W/Earth nav data/"
from2="Apts_E/Earth nav data/"
to="Global Airports/Earth nav data/"

echo Cleaning out destination DSFs
rm -rf "${to}"*0/

echo Synching $from1 ...
rsync -ab "${from1}"* "$to"
echo Synching $from2 ...
rsync -ab "${from2}"* "$to"

echo duplicated tile check:
ls -- "${to}"*/*dsf~
echo if any files get listed above we have a BIG problem !!!

echo Combineing apt.dat
head -n -1 "${from1}apt.dat" >"${to}apt.dat"
tail -n +2 "${from2}apt.dat" >>"${to}apt.dat"
rm "${to}apt.dat~"

read -rsp $'Ready to compress DSFs, press any key to continue...\n' -n1 key

find "$to" -name '*.dsf' -size +4000c -execdir xp7zip {} \;

7z a GlobalAirports_`date +%b%Y`.7z Global\ Airports
