#!/bin/bash

g++ --std=c++17 \
  -O2 -DRELEASE -DNDEBUG \
  -I/usr/include/json \
  -I/usr/include/hidapi \
  -I/usr/include/jsoncpp \
  -I./inc \
  o2_protocol.cpp \
  main.cpp \
  src/http.cpp \
  src/tools.cpp \
  -I/usr/include \
  -ljsoncpp \
  -lhidapi-hidraw \
  -lpthread \
  -o Sayo_CLI_Linux

