#!/bin/bash

# change below to suit your needs ...

from1="Apts_W/Earth nav data/"
from2="Apts_E/Earth nav data/"

to="Global Airports/Earth nav data/"

echo Cleaning out destination DSFs
rm -rf "${to}"*0/
echo Copying files from $from1 ...
cp -r "${from1}"* "$to"
echo Copying w/backups files from $from2 ...
cp -rb "${from2}"* "$to"

echo duplicated tile check:
ls -- "${to}"*/*~
read -rsp $'if any files ending in ~ got listed above we have a BIG problem !!!\n' -n1 key

echo Combining the apt.dat found in all source directories ...
head -n -1 "${from1}apt.dat" >"${to}apt.dat"
tail -n +3 "${from2}apt.dat" >>"${to}apt.dat"
rm "${to}apt.dat~"

# someday, WED might compress at export. It already reads compressed dsf's ...
echo Compressing DSFs ...
script="`pwd`/7z_helper.sh"

cat << EOF > "$script"
#!/bin/sh
touch -t 202101010000.00 -- \$1
echo compressing -- \$1
7z a -- \$1.7z \$1 >/dev/null
mv -f -- \$1.7z \$1
EOF

chmod u+x "$script"

find "$to" -name '*.dsf' -size +4090c -execdir "$script" {} \;

# only needed for upload to the .org
# 7z a GlobalAirports_`date +%b%y`.7z Global\ Airports
