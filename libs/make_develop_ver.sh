#!/bin/bash

TARGET_DIR=mylibs_qt5
SOURCE_DIR=src
NEED_CLEAN=0
DELAY=0.2



# check script arguments 
echo "look extra argumets:"
sleep 2
if [ $# -eq 0 ]; then
    echo "   args list is empty!"
else
    echo "   args number: $#"
    for arg_value in "$@"; do
        if [ $arg_value == "-c" ]; then
            NEED_CLEAN=1
#        elif [ $arg_value == "-s" ]; then
#            ARG_PUT_SCRIPTS=1
        else
            echo "WARNING: $arg_value is wrong argument!"
	    exit -1
        fi
    done
fi
sleep $DELAY



#start
sleep $DELAY
echo "check target dir [$TARGET_DIR]........"
sleep $DELAY
if [ -d $TARGET_DIR ]; then
    echo "  allready exist, need remove"
    sleep $DELAY
    rm -r -f $TARGET_DIR    
fi
sleep $DELAY
echo "try create target dir [$TARGET_DIR]........"
sleep $DELAY
mkdir $TARGET_DIR
echo -e "done! \n"


echo "copy souce dir [$SOURCE_DIR]........"
sleep $DELAY
cp -r $SOURCE_DIR $TARGET_DIR
echo -e "done! \n"


add_lib()
{
    sleep $DELAY
    echo -e " \n ----------Add lib: $1 ------------------"
    sleep $DELAY
    echo "copy lib folder"
    sleep $DELAY
    cp -r $1 $TARGET_DIR
    echo -e "done! \n"
    sleep $DELAY
    cd $TARGET_DIR/$1/

    if [ $NEED_CLEAN -eq 1 ]; then
        echo "need full clear ....."
        sleep $DELAY
	echo -e "\n [make clean]"
	sleep 1
	make clean
        sleep 1

	echo -e "\n [make distclean]"
	sleep 1
	make distclean
	sleep 1
        echo "done!"
    fi

    echo "remove generic files from $1 ..."	
    sleep $DELAY
    rm -r -f moc/ obj/
    rm full_clear *.user Makefile .gitignore .qmake.stash
    echo -e "done! \n"
    cd ../../            
}

add_lib "base"
#rebuild_lib "web"
add_lib "io_bus"
add_lib "process"
add_lib "mq"
add_lib "tcp"
#rebuild_lib "tg"
add_lib "xmlpack"
add_lib "socat"
sleep 1

#exit 1;

#make tar file
echo ""
echo "make tar distrib file ....."
sleep 0.5
tar -czvf $TARGET_DIR.tar.gz $TARGET_DIR
echo -e "done! \n"
sleep $DELAY

echo "remove temporary folder[$TARGET_DIR] ....."
sleep $DELAY
rm -r -f $TARGET_DIR
sleep $DELAY

echo "/////////// SCRIPT FINISHED///////////////"

exit 1
