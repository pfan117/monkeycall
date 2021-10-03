#!/usr/bin/env bash

if ! [ -e "./mkc" ] ; then
	echo "run 'make' first"
	exit 0
fi

for file in `find mt/correct/*`
do
	./mkc ${file} 1>/dev/null
	RESULT=$?
	if (( $RESULT )) ; then
		echo -e "\e[1;31m${file} failed\e[0m"
	fi
done

for file in `find mt/wrong/*`
do
	./mkc ${file} 2>/dev/null
	RESULT=$?
	if [ "0" -eq "$RESULT" ] ; then
		echo -e "\e[1;31m${file} failed\e[0m"
	elif [ "1" -ne "$RESULT" ] ; then
		echo -e "\e[1;31m${file} failed\e[0m"
	fi
done
