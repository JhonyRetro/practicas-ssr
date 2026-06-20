#!/usr/bin/env bash

declare -r NAME='ssr08'

docker run -d --rm --name "${NAME}" debian:stable sleep infinity
docker exec "${NAME}" bash -c 'apt update && apt install -y build-essential gdb'
docker cp sources "${NAME}":/root
docker exec -it --workdir=/root/sources "${NAME}" bash
docker kill "${NAME}"
