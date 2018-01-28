#!/bin/sh

THEME=${1}
COLOR=${2}
if [ "z" == "${THEME}z" ] || [ "z" == "${COLOR}z" ]; then
  echo "Usage: changeFillColor.sh <ThemeDir> <HexColor>"
  return 1;
fi

echo "Changing Fill color: ${THEME} files to ${COLOR}"
for i in `find ${THEME} | grep svg`
do
  echo "Found File ${i}"
  sed -i '' "s/ fill=\"#000000\" / fill=\"${COLOR}\" /g" ${i}
done
