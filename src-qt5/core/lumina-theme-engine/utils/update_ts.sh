#!/bin/sh

LOCALES='en ru uk_UA zh_CN zh_TW tr cs pt_BR pt de pl fr it it_IT kk lt hu nl ja sk es_ES es he gl_ES sr_BA sr_RS bg nl_NL el sr ca ar'

echo "Locales: ${LOCALES}"

for tr_dir in `find ../src/ -type d -name "translations"`
do

     plug_name=`echo $tr_dir | cut -d '/' -f 3`

     echo "Updating ${plug_name}"

     ts_files=''
     qm_files=''

     for code in $LOCALES
     do
        ts_files="${ts_files} ${tr_dir}/${plug_name}_${code}.ts"
        if [ "${code}" != "en" ]; then
            qm_files="${qm_files} ${plug_name}_${code}.qm"
        fi
     done
       qrc_file="${tr_dir}/translations.qrc"


     /opt/qt56/bin/lupdate -no-obsolete -silent -extensions "cpp,ui" ${tr_dir}/../ -ts ${ts_files}



     echo "<!DOCTYPE RCC>" > $qrc_file
     echo "<RCC version=\"1.0\">" >> $qrc_file
     echo "  <qresource>" >> $qrc_file
     for qm_file in $qm_files
     do
         echo "    <file>${qm_file}</file>" >> $qrc_file;
     done
     echo "  </qresource>" >> $qrc_file
     echo "</RCC>" >> $qrc_file
done
