### Build/test container ###
# Define builder stage
FROM mrjk-web-server:base as builder

# Share work directory
COPY . /usr/src/project

# Build and run coverage report
WORKDIR /usr/src/project/build_coverage
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage