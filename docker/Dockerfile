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
COPY files/ files/

# Copy dynamic library for libxml2
COPY --from=builder /usr/lib/x86_64-linux-gnu/libxml2* /usr/lib/x86_64/
# Copy dynamic library for ICU library
COPY --from=builder /usr/lib/x86_64-linux-gnu/libicu* /usr/lib/x86_64/

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["config"]
