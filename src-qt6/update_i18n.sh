#!/bin/sh
#==========================
#This is just a simple script to go through the source tree and ensure that all the 
#  project translation files are up-to-date prior to committing to Git
#==========================
# NOTE: This does NOT need to be run prior to building Lumina - this is a developer tool only
#==========================
LUPDATE="/usr/lib/qt5/bin/lupdate"

for i in `find . | grep 'pro'`
do
  echo "Checking pro file: ${i}"
  if [ -z `basename ${i} | grep "lumina-"` ] ; then continue; fi  #Not a top-level project file
  echo "Updating translations:" `basename ${i}`
  ${LUPDATE} -no-obsolete ${i}
  if [ $? -ne 0 ] ; then
    echo " -- EXPERIENCED ERROR"
  fi
done
