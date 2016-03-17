#!/bin/bash
cd src/protocol
./gen.sh
cd ../Script
./tolua.sh
