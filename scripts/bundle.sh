#!/bin/bash

# usage: bundle.sh product platform version

cd ..
home_dir=`pwd`

# home is root of XPTools
# build dir is rel path to final binary objs from home
# src_files are all bin objects relative to build dir
# doc_files are all others relative to home
# out_file is name of final, ends up in scripts
# WHY is it like this?   Mac apps are freaking folder trees - so we need to be IN the build dir 
# to recursively zip the app.  The "doc files" is everything else, assumed single file, so thus we
# can just zip -j to discard the paths.

case "$2" in
"")
	echo "Second parameter must be mac|win|lin"
	exit 1
	;;
mac)
	build_dir=build/Release
	suffix=Mac
	tsuffix=""
	asuffix=".app"
	;;
win)
	build_dir=build/Mingw/release_opt
	suffix=Win
	tsuffix=".exe"
	asuffix=".exe"
	;;
lin)
	build_dir=build/Linux/release_opt
	suffix=Lin
	tsuffix=""
	asuffix=""
	;;
esac

case "$1" in
"")
	echo "First parameter must be ac3d|meshtool"
	exit 1
	;;
ac3d)
	out_file="ac3d_plugin_"$2"_"$3".zip"
	src_files="XPlaneSupport$suffix.p"
	doc_files="$build_dir/XPlaneSupport$suffix.p src/AC3DPlugins/README src/AC3DPlugins/XPlaneSupport.tcl"
	if [ "$2" == "lin" ]; then
		doc_files="$doc_files $build_dir/XPlaneSupport$suffix.p.debug"
	fi
	;;
meshtool)
	out_file="meshtool_"$2"_"$3".zip"
	src_files="MeshTool$tsuffix"
	doc_files="src/MeshTool/README.meshtool"
	if [ "$2" == "lin" ]; then
		doc_files="$doc_files $build_dir/MeshTool.debug"
	fi
	;;
wed)
	out_file="wed_"$2"_"$3".zip"
	src_files="WED$asuffix"
	#doc_files="src/WEDResources/WEDManual.pdf"
	doc_files=""
	;;
tools)
	echo $build_dir/tools
	mkdir $build_dir/tools
	mv $build_dir/ObjConverter$tsuffix $build_dir/DDSTool$tsuffix $build_dir/DSFTool$tsuffix $build_dir/tools
	out_file="xptools_"$2"_"$3".zip"
	src_files="tools/ObjConverter$tsuffix tools/DDSTool$tsuffix tools/DSFTool$tsuffix ObjView$asuffix XGrinder$asuffix"
	doc_files="src/XPTools/README_FIRST src/XPTools/README.DDSTool src/XPTools/README.ObjConverter src/XPTools/README.ObjView src/XPTools/README.XGrinder"
	;;
esac

cd "$home_dir/scripts"
rm $out_file
cd ../$build_dir
zip -r "$home_dir/scripts/$out_file" $src_files
cd "$home_dir"
zip -j "$home_dir/scripts/$out_file" $doc_files
cd "$home_dir/scripts"
scp $out_file bsupnik@dev.x-plane.com:/shared/download/tools/
