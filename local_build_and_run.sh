#!/bin/sh

# You should run this outside of your development environment container and run it on your local machine instead

docker build -f docker/base.Dockerfile -t mrjk-web-server:base .

docker build -f docker/Dockerfile -t mrjk-web-server:base .

docker run --rm -p 80:80 --name local_test_run mrjk-web-server:base

# Now you can navigate to any valid path on your machine on a browser by going to localhost:80/your/path/here

# Note: run ctrl+c and the following command to double check that the container has stopped:
#   docker container stop local_test_run
# If it says error: no such container, that's fine, it just means that the container stopped already
