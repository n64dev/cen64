#!/bin/sh
#Devin's Cen64 Linux Compile Script.

GetVersionFromFile()
{
	VERSION=`cat $1 | tr "\n" ' ' | sed s/.*VERSION.*=\ // `
}

	if [ -f /etc/redhat-release ] ; then
		DIST='RedHat'
	elif [ -f /etc/debian_version ] ; then
		DIST="Debian"
	fi

if [ "${DIST}" = "RedHat" ] ; then
sudo yum install git libglfw-devel libalut-devel
elif [ "${DIST}" = "Debian" ] ; then
sudo apt-get install git libglfw-dev libalut-dev
fi

cd ../
git submodule init
git pull
git submodule update
make clean && make -e debug
