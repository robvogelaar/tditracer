#!/bin/sh

kill -9 $(pidof simserver)
kill -9 $(pidof sleeper)

proc()
{

LD_PRELOAD=libtdi.so TRACEBUFFERSIZE=1 REMOVE=0 SELFINFO=100 ./simserver /tmp/s$1 &
sleep 1

local fcounter=1
while [ $fcounter -le `expr $4` ]
do

echo "mmap $2"                              | ./simclient /tmp/s$1
echo "code libcode$1.so f$fcounter 100 $3"  | ./simclient /tmp/s$1

fcounter=`expr $fcounter + 1`
done
}

code()
{
echo "code libcode$1.so f$2 $3 $4"  | ./simclient /tmp/s$1
}

memset()
{
echo "memset $2" | ./simclient /tmp/s$1
}

mark()
{
echo "mark" | ./simclient /tmp/s1
}


###############################################################################

LD_PRELOAD=libtdi.so DISKS=sda2,sdb1 TRACEBUFFERSIZE=2 SYSINFO=100 ./sleeper & 
sleep 1

proc 1 8M 1024 4
proc 2 8M 1024 4
proc 3 8M 1024 4

mark

counter=1
while [ $counter -le `expr 10` ]
do

code 1 1 100 1024
code 1 2 100 1024
code 1 3 100 1024
code 1 4 100 1024

code 2 1 100 1024
code 2 2 100 1024
code 2 3 100 1024
code 2 4 100 1024

code 3 1 100 1024
code 3 2 100 1024
code 3 3 100 1024
code 3 4 100 1024

counter=`expr $counter + 1`
done

mark

proc 4 24M 2048 4

mark

counter=1
while [ $counter -le `expr 20` ]
do

code 1 1 100 1024
code 1 2 100 1024
code 1 3 100 1024
code 1 4 100 1024

code 2 1 100 1024
code 2 2 100 1024
code 2 3 100 1024
code 2 4 100 1024

code 3 1 100 1024
code 3 2 100 1024
code 3 3 100 1024
code 3 4 100 1024

code 4 1 100 1024
code 4 2 100 1024
code 4 3 100 1024
code 4 4 100 1024

counter=`expr $counter + 1`
done

mark

memset 1 1
memset 1 2
memset 1 3
memset 1 4

memset 2 1
memset 2 2
memset 2 3
memset 2 4

memset 3 1
memset 3 2
memset 3 3
memset 3 4

memset 4 1
memset 4 2
memset 4 3
memset 4 4

mark

counter=1
while [ $counter -le `expr 20` ]
do

code 1 1 100 1024
code 1 2 100 1024
code 1 3 100 1024
code 1 4 100 1024

code 2 1 100 1024
code 2 2 100 1024
code 2 3 100 1024
code 2 4 100 1024

code 3 1 100 1024
code 3 2 100 1024
code 3 3 100 1024
code 3 4 100 1024

code 4 1 100 1024
code 4 2 100 1024
code 4 3 100 1024
code 4 4 100 1024

counter=`expr $counter + 1`
done

mark
