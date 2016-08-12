#!/bin/bash

g++ -o marketSrv ../src/market/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lthostmduserapi -lhiredis -ljsoncpp
g++ -o tradeSrv ../src/trade/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lthosttraderapi -lhiredis -ljsoncpp


