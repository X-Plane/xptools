#!/bin/sh
if [ $# != 3 ]; then
	echo Usage: "$0 <west> <south> <dirname>"
	exit 1
fi


tool=none
datadir=none
artdir=none

. ./config.sh

if [ "$tool" == "none" ]; then
	echo "No tool set."
	exit 1
fi

if [ "$datadir" == "none" ]; then
	echo "No datadir set."
	exit 1
fi

if [ "$artdir" == "none" ]; then
	echo "No artdir set."
	exit 1
fi


west=$1
south=$2
east=`expr $west + 1`
north=`expr $south + 1`
folder=`./genpath folder $west $south`
file=`./genpath file $west $south`
output="$3Earth nav data/$folder/$file.dsf"
hydro_cmd=
hydro_file=
logdir=$datadir/logs/$folder/$file.txt

if [ -e "$output" ]; then
	echo "Skipping $output - already created."
	exit 0
fi

if [ ! -e "$datadir/DEM output-earth/$folder/$file.oz" ]; then
	echo "skipping - "$datadir/DEM output-earth/$folder/$file.oz" - SRTM does not exist."
	exit 0
fi

xes_dir=$datadir/blend_xes/$folder/$file.xes
if [ ! -e $xes_dir ]; then
xes_dir=$datadir/us_xes/$folder/$file.xes
fi

if [ ! -e $xes_dir ]; then
	xes_dir=$datadir/world_xes/$folder/$file.xes
	if [ -e $datadir/swbd/$folder/$file.shp ]; then
		hydro_cmd="-hydro $datadir/swbd/$folder/$file.shp -hydrosimplify"
	else
		hydro_cmd="-hydro -hydrosimplify"
	fi
fi

if [ ! -e $xes_dir ]; then
	echo "skipping - $xes_dir does npt exist."
	exit 0
fi

apt_cmd=-apt
apt_file=$datadir/apts/$folder/$file.apt

if [ ! -e $apt_file ]; then
echo No airport file $apt_file
apt_cmd=
apt_file=
fi

obs_cmd=-obs
obs_mode=deg
obs_file=$datadir/faa_obs/$file.obs

if [ ! -e $obs_file ]; then
obs_cmd=
obs_mode=
obs_file=
fi

$tool \
	-noprogress \
	-extent $west $south $east $north \
	-load config/global_climate_smooth_rain.xes \
	-crop \
	-load $xes_dir \
	-glcc $datadir/glcc/lu_new.raw oge2_import.txt \
	-oz $datadir/"DEM output-earth"/$folder/$file.oz \
	$obs_cmd $obs_mode $obs_file \
	$apt_cmd $apt_file \
	-bbox \
	-simplify \
	-validate \
	-spreadsheet "$artdir"/spreadsheets/master_terrain.txt \
	-upsample \
	-calcslope \
	$hydro_cmd \
	-derivedems \
	-buildroads \
	-burnapts \
	-zoning \
	-calcmesh \
	-assignterrain \
	-exportdsf "$3" | tee $logdir


#	-instobjs \
#	-removedupes \
#	-forests \
