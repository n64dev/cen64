#!/bin/sh
#Devin's Cen64 Linux Compile Script.

GetVersionFromFile()
{
	VERSION=`cat $1 | tr "\n" ' ' | sed s/.*VERSION.*=\ // `
}

	if [ -f /etc/redhat-release ] ; then
		sudo yum install git libglfw-devel libalut-devel
	elif [ -f /etc/debian_version ] ; then
		sudo apt-get install git libglfw-dev libalut-dev
	fi

cd ../
git submodule init
git pull
git submodule update
make -e debug
