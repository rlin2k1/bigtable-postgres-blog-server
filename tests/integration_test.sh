#!/bin/bash
## -------------------------------------------------------------------------- ##
## Variable declarations
## -------------------------------------------------------------------------- ##
SRC_DIR=bin
TEST_DIR=../tests
BINARY_NAME=webserver
CONFIG_NAME=integration_test_config
IP_ADDRESS=localhost
PORT=8080

output_file="output.txt"
get_request_file="GETRequest.txt"
post_request_file="POSTRequest.txt"
long_body_request_file="LongBodyRequest.txt"
keep_alive_request_file="KeepAliveRequest.txt"
bad_request_file="BadRequest.txt"

# ---------------------------------------------------------------------------- #
# Start the webserver with specified config
# ---------------------------------------------------------------------------- #
$SRC_DIR/$BINARY_NAME $TEST_DIR/$CONFIG_NAME &
WEBSERVER_PID=$!

sleep 0.5 # Wait for Server to Start Up

# ---------------------------------------------------------------------------- #
# Run the Tests
# ---------------------------------------------------------------------------- #
printf "GET / HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$get_request_file

if [ $? != 0 ]
then
    echo "FAILED: GETRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "POST /index.html HTTP/1.1\r\nHost: 34.83.52.12\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_0) AppleWebKit/537.36 \
(KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36\r\nAccept: \
text/html,application/xhtml+xml,application/xml;q=0.9,\
image/webp,image/apng,*/*;q=0.8,application/signed-exchange;\
v=b3;q=0.9\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: \
en-US,en;q=0.9\r\n\r\n" | nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$post_request_file

if [ $? != 0 ]
then
    echo "FAILED: POSTRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "POST /test/test.php HTTP/1.0\r\nUser-Agent: Firefox\r\n\
Content-Length: 1394\r\nHost: 127.1.1.1\r\n\r\nonce upon a time\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
...............................................................\
................................................the end" |\
nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$long_body_request_file

if [ $? != 0 ]
then
    echo "FAILED: LongBodyRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET / HTTP/1.1\r\nConnection: Keep-Alive\r\nUser-Agent: nc/0.0.1\r\n\
Host: 127.0.0.1\r\nAccept: */*\r\n\r\n" |\
timeout .3 nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$keep_alive_request_file

if [ $? != 0 ]
then
    echo "FAILED: KeepAliveRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "POST /test/test.php HTTP/1.0\r\nUser-Agent: Firefox\r\n\
Content-Length: 30\r\nHost: 127.1.1.1\r\n\r\nonce upon a time\
.....................................................the end" |\
nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$bad_request_file

if [ $? != 0 ]
then
    echo "FAILED: IncorrectContentLength"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET / HTTP/1.1\r\nUser-Agent: nc/0.0.1\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$bad_request_file

if [ $? != 0 ]
then
    echo "FAILED: BadFormat"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
# ---------------------------------------------------------------------------- #
# Stop the WebServer and Exit
# ---------------------------------------------------------------------------- #
echo "SUCCESS: PASSED ALL TESTS"
kill $WEBSERVER_PID
exit 0
