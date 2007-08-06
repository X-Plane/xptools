#!/bin/sh

### WED

rm ../XPWebSite/scenery/tools/wed_mac_$1.zip
rm ../XPWebSite/scenery/tools/wed_win_$1.zip

cd build/Release/
mv WED.app WED_native.app
cp -R WED_native.app WED.app
unzip -p WEDi386.zip > WED_i386
unzip -p WEDppc.zip > WED_ppc
lipo -create WED_i386 WED_ppc -output WED.app/Contents/MacOS/WED 
chmod a+x WED.app/Contents/MacOS/WED

zip -r ../../../XPWebSite/scenery/tools/wed_mac_$1.zip WED.app

rm -r WED_ppc
rm -r WED_i386
rm -r WED.app
mv WED_native.app WED.app

cd ../..

cd SceneryToolsWin/release/

zip -r ../../../XPWebsite/scenery/tools/wed_win_$1.zip WED.exe

