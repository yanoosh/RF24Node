# RF24NodeRaspberry

## How to build
Install lib in global dir /usr/local/include/* by script "build_rf24libs.sh"

## Author
Project was forked from Sony Arouje (https://github.com/sonyarouje)

## Install rf24lib
curl -sL https://raw.githubusercontent.com/yanoosh/RF24NodeRaspberry/master/build_rf24libs.sh | sudo -E bash -

## Config RPi user

enable spi on RPI
uncoment in `/boot/config.txt` line `dtparam=spi=on` or `sudo raspi-config`

`sudo useradd -M user_running_program`  
`sudo usermod -L user_running_program`  
`sudo usermod -a -G spi user_running_program`  
`sudo usermod -a -G gpio user_running_program`  
