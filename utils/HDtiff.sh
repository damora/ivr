#!/bin/sh

raw2tiff -w 1920 -l 1080 -d float -b 4  -p rgb $1.raw $1.tiff
