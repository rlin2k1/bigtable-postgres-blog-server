#!/bin/bash
# Integration Test should be run in DIR: mrjk-web-server/tests

## -------------------------------------------------------------------------- ##
## Variable declarations
## -------------------------------------------------------------------------- ##
SRC_FOLDER=bin
BINARY_NAME=webserver
CONFIG_NAME=integration_test_config
IP_ADDRESS=127.0.0.1
PORT=8080
file1="output.txt"
file2="GETRequest.txt"

# ---------------------------------------------------------------------------- #
# Start the webserver with specified config
# ---------------------------------------------------------------------------- #
../build/$SRC_FOLDER/$BINARY_NAME $CONFIG_NAME &
WEBSERVER_PID=$!

# ---------------------------------------------------------------------------- #
# Run the Test
# ---------------------------------------------------------------------------- #
printf "GET / HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT > $file1
kill -9 $WEBSERVER_PID

if cmp -s "$file1" "$file2"; then
    echo "SUCCESS"
    rm $file1
    exit 0 # Exit Success
else
    echo "FAILURE"
    rm $file1
    exit 1 # Exit Failure
fi
