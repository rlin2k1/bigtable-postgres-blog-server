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
FROM ubuntu:latest as deploy

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/server .
# Copy configuration file to "."
COPY --from=builder /usr/src/project/config .

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["config"]