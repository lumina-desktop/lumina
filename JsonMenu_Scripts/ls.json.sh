#!/bin/sh
cmd="ls $1"
OUT=""
for name in `${cmd}`; do
  if [ "${OUT}" != "" ] ; then
    OUT="${OUT},"
  fi
  OUT="${OUT} \"${name}\" : { \"type\" : \"item\", \"action\" : \"${name}\"}"
done
echo "{ ${OUT} }"
