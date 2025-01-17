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
# Copy dynamic library for libkeyutils
COPY --from=builder /lib/x86_64-linux-gnu/libkeyutils* /usr/lib/x86_64/
# Copy dynamic library for pq and pqxx library
COPY --from=builder /usr/lib/x86_64-linux-gnu/libpq* /usr/lib/x86_64/
# Copy dynamic library for pq and pqxx library
COPY --from=builder /usr/lib/x86_64-linux-gnu/libssl* /usr/lib/x86_64/
# Copy dynamic library for crypto library
COPY --from=builder /usr/lib/x86_64-linux-gnu/libcrypto* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libgssapi* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libldap_r-2.4* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libkrb5* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libk5crypto* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/liblber-2.4* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libsasl2* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libheimntlm* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libasn1* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libhcrypto* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libroken* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libwind* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libheimbase* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libhx509* /usr/lib/x86_64/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libsqlite3* /usr/lib/x86_64/

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["config"]
