# RF24NodeRaspberry

## How to build
1. clone RF24 from github
`git clone https://github.com/tmrh20/RF24.git RF24`
2. clone RF24Network from github
`git clone https://github.com/tmrh20/RF24Network.git RF24Network`
3. Goto RF24/utility/RPi/ and rename interrupt.c to interrupt.cpp
`mv RF24/utility/RPi/interrupt.c RF24/utility/RPi/interrupt.cpp`
4. Link includes for RPi in RF24
`ln -s RPi/includes.h RF24/utility/includes.h`
6. Install dependencies and build node file - remeber to make build on raspberry pi
`npm install`

>Important: In the make file of RF24, interrupt.c build uses CXX but I couldnt find a way of telling node-gyp to compile this file using CXX. If it's .c then it compile using CC and throw errors. To tell node-gyp to use CXX compiler, I have to rename the file extension of interrup.c

## Author
Project was forked from Sony Arouje (https://github.com/sonyarouje)

