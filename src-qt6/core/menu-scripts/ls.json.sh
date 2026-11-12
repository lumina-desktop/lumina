#!/bin/sh
DIR=${1}
if [ "$1" = "" ] ; then
  DIR=`pwd`
fi

OUT="${OUT} \"   Open Directory\" : { \"type\" : \"item\", \"icon\":\"document-open\", \"action\" : \"xdg-open \\\"${DIR}\\\"\"}"
ls "${DIR}" > /tmp/.tmp.lines.$$ 
while read name
do
  OUT="${OUT},"
  if [ -d "${DIR}/${name}" ] ; then
    OUT="${OUT} \"${name}\" : { \"type\" : \"jsonmenu\", \"exec\" : \"${0} \\\"${DIR}/${name}\\\"\", \"icon\":\"folder\"}"
  else
    OUT="${OUT} \"${name}\" : { \"type\" : \"item\", \"icon\":\"unknown\", \"action\" : \"xdg-open \\\"${DIR}/${name}\\\"\"}"
  fi
done < /tmp/.tmp.lines.$$
rm /tmp/.tmp.lines.$$
echo "{ ${OUT} }"
