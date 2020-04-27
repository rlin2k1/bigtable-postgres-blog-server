### Build/test container ###
# Define builder stage
FROM mrjk-web-server:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:bionic as deploy

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/webserver .
# Copy configuration file to "."
COPY --from=builder /usr/src/project/config .
# Copy static files to "."
COPY server_static_1/ /usr/src/project/server_static_1/
COPY server_static_2/ /usr/src/project/server_static_2/

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["config"]