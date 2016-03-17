#!/bin/sh
cd ../MCC/cocos2dx/proj.linux
scons
cd ../../CocosDenshion/proj.linux
make
cd ../../extensions/proj.linux
make
cd ../../scripting/lua/proj.linux
make
cd ../../../../YYFramework/
scons
