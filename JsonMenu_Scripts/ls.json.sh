#!/bin/sh
DIR=${1}
if [ "$1" == "" ] ; then
  DIR=`pwd`
fi

ls "${DIR}" > /tmp/.tmp.lines.$$ 
while read name
do
  if [ -n "${OUT}" ] ; then
    OUT="${OUT},"
  fi
  if [ -d "${DIR}/${name}" ] ; then
    OUT="${OUT} \"${name}\" : { \"type\" : \"jsonmenu\", \"exec\" : \"${0} \\\"${DIR}/${name}\\\"\", \"icon\":\"folder\"}"
  else
    OUT="${OUT} \"${name}\" : { \"type\" : \"item\", \"icon\":\"unknown\", \"action\" : \"xdg-open \\\"${DIR}/${name}\\\"\"}"
  fi
done < /tmp/.tmp.lines.$$
rm /tmp/.tmp.lines.$$
echo "{ ${OUT} }"
