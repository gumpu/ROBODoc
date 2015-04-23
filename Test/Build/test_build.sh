#!/usr/bin/bash

# TODO Fetch from website
version=4.99.43
tarball=../../robodoc-4.99.43.tar.gz
builddir=/tmp/robo

echo $tarball

# Drop out on the first error we encounter
set -e

rm -rf $builddir
mkdir $builddir
cp $tarball $builddir/
cd $builddir
tar -zxf robodoc*.gz
cd robodoc-$version
./configure --prefix=$HOME
make
make install
build_version=`./Source/robodoc --version`

echo "Build was successful."
echo "Executable reports to be $build_version"

# vi: spell spl=en
