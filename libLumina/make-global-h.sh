#!/bin/sh

# create global.h

echo "#ifndef PREFIX
  #define PREFIX QString(\"${1}\")
#endif" > global.h
