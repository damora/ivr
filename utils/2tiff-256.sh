#!/bin/sh

raw2tiff -w $1 -l $2 -d float -b 4  -p rgb $3.raw $3.tiff
