#!/bin/sh
#date "+dump/wed_mac_%m%d%y" | xargs set
mkdir dump/$1
cp -r docs dump/$1/docs
cp -r SceneryTools/config dump/$1/config
/Developer/Tools/CpMac SceneryTools/WorldEditor dump/$1/WorldEditor
/Developer/Tools/CpMac SceneryTools/WorldEditor_d dump/$1/WorldEditor_d
#/Developer/Tools/CpMac ../design++/World-Maker dump/$1/World-Maker
#/Developer/Tools/CpMac ../design++/X-Plane dump/$1/X-Plane
