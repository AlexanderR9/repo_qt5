#!/bin/bash

TARGET_DIR=mylibs_qt5
DELAY=0.2


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
    echo "remove generic files from $1 ..."	
    cd $TARGET_DIR/$1/
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
