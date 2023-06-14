#!/bin/bash


rebuild_lib()
{
    echo "Try rebuild lib: $1"
    sleep 1
    cd $1
    echo "clearing ...."
    sleep 0.5
    ./full_clear
    echo -e "done! \n"
    sleep 0.5
    echo "building ...."
    sleep 0.5
    qmake
    sleep 0.5
    make
    sleep 1
    echo  -e "//////////////////////FINISHED/////////////////////////////// \n"
    cd ..
}

rebuild_lib "base"
rebuild_lib "web"
rebuild_lib "fx"
rebuild_lib "io_bus"
rebuild_lib "process"
rebuild_lib "mq"
rebuild_lib "tcp"
rebuild_lib "tg"
rebuild_lib "xmlpack"
rebuild_lib "socat"

exit 1
