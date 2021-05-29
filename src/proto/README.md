
A docker file for helping build protos can be found [here](https://github.com/grafana/arduino-snappy-proto/blob/main/src/proto/Dockerfile)

Use these commands to mount the current dir into the image at /tmp and then generate the c files from the protos:

```
docker run --rm -v ${PWD}:/tmp slimbean/nanopb-gen --proto_path=/tmp --nanopb_out=/tmp /tmp/remote.proto
docker run --rm -v ${PWD}:/tmp slimbean/nanopb-gen --proto_path=/tmp --nanopb_out=/tmp /tmp/types.proto
```
