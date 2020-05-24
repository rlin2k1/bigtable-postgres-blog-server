: '
integration_test.sh
This module contains the end-to-end integration tests for the webserver exec.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 18th, 2020
'

#!/bin/bash
## -------------------------------------------------------------------------- ##
## Variable declarations
## -------------------------------------------------------------------------- ##
SRC_DIR=./bin
TEST_DIR=../tests
STATIC_DIR=../files
BINARY_NAME=webserver
CONFIG_NAME=integration_test_config
IP_ADDRESS=localhost
PORT=8080

output_file="output.txt"
output_jpg_file="output.jpg"
output_png_file="output.png"
output_zip_file="output.zip"
output_html_file="output.html"
output_gif_file="output.gif"
output_pdf_file="output.pdf"
get_request_file="GETRequest.txt"
post_request_file="POSTRequest.txt"
long_body_request_file="LongBodyRequest.txt"
keep_alive_request_file="KeepAliveRequest.txt"
bad_request_file="BadRequest.txt"
not_found_request_file="NotFoundRequest.txt"
masked_echo_request_file="MaskedEchoGetRequest.txt"
status_request_file="StatusRequest.txt"
sorted_response_file="SortedResponse.txt"
echo_proxy_file="EchoProxy.txt"
echo_proxy_redirect_file="EchoProxyRedirect.txt"

log_file="sample_0.log"
nondeterministic_log_file="output.log"
request_log="Request.log"

proxy_config1="proxy_config1"
proxy_config2="proxy_config2"

printf "port 8080; # The port my server listens on

location \"/echo\" EchoHandler {
}

location \"/echo2\" EchoHandler {
}

location \"/static/masked\" EchoHandler {
}

location \"/static\" StaticHandler {
  root \"../files\";  # supports relative path
}

location \"/static2\" StaticHandler {
  root \"../files\";  # second path points to the same directory
}

location \"/sta tic\" StaticHandler {
  root \"../files\";  # path with a space is also supported
}

location \"/status\" StatusHandler {
}
" > $CONFIG_NAME;

# ---------------------------------------------------------------------------- #
# Start the webserver with specified config
# ---------------------------------------------------------------------------- #
chmod +x $SRC_DIR/$BINARY_NAME
$SRC_DIR/$BINARY_NAME $CONFIG_NAME &
WEBSERVER_PID=$!

sleep 0.5 # Wait for Server to Start Up

# ---------------------------------------------------------------------------- #
# Run the Tests
# ---------------------------------------------------------------------------- #
printf "GET /echo HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | sort > $output_file

sort $TEST_DIR/$get_request_file > $sorted_response_file
diff $output_file $sorted_response_file

if [ $? != 0 ]
then
    echo "FAILED: GETRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $sorted_response_file
rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/masked HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | sort > $output_file

sort $TEST_DIR/$masked_echo_request_file > $sorted_response_file
diff $output_file $sorted_response_file

if [ $? != 0 ]
then
    echo "FAILED: GETEchoMaskedRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $sorted_response_file
rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/helloworld.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 12 > $output_file

diff $output_file $STATIC_DIR/helloworld.txt

if [ $? != 0 ]
then
    echo "FAILED: GETRequestTXTFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /sta%%20tic/helloworld.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 12 > $output_file

diff $output_file $STATIC_DIR/helloworld.txt

if [ $? != 0 ]
then
    echo "FAILED: GETRequestSpaceRootPathTXTFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
# Note that we need to put %%20 here becuase printf normally uses % to indicate format characters
printf "GET /static/subdirectory/hello%%20world.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 12 > $output_file

diff $output_file $STATIC_DIR/subdirectory/hello\ world.txt

if [ $? != 0 ]
then
    echo "FAILED: GETRequestSpaceNameTXTFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/subdirectory/two%%20%%20spaces.jpg HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 23544 > $output_jpg_file

diff $output_jpg_file $STATIC_DIR/subdirectory/two\ \ spaces.jpg

if [ $? != 0 ]
then
    echo "FAILED: GETRequestTwoSpaceNameJPGFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_jpg_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/nothanks.jpg HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 23544 > $output_jpg_file

diff $output_jpg_file $STATIC_DIR/nothanks.jpg

if [ $? != 0 ]
then
    echo "FAILED: GETRequestJPGFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_jpg_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/subdirectory/helloworld.png HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 18664 > $output_png_file

diff $output_png_file $STATIC_DIR/subdirectory/helloworld.png

if [ $? != 0 ]
then
    echo "FAILED: GETRequestPNGFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_png_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/hack.gif HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 3516529 > $output_gif_file

diff $output_gif_file $STATIC_DIR/hack.gif
if [ $? != 0 ]
then
    echo "FAILED: GETRequestGIFFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_gif_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/kek.html HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 255791 > $output_html_file

diff $output_html_file $STATIC_DIR/kek.html

if [ $? != 0 ]
then
    echo "FAILED: GETRequestHTMLFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_html_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/zippitydooda.zip HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 42266 > $output_zip_file

diff $output_zip_file $STATIC_DIR/zippitydooda.zip

if [ $? != 0 ]
then
    echo "FAILED: GETRequestZIPFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_zip_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/hulkhogan.pdf HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 77885 > $output_pdf_file

diff $output_pdf_file $STATIC_DIR/hulkhogan.pdf

if [ $? != 0 ]
then
    echo "FAILED: GETRequestPDFFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_pdf_file
#---------------------------------------------------------------------------------------------------
printf "GET /static/nonexistent.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$not_found_request_file

if [ $? != 0 ]
then
    echo "FAILED: GETNonexistentFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /static2939/nonexistentpath.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$not_found_request_file

if [ $? != 0 ]
then
    echo "FAILED: GETNonexistentPathFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "POST /echo2 HTTP/1.1\r\nHost: 34.83.52.12\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_0) AppleWebKit/537.36 \
(KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36\r\nAccept: \
text/html,application/xhtml+xml,application/xml;q=0.9,\
image/webp,image/apng,*/*;q=0.8,application/signed-exchange;\
v=b3;q=0.9\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: \
en-US,en;q=0.9\r\n\r\n" | nc $IP_ADDRESS $PORT | sort > $output_file

sort $TEST_DIR/$post_request_file > $sorted_response_file
diff $output_file $sorted_response_file

if [ $? != 0 ]
then
    echo "FAILED: POSTRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $sorted_response_file
rm $output_file
#---------------------------------------------------------------------------------------------------
printf "POST /echo HTTP/1.0\r\nUser-Agent: Firefox\r\n\
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
nc $IP_ADDRESS $PORT | sort > $output_file

sort $TEST_DIR/$long_body_request_file > $sorted_response_file
diff $output_file $sorted_response_file

if [ $? != 0 ]
then
    echo "FAILED: LongBodyRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $sorted_response_file
rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /echo2 HTTP/1.1\r\nConnection: Keep-Alive\r\nUser-Agent: nc/0.0.1\r\n\
Host: 127.0.0.1\r\nAccept: */*\r\n\r\n" |\
timeout .3 nc $IP_ADDRESS $PORT | sort > $output_file

sort $TEST_DIR/$keep_alive_request_file > $sorted_response_file
diff $output_file $sorted_response_file

if [ $? != 0 ]
then
    echo "FAILED: KeepAliveRequest"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $sorted_response_file
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
#---------------------------------------------------------------------------------------------------
printf "GET /status HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT > $output_file

diff $output_file $TEST_DIR/$status_request_file

if [ $? != 0 ]
then
    echo "FAILED: GETStatus"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

# ---------------------------------------------------------------------------- #
# Stop the WebServer
# ---------------------------------------------------------------------------- #
kill $WEBSERVER_PID
rm $CONFIG_NAME

# ---------------------------------------------------------------------------- #
# Start 2 servers and do proxy tests
# ---------------------------------------------------------------------------- #
$SRC_DIR/$BINARY_NAME $TEST_DIR"/proxy_config1" &
WEBSERVER1_PID=$!
$SRC_DIR/$BINARY_NAME $TEST_DIR"/proxy_config2" &
WEBSERVER2_PID=$!

curl http://localhost:8080/proxy > "$output_file"
diff $output_file $TEST_DIR/$echo_proxy_file

if [ $? != 0 ]
then
    echo "FAILED: EchoProxy"
    kill $WEBSERVER1_PID
    kill $WEBSERVER2_PID
    exit 1 # Exit Failure
fi

curl http://localhost:8081/proxy > "$output_file"
diff $output_file $TEST_DIR/$echo_proxy_redirect_file

if [ $? != 0 ]
then
    echo "FAILED: EchoRedirectProxy"
    kill $WEBSERVER1_PID
    kill $WEBSERVER2_PID
    exit 1 # Exit Failure
fi

# ---------------------------------------------------------------------------- #
# Kill Both webservers
# ---------------------------------------------------------------------------- #
kill $WEBSERVER1_PID
kill $WEBSERVER2_PID

# ---------------------------------------------------------------------------- #
# Tests passed and Exit
# ---------------------------------------------------------------------------- #
echo "SUCCESS: PASSED ALL TESTS"
exit 0
