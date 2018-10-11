#!/bin/bash

bsub -Is -q geyser -W 4:00 -n 1 -P SSSG0001 $SHELL

