#!/bin/sh
for i in `ls image*.raw`; 
do 
j=${i%\.raw}; 
echo "HDtiff $j";
./HDtiff.sh $j; 
done
