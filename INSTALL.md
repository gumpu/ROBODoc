# BUILD

ROBODoc can be build under Linux and Windows. It might build under MacOS but I
do not have access to this OS so cannot test this.

## Linux

Building RODODoc requires a C compiler and make. This these can be installed
on a Debian based system with:

    sudo apt-get install build-essential

Once installed there are two ways of building ROBODoc under Linux. Which one
to choose depends on how you got ROBODoc sources of.  If you downloaded
the official source distribution (robodoc-4.99.45.zip) you can build ROBODoc using:

    sudo apt install unzip
    unzip robodoc-4.99.45.zip
    cd robodoc-4.99.45
    ./configure
    make

If you got the sources using a git clone or by downloading release as a zip
file then you can build ROBODoc using

    ./build_robodoc.sh

In both cases you can find the executable in `Source/robodoc`.
If you place this file somewhere in you path, for instance in the directory
`~/bin/` you are ready to go.


## Windows

TODO

# INSTALL

TODO

