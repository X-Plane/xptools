#!/bin/sh

### DSF2Text
rm ../XPWebSite/scenery/tools/dsf2text$1.mac.zip
rm ../XPWebSite/scenery/tools/dsf2text$1.win.zip
cd XPTools\ Mac
zip -r ../../XPWebSite/scenery/tools/dsf2text$1.mac.zip DSF2Text.app DSFTool -x \*.DS_Store \*.xSYM
cd ..
cd XPTools\ Win
zip -r ../../XPWebSite/scenery/tools/dsf2text$1.win.zip DSF2Text.exe DSFTool.exe -x \*.DS_Store \*.xSYM
cd ..
cd src/DSFTools
zip ../../../XPWebSite/scenery/tools/dsf2text$1.win.zip README.dsf2text
zip ../../../XPWebSite/scenery/tools/dsf2text$1.mac.zip README.dsf2text
cd ../..
