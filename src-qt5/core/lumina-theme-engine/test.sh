#!/bin/sh

MYDIR=$(dirname $0)

mkdir -p ${MYDIR}/src/qt5ct/styles
ln -s "../../qt5ct-style/libqt5ct-style.so" "${MYDIR}/src/qt5ct/styles/"
export QT_DEBUG_PLUGINS=1
export QT_QPA_PLATFORMTHEME=qt5ct
export QT_QPA_PLATFORM_PLUGIN_PATH=${MYDIR}/src/qt5ct-qtplugin
${MYDIR}/src/qt5ct/qt5ct
rm -rf ${MYDIR}/src/qt5ct/styles/
