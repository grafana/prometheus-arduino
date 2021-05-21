FROM debian:latest

ARG version=0.4.5

WORKDIR /opt/nanopb
# Download first to aid debugging problems with downloading
RUN apt-get update && apt-get install -y curl
RUN curl -O -L https://github.com/nanopb/nanopb/archive/refs/tags/${version}.tar.gz && ls -al && tar -xzf ${version}.tar.gz && mv nanopb-${version}/generator .
RUN apt-get install -y python3 python3-pip curl
RUN pip3 install protobuf grpcio-tools

WORKDIR /opt/nanopb/generator

ENTRYPOINT ["./protoc"]