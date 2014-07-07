#!/bin/sh
if ["${config}"="release64" && "$CC"="clang"] then exit 0 fi

cd /tmp
git clone https://github.com/doxygen/doxygen.git
cd doxygen
./configure
make
make distclean
git pull
./configure
make
sudo make install

cd "${TRAVIS_BUILD_DIR}"
doxygen build/Doxyfile
