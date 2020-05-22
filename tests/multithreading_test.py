""" multithreading_test.py
Tests multithreading capabilities of the webserver.

Specification proving that the server can handle simultaneous requests:
    N = NUM_THREADS
    1. Open N-1 connection to the web server.
    2. Send N-1 partial request, and leave the connection open.
    3. On the Nth request, Open a new connection to the server,
       and send a complete request.
       Get an immediate response back from the server.
       It shouldn’t block on the partial request.
    4. Finish the first N-1 requests.

How to run: In mrjk-web-server directory: python3 multithreading_test.py

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 21st, 2020
"""
from subprocess import Popen
from subprocess import PIPE
import socket
import sys
import time

NUM_THREADS = 4
SRC_DIR ="./bin"
BINARY_NAME ="webserver"
EXECUTABLE_PATH = SRC_DIR + "/" + BINARY_NAME
CONFIG_NAME ="config"
IP_ADDRESS = "localhost"
PORT = 8080

# ---------------------------------------------------------------------------- #
# Sentinel for a successful test
# ---------------------------------------------------------------------------- #
exit_code = 0

# ---------------------------------------------------------------------------- #
# Open the web server in a subprocess
# ---------------------------------------------------------------------------- #
webserver = Popen([EXECUTABLE_PATH, CONFIG_NAME], stdout=PIPE)

# Wait for web server to start up
time.sleep(1);

# ---------------------------------------------------------------------------- #
# Test the multithreading capabilities of the webserver
# ---------------------------------------------------------------------------- #
expected_multithreaded_output = (
    "HTTP/1.0 200 OK\r\n"
    "Content-Length: 22\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "GET /echo HTTP/1.0\r\n"
    "\r\n"
)

def diff_check(multithreaded_output):
    if not multithreaded_output == expected_multithreaded_output:
        exit_code = 1
        sys.stdout.write("\nFAILED to match the following server output:\n")
        sys.stdout.write(expected_multithreaded_output)
        sys.stdout.write("\nMULTITHREADED TEST FAILED\n")
        sys.exit(exit_code)

# We will open multiple socket connections to the web server
sockets = []

for i in range(NUM_THREADS):
    s = socket.socket()
    sockets.append(s)
    s.connect((IP_ADDRESS, PORT))

    # Pick the last thread to finish
    if i < (NUM_THREADS - 1):
        # NUM_THREADS - 1 of partial requests
        sockets[i].send(b'GET /echo HT')
    else:
        # A complete request
        sockets[i].send(b'GET /echo HTTP/1.0\r\n\r\n')
        # We do not want to wait forever for the response
        sockets[i].settimeout(1)
        multithreaded_output = sockets[i].recv(1024).decode(sys.stdout.encoding)

        sys.stdout.write("Server Response for Thread " + str(i) + ":\n")
        sys.stdout.write(multithreaded_output)
        sys.stdout.write("#---------------------------------------------------")
        diff_check(multithreaded_output)

# Finish off the rest of the requests
for i in range(NUM_THREADS - 1):
    sockets[i].send(b'TP/1.0\r\n\r\n')
    sockets[i].settimeout(1)
    multithreaded_output = sockets[i].recv(1024).decode(sys.stdout.encoding)

    sys.stdout.write("\nServer Response for Thread " + str(i) + ":\n")
    sys.stdout.write(multithreaded_output)
    sys.stdout.write("#---------------------------------------------------")
    diff_check(multithreaded_output)

for i in range(NUM_THREADS):
    sockets[i].close()

# ---------------------------------------------------------------------------- #
# Close the webserver
# ---------------------------------------------------------------------------- #
webserver.terminate()

sys.stdout.write("\nMULTITHREADED TEST SUCCEEDED\n")
sys.exit(exit_code)
