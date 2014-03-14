#!/bin/bash -e

ZIPFILE=submission.zip
SOURCES=(SimplePlayer3.cpp SimplePlayer2.cpp world.cpp client.cpp)
HEADERS=(SimplePlayer3.h SimplePlayer2.h Player.h world.h)
DIR=`pwd`

echo "Creating: $DIR/$ZIPFILE"
rm -f "$ZIPFILE"
zip "$ZIPFILE" "${HEADERS[@]}" "${SOURCES[@]}"

TMP=`mktemp -d`
if chmod 700 "$TMP" &&  cd "$TMP" && unzip "$DIR"/"$ZIPFILE" && \
	g++ -O2 -Wall -Werror -std=c++0x "${SOURCES[@]}" && test -x a.out
then
	echo "Submission compiled successfully."
	rm -r "$TMP"
	exit 0
else
	echo "FAILED TO COMPILE SUBMISSION!"
	rm -r "$TMP"
	exit 1
fi
