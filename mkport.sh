#!/bin/sh
# Helper script which will create the port / distfiles
# from a checked out git repo

# Set the distfile URL we will fetch from
DURL="http://www.pcbsd.org/~kris/software/"

get_last_rev_git()
{
   oPWD=`pwd`
   cd "${1}"
   rev=0
   rev=`git log -n 1 --date=raw | grep 'Date:' | awk '{print $2}'`
   cd $oPWD
   if [ $rev -ne 0 ] ; then
     echo "$rev"
     return 0
   fi
   return 1
}

if [ -z "$1" ] ; then
   echo "Usage: ./mkports.sh <outdir>"
   exit 1
fi

if [ ! -d "${1}" ] ; then
   echo "Invalid directory: $1"
   exit 1
fi

portsdir="${1}"
distdir="${1}/distfiles"
if [ ! -d "$portsdir" ] ; then
  mkdir ${portsdir}
fi
if [ ! -d "$portsdir/sysutils" ] ; then
  mkdir ${portsdir}/sysutils
fi
if [ ! -d "$distdir" ] ; then
  mkdir ${distdir}
fi

REV=`get_last_rev_git "."`

# Make the dist files
rm ${distdir}/lumina*.tar.bz2 2>/dev/null
echo "Creating lumina dist file for version: $REV"
cd ..
tar cvjf ${distdir}/lumina-${REV}.tar.bz2 --exclude .git lumina 2>/dev/null
cd lumina

# Copy ports files
rm -rf ${portsdir}/x11/lumina 2>/dev/null
cp -r port-files ${portsdir}/x11/lumina

# Set the version numbers
sed -i '' "s|CHGVERSION|${REV}|g" ${portsdir}/x11/lumina/Makefile

# Set the mirror to use
sed -i '' "s|http://www.pcbsd.org/~kris/software/|${DURL}|g" ${portsdir}/x11/lumina/Makefile

# Create the makesums / distinfo file
cd ${distdir}
sha256 lumina-${REV}.tar.bz2 > ${portsdir}/x11/lumina/distinfo
echo "SIZE (lumina-${REV}.tar.bz2) = `stat -f \"%z\" lumina-${REV}.tar.bz2`" >> ${portsdir}/x11/lumina/distinfo
