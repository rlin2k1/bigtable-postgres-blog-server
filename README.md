# MRJK Server 

A configurable web server that listens on a configurable port and responds to HTTP 1.1 requests by echoing the request back to the client. Our web server can also be configured to serve a static file. 

## Running the web server locally 

Run and start docker development using the command: 

```
tools/env/start.sh -u <user> -r 
```

Pull the latest commit on Gerrit for the project within the mrjk-web-server repository. 

or simply run 

```
git pull
``` 
within the local master branch. 

Then in one terminal head to your build directory and then the bin directory to find the server executable and link it up to a config file with a local port number e.g 8080: 
```
cd build 
cd bin 
./webserver <config file>
```

And in another terminal you can run a netcat command to make an HTTP request to the server: 
```
Example: printf "GET /echo HTTP/1.1\r\nHost: z\r\n\r\n" | nc 127.0.0.1 8081
```

which will return: 
```
HTTP/1.0 200 OK
Content-Length: 31
Content-Type: text/plain

GET /echo HTTP/1.1
Host: z
```

You can also check out our web server on our browser using our IP address http://34.83.88.81/ and check out our web server's functionality e.g static picture functionality with url http://34.83.88.81/client_static_1/nothanks.jpg 