#!/bin/sh

PROJECT_NAME=qt5ct
TX_CONFIG="../.tx/config"

echo "[main]" > ${TX_CONFIG}
echo "host = https://www.transifex.com" >> ${TX_CONFIG}
echo "" >> ${TX_CONFIG}

for tr_dir in `find ../src/ -type d -name "translations"`
do

     plug_name=`echo $tr_dir | cut -d '/' -f 3`

     echo "Updating ${plug_name}"


     file_filter=`echo ${tr_dir} | sed 's/..\///'`
     echo "[${PROJECT_NAME}.${plug_name}]" >> ${TX_CONFIG}
     echo "file_filter = ${file_filter}/${plug_name}_<lang>.ts" >> ${TX_CONFIG}
     echo "source_lang = en" >> ${TX_CONFIG}
     echo "source_file = ${file_filter}/${plug_name}_en.ts" >> ${TX_CONFIG}
     echo "type = QT" >> ${TX_CONFIG}
     echo "" >> ${TX_CONFIG}

done


for RESOURCE_NAME in qt5ct
do
    echo "[${PROJECT_NAME}.${RESOURCE_NAME}-desktop]" >> ${TX_CONFIG}
    echo "file_filter = src/qt5ct/desktop-translations/${RESOURCE_NAME}_<lang>.desktop.in" >> ${TX_CONFIG}
    echo "source_lang = en" >> ${TX_CONFIG}
    echo "source_file = src/qt5ct/${RESOURCE_NAME}.desktop" >> ${TX_CONFIG}
    echo "type = DESKTOP" >> ${TX_CONFIG}
    echo "" >> ${TX_CONFIG}
done
