#!/bin/sh
for i in `ls tile*.raw`; 
do 
j=${i%\.raw}; 
echo "2tiff-256 $j";
./2tiff-256.sh 512 256 $j; 
done
