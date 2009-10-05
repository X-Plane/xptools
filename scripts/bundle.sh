#!/bin/bash

# usage: bundle.sh product platform version

case "$2" in
"")
	echo "Second parameter must be mac|win|lin"
	exit 1
	;;
mac)
	build_dir=../build/Release
	suffix=Mac
	tsuffix=""
	asuffix=".app"
	;;
win)
	build_dir=../build/Mingw/release_opt
	suffix=Win
	tsuffix=".exe"
	asuffix=".exe"
	;;
lin)
	build_dir=../build/Linux/release_opt
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
	src_files="$build_dir/XPlaneSupport$suffix.p ../src/AC3DPlugins/README ../src/AC3DPlugins/XPlaneSupport.tcl"
	if [ "$2" == "lin" ]; then
		src_files="$src_files $build_dir/XPlaneSupport$suffix.p.debug"
	fi
	;;
meshtool)
	out_file="meshtool_"$2"_"$3".zip"
	src_files="$build_dir/MeshTool$tsuffix ../src/MeshTool/README.meshtool"
	if [ "$2" == "lin" ]; then
		src_files="$src_files $build_dir/MeshTool.debug"
	fi
	;;
wed)
	out_file="wed_"$2"_"$3".zip"
	src_files="$build_dir/WED$asuffix ../src/WEDResources/WEDManual.pdf"
	;;
esac

rm $out_file
zip -j $out_file $src_files
scp $out_file bsupnik@dev.x-plane.com:/shared/download/tools/
