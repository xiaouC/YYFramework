#!/bin/sh
cd ../MCC/cocos2dx/proj.linux
scons -c
cd ../../CocosDenshion/proj.linux
make clean
cd ../../extensions/proj.linux
make clean
cd ../../scripting/lua/proj.linux
make clean
cd ../../../../YYFramework
scons -c
