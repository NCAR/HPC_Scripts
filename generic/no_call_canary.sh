#!/bin/bash

echo "SSG No Call Canary: $0 $@" | mail -s "$(hostname -s): NO Call Canary" ssgmon@ucar.edu
echo "SSG No Call Canary: $0 $@"
echo "SSG No Call Canary: $0 $@" | /usr/bin/logger -t SSG

