#!/bin/sh

#Quick script to ensure that the proper Linux distro template is selected within the libLumina build
#NOTE: Provide the Distro Name as the input for this script
#  it will use the general -Linux template if a specific template is not available for that distro

DISTRO=${1}
#Only perform the change if a distro-specific file is available
if [ -r LuminaOS-${DISTRO}.cpp ]; then
  sed -i "s/LuminaOS-Linux.cpp/LuminaOS-${DISTRO}.cpp/" Makefile
fi
