#!/bin/bash

g++ --std=c++17 \
  -O2 -DRELEASE -DNDEBUG \
  -I./hidapi/hidapi \
  -L./hidapi/src/mac -lhidapi \
  -I./jsoncpp/include \
  -L./jsoncpp/build/lib -ljsoncpp \
  -I./inc \
  o2_protocol.cpp \
  main.cpp \
  src/http.cpp \
  src/tools.cpp \
  -framework IOKit \
  -framework CoreFoundation \
  -framework AppKit \
  -o Sayo_CLI_Mac