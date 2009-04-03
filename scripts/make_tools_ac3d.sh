#!/bin/sh

### ac3d
rm ../XPWebSite/scenery/tools/ac3d_xplane_$1.zip
cd build/Release/XPlaneSupportMac.p.bundle/Contents/MacOS/
zip ../../../../../../XPWebSite/scenery/tools/ac3d_xplane_$1.zip XPlaneSupportMac.p
cd ../../../../../
cd AC3D\ Plugins/
zip -r ../../XPWebSite/scenery/tools/ac3d_xplane_$1.zip XPlaneSupportWin.p
cd ..
cd src/AC3DPlugins
zip ../../../XPWebSite/scenery/tools/ac3d_xplane_$1.zip README XPlaneSupport.tcl
cd ../..
