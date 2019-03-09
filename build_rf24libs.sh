#!/bin/bash
#source https://github.com/ludiazv/node-nrf24
RF24GIT=https://github.com/tmrh20

echo "Check if RF24libs are installed..."
ldconfig -p | grep librf24.so
if [ $? -eq 0 ]; then
  read -p "Library seems to be installed. Do you want to rebuild last master version? [Y/n]" choice
  case "$choice" in
    n|N ) exit 0 ;;
    * ) echo "-----";;
  esac
fi

echo "Building RF24 libs supported..."
mkdir -p rf24libs
cd rf24libs
if [ -d RF24 ] ; then
cd RF24 ; git pull ; cd ..
else
git clone $RF24GIT/RF24.git RF24
fi
echo "=>RF24..."
cd RF24
./configure --driver=RPi
make
sudo make install
cd ..
echo "=>RF24Network..."
if [ -d RF24Network ] ; then
cd RF24Network ; git pull ; cd ..
else
git clone $RF24GIT/RF24Network.git RF24Network
fi
cd RF24Network
make
sudo make install
cd ..
cd ..
echo "done!"
