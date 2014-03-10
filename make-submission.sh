#!/bin/bash -e

ZIPFILE=submission.zip
SOURCES=(SimplePlayer.cpp world.cpp client.cpp)
HEADERS=(Player.h SimplePlayer.h world.h)
DIR=`pwd`

echo "Creating: $DIR/$ZIPFILE"
rm -f "$ZIPFILE"
zip "$ZIPFILE" "${HEADERS[@]}" "${SOURCES[@]}" 

TMP=`mktemp -d`
if chmod 700 "$TMP" &&  cd "$TMP" && unzip "$DIR"/"$ZIPFILE" && \
	g++ -O2 -Wall -Werror "${SOURCES[@]}" && test -x a.out
then
	echo "Submission compiled successfully."
	rm -r "$TMP"
	exit 0
else
	echo "FAILED TO COMPILE SUBMISSION!"
	rm -r "$TMP"
	exit 1
fi
