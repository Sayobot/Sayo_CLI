#!/bin/bash

g++ --std=c++17 \
  -O2 -DRELEASE -DNDEBUG \
  -I./jsoncpp/include \
  -I./hidapi/hidapi \
  -I./inc \
  o2_protocol.cpp \
  main.cpp \
  src/http.cpp \
  src/tools.cpp \
  -I/usr/include \
  ./jsoncpp/build/lib/libjsoncpp.a \
  -lhidapi-hidraw \
  -lpthread \
  -o Sayo_CLI_Linux

