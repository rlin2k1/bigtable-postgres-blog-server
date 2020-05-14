# MRJK Server 

## Code layout

In our main function where the program starts, we use the config parser to parse the config file that is passed as a command line argument when the program is run. From this config file, we extract the port number, the mappings of client locations paths to server base directory paths (for static handlers), and the list of client echo location paths (for echo handlers). The parsed information is stored in an NginxConfig object.

We also instantiate our request handler dispatcher in the main function and pass both the config object and a reference to the dispatcher to the server. During the server setup, the dispatcher takes the information from the config object, and registers a bunch of different handlers depending on the type. For each of the paths in our unordered_set of echo paths, we create an echo handler. Similarly, with the static handlers, for each client path -> server path mapping in our config object's unordered_map for static locations, we create a new static handler. The path that we pass to the init function for the static handlers is the client side path so that the handler knows which client path it should be looking for. It also takes the map that contains the path mappings so that it can map this client location to the actual server-side base directory to look for the files to give back.

When a client sends a request, we start a new session, passing a pointer to our request handler dispatcher object. This session asynchronously reads until the request that we received is determined to be either good or bad (if it's indeterminate, it will wait for more input). The request parser is an adapted version of the boost example request parser (link is available in ./src/request_parser.cc). The request parser gets the information from the request and puts it in session's request_builder member object (also adapted from boost). After the request parsing is done, if the client's request is good, the request_builder object is translated into a request based on our common API. (If the request is bad, we return a default bad request response.) We then use the handler dispatcher to determine which handler to use, and then use that given handler to return us a response object. We then take this response object and write it to the socket with a little help from our response_helper library.

The echo handler works by taking its request object parameter, taking each of the individual fields, and rebuilding from those pieces to populate a response object. This object is then returned back to the session, and the session writes to the socket. Note that because we are using an ordered map for our headers, the order of the headers will be the same, but not necessarily the same order that they were sent to us.

Our static handler works by receiving a request object from session, and then parsing the uri to find which file to serve back. The static handler first parses out the client location path from the uri (using the location parameter it was passed in the init function). Then it replaces that with the server side path from the map that it got from the config object (in the init function as well). After constructing this new path, it attempts to open the file on the server side, read the file into a vector of bytes (chars), and then populate a response object, and return that response object back to session.

The status handler returns two pieces of information: 1) a list of all existing handlers and their URL prefixes 2) a list of the number of request received and its respective response code. The list of all handlers is found during the initialization of the status handler, where it takes in a configuration object in its parameter. Status handler references this config object's echo and static locations to create the handler list. The list of all requests received by the webserver is stored with a setter function in the status handler (record_received_request). This setter function is called within ./src/session.cc after it has been determined that the parsing of the request was successful, and the corresponding request is handled. This setter function is only called if a flag is enabled that indicates the status handler is enabled. This flag is determined when we create the handler mapping within the request dispatcher using the configuration object. 

If the handler dispatcher cannot map the client's uri to any of our handlers, then it returns the 404 handler, which then returns a default not found response.

Our config file that we use for deployment is called "config" and it is located in the mrjk-web-server directory. 


## Building and Testing Code

To build the server, cd into the build directory and run:

```
cmake .. && make [&& make test]
```

This will produce an executable called "webserver" that you can run inside of the build/bin directory. You will also need to pass a config file as an argument, but you will need to be careful about how you specify the paths in the config file (primarily for static handler), because if you use relative paths, it will be relative to where the executable is. It might be best to just copy that webserver executable to the main mrjk-web-server directory and run it there with your config file that uses "./your/path/here" for the server side base directories in the config file. 

After you get the server started, you can open a new terminal window, and do something like:

```
printf "GET /your/request/here\r\n\r\n" | nc localhost your_portnum
```

And you should see the repsonse sent back by our server. The logging output should show up in the terminal window in which you started the server.

Running it this way might seem a little tedious, so we made a script to build and run our server inside a docker container, just like it does on google cloud. This way, you can see the logging of the server in the terminal as well as the actual files being served on the browser.

To run the script, go to the mrjk-web-server directory *outside* of your development environment (this is important so that you can see the server run on your machine's localhost 80). Then run 
```
./local_build_and_run.sh
```
This will run all the docker build instructions, as well as our tests. Once you begin to see logging output, our server is running. Then navigate to your browser at localhost:80/your/path/here and you should see the response returned by the server. As you make requests to the server, you will also see the log output in the terminal window where you started the server.


## Adding Handlers

To add handlers, the primary files that you will need to change are:

- ./src/request_dispatcher.cc
- ./include/request_dispatcher.h
    - The two above files define and implement our request handler dispatcher. You will need to change these files to register your new handler based on the client uri from the request. The .cc file has a template/instructions to help get you started.
- ./CMakeLists.txt
    - You will need to add your new files into this file as libraries so that the server actually picks them up when it's building.
- (probably) ./src/status_request_handler.cc
    - If you want the requests to show up in the status report, you will need to add support to this handler for it.
- (probably) ./tests/integration_test.sh
    - We found it better and easier to test the more complicated handlers using integration tests instead of unit tests.
- (maybe) ./config
    - This is our config file that we use for deployment onto gcloud, so when you finish your code, if it was necessary to add a path to the config file, then you will need to update this file with the final version of the config statement
- (maybe) ./src/NginxConfigParser.cc
    - We get most of the information we need from the config file in the parser, and then transfer that information over to the 
- (maybe) ./include/config_parser.h
- plus any files that you add for your handler

Note that the header (.h) files go in the ./include directory, the source files go in the ./src directory, and the test files go in the ./tests directory. 

## Group Contact Information

In the event where our webserver does not run as expected or you run into difficulties after thoroughly reading this file and documentation, feel free contact any of the team members below:

Kubilay Agi: <kubilayagi@g.ucla.edu>
Michael Gee: <mkgee@g.ucla.edu>
Jane Lee: <janejiwonlee@g.ucla.edu>
Roy Lin: <rlin2k1@g.ucla.edu>