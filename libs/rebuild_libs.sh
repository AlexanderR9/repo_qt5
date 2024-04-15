#!/bin/bash


fclear()
{
    echo -e "\n [make clean]"
    sleep 1
    make clean    
    sleep 1

    echo -e "\n [make distclean]"
    sleep 1
    make distclean
    sleep 1


    echo -e "\n [remove tempory files]"
    sleep 1
    find . -type d -regex '.*\(moc\|obj\|pch\|rcc\|uic\|ui_h\|build\)' | sort | while read file; do
	echo "Remove: $file"
	rm -rf $file
    done

#    sleep 5
#    rm -rf ui_h
#    rm -rf build
}

rebuild_lib()
{
    echo "Try rebuild lib: $1"
    sleep 1
    cd $1


    echo "clearing ...."
    sleep 0.5
    fclear
    echo -e "done! \n"


    sleep 0.5
    echo "building ...."
    sleep 1
    qmake
    sleep 0.5
    make
    sleep 1
    echo  -e "//////////////////////FINISHED/////////////////////////////// \n"
    sleep 1
    cd ..
}

rebuild_lib "base"
rebuild_lib "web"
#exit 2

rebuild_lib "fx"
rebuild_lib "io_bus"
rebuild_lib "process"
rebuild_lib "mq"
rebuild_lib "tcp"
rebuild_lib "tg"
rebuild_lib "xmlpack"
rebuild_lib "socat"

exit 1
