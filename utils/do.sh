#!/bin/bash

counter=1

while [ $counter -lt 50 ]
do
   #echo $counter

   python /home/rev/git/tditracer/utils/tdicustom combined123rmcycle.tdi 'VK_O,DOWN' 'VK_O,DOWN' $counter

   counter=`expr $counter + 1`
done
