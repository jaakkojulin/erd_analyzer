#!/bin/bash
echo "This script is intended for the software maintainer only! You may proceed
at your own risk. Note that I may \"accidentally\" remove some files when I 
clean the working directory.

You have been warned.

"

do_release() {
    ./autogen.sh
    make clean;
    make distclean;
    git clean -f;
    rm -rf autom4te.cache/;
    git status;
    version=$(grep "^AC_INIT" < configure.ac|sed "s/AC_INIT(\[.*\].*\[\(.*\)\].*\[.*\].*/\1/");
    echo "CHECK THE STATUS OF THE REPOSITORY."
    read -p "About to tag the repo and create a bare (autogen.sh needs to be run) source package for version $version. Is this ok? ^C if not.";
    git tag $version
    #git archive -o "../histo_2d-$version-bare.tar.gz" --prefix="histo_2d-$version/" HEAD;
    ./autogen.sh
    ./configure
    make dist
    shasum -a 256 erd_analyzer-$version.tar.gz
}


read -p "Do you wish to continue? " reply
case $reply in 
    [Yy]* ) do_release;;
esac

exit;
