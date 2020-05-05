#!/bin/bash
## -------------------------------------------------------------------------- ##
## Variable declarations
## -------------------------------------------------------------------------- ##
SRC_DIR=./bin
TEST_DIR=../tests
STATIC_DIR_1=../server_static_1
STATIC_DIR_2=../server_static_2
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

log_file="sample_0.log"
nondeterministic_log_file="output.log"
request_log="Request.log"

# ---------------------------------------------------------------------------- #
# Create Dynamic Config File
# ---------------------------------------------------------------------------- #
# Checks if root paths exist for dev environment. If not, use production
root_path="/usr/src/projects/mrjk-web-server"
if [ ! -d $root_path ]
then
    root_path="/usr/src/project"
    echo "Using Production Environment Path."
else
    echo "Using Development Environment Path."
fi

printf "
            listen 8080;

            root $root_path;

            servlet static {
                server_location /server_static_1;
                client_location /client_static_1;
            }

            servlet static {
                server_location /server_static_2;
                client_location /client_static_2;
            }

            servlet echo {
                location /echo;
            }

            servlet echo {
                location /echo2;
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
printf "GET /client_static_1/helloworld.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 12 > $output_file

diff $output_file $STATIC_DIR_1/helloworld.txt

if [ $? != 0 ]
then
    echo "FAILED: GETRequestTXTFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_1/nothanks.jpg HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 23544 > $output_jpg_file

diff $output_jpg_file $STATIC_DIR_1/nothanks.jpg

if [ $? != 0 ]
then
    echo "FAILED: GETRequestJPGFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_jpg_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_1/subdirectory/helloworld.png HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 18664 > $output_png_file

diff $output_png_file $STATIC_DIR_1/subdirectory/helloworld.png

if [ $? != 0 ]
then
    echo "FAILED: GETRequestPNGFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_png_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_2/hack.gif HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 3516529 > $output_gif_file

diff $output_gif_file $STATIC_DIR_2/hack.gif
if [ $? != 0 ]
then
    echo "FAILED: GETRequestGIFFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_gif_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_1/kek.html HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 255791 > $output_html_file

diff $output_html_file $STATIC_DIR_1/kek.html

if [ $? != 0 ]
then
    echo "FAILED: GETRequestHTMLFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_html_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_1/zippitydooda.zip HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 42266 > $output_zip_file

diff $output_zip_file $STATIC_DIR_1/zippitydooda.zip

if [ $? != 0 ]
then
    echo "FAILED: GETRequestZIPFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_zip_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_2/hulkhogan.pdf HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
Accept: */*\r\n\r\n" | nc $IP_ADDRESS $PORT | tail -c 77885 > $output_pdf_file

diff $output_pdf_file $STATIC_DIR_2/hulkhogan.pdf

if [ $? != 0 ]
then
    echo "FAILED: GETRequestPDFFile"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $output_pdf_file
#---------------------------------------------------------------------------------------------------
printf "GET /client_static_1/nonexistent.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
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
printf "GET /client_static_2939/nonexistentpath.txt HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\
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
printf "GET /echo2 HTTP/1.1\r\nConnection: Keep-Alive\r\nUser-Agent: nc/0.0.1\r\n\
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
#---------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------- #
# Logging Integration Tests from ALL Above Tests
# ---------------------------------------------------------------------------- #
cat $log_file |  sed -r -e 's/^.*\|.*\| (.*\|.*)/\1/g' | \
sed -e '/info | Client at IP Address: .*/d' \
-e '/info | Root path: .*/d' -e '/info | ProcessID of server is: .*/d' \
> $nondeterministic_log_file

diff $nondeterministic_log_file $TEST_DIR/$request_log

if [ $? != 0 ]
then
    echo "FAILED: RequestLog"
    kill -9 $WEBSERVER_PID
    exit 1 # Exit Failure
fi

rm $nondeterministic_log_file
# ---------------------------------------------------------------------------- #
# Stop the WebServer and Exit
# ---------------------------------------------------------------------------- #
echo "SUCCESS: PASSED ALL TESTS"
kill $WEBSERVER_PID
rm $CONFIG_NAME
exit 0
