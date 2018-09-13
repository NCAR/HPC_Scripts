#!/bin/bash
[ "$1" != "do it now" ] && echo 'this script is dangerous. please dont use it' && exit 1

docker rm -f $(docker ps -a -q)
docker rmi -f $(docker images -q)
