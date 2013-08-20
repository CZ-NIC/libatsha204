#!/bin/sh

LOG_FILE_PATH="programming.log"

if [ $# -ne 1 ]; then
	echo "ERROR: Bad argument count"
	echo "Usage: $0 config"
	exit 1
fi

if [ $(whoami) != "root" ]; then
	echo "ERROR: Script needs to be run as superuser"
	echo "Use: sudo $0"
	exit 1
fi

date +"Run from %Y-%m-%d %H:%M:%S" >> $LOG_FILE_PATH

echo "Device programmer..."
CHIPINIT_STDOUT=$(chipinit $1 2>> $LOG_FILE_PATH)
if [ $? -eq 0 ]; then
	echo $CHIPINIT_STDOUT
	echo "Programming OK" >> $LOG_FILE_PATH
else
	echo $CHIPINIT_STDOUT
	echo $CHIPINIT_STDOUT >> $LOG_FILE_PATH
	exit 1
fi

CHIPTOOLS_STDOUT=$(chiptools chipsn)
if [ $? -eq 0 ]; then
	echo $CHIPTOOLS_STDOUT >> $1
else
	echo "Get SN FAILED"
	echo "Get SN FAILED" >> $LOG_FILE_PATH
	exit 1
fi

echo "Device test..."
chiptest $1 > /dev/null 2>> $LOG_FILE_PATH
if [ $? -eq 0 ]; then
	echo "OK"
	echo "Test OK" >> $LOG_FILE_PATH
else
	echo "FAILED"
	echo "Test FAILED" >> $LOG_FILE_PATH
	exit 1
fi
