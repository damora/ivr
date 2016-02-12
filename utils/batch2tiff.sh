#!/bin/sh
for i in `ls *image*.raw`; 
do 
j=${i%\.raw}; 
echo "2tiff $j";
./2tiff.sh $j; 
done
