IP=192.168.2.106
#IP=10.7.241.20

g++ -m32 -shared -pthread -Os -Wall -Wno-unused-result libtdi.cpp -o libtdi.so -ldl && scp libtdi.so root@$IP:/usr/lib/
g++ -m32 -pthread -O2 tdi.cpp -Wall -ldl -o tdi && scp tdi root@$IP:/usr/bin

#g++ -m32 membench.cpp -Wall -Wno-unused-variable -o membench && scp membench root@$IP:/home/root/
#g++ -m32 memspeed.cpp -Wall -o memspeed && scp memspeed root@192.168.2.106:/home/root/

#gcc -m32 smemcap.c -Wall -o smemcap && scp smemcap root@192.168.2.106:/home/root/
#gcc -m32 fincore.c -Wall -lm -o fincore && scp fincore root@192.168.2.106:/home/root/
#gcc -m32 memlock.c -Wall -o memlock && scp memlock root@192.168.2.106:/home/root/

#g++ -m32 -O2 procdiskstats.cpp -Wall -o procdiskstats && scp procdiskstats root@$IP:/home/root/
#g++ -m32 -O2 procnetdev.cpp -Wall -o procnetdev && scp procnetdev root@$IP:/home/root/
#g++ -m32 -O2 procselfstat.cpp -Wall -o procselfstat && scp procselfstat root@$IP:/home/root/
#g++ -m32 -O2 procsmaps.cpp -Wall -o procsmaps && scp procsmaps root@$IP:/home/root/
#g++ -m32 -O2 procstat.cpp -Wall -Wno-unused-result -o procstat && scp procstat root@$IP:/home/root/

#g++ -m32 sleeper.cpp -Wall -Wno-unused-variable -o sleeper && scp sleeper root@$IP:/home/root/

g++ -m32 simserver.cpp -Wall -Wno-unused-variable -ldl -o simserver && scp simserver root@$IP:/home/root/
g++ -m32 simclient.cpp -Wall -Wno-unused-variable -o simclient && scp simclient root@$IP:/home/root/

#gcc -m32 -shared -O0 libcode.c -o libcode.so

#scp libcode.so root@$IP:/usr/lib/
#scp libcode.so root@$IP:/usr/lib/libcode1.so
#scp libcode.so root@$IP:/usr/lib/libcode2.so
#scp libcode.so root@$IP:/usr/lib/libcode3.so
#scp libcode.so root@$IP:/usr/lib/libcode4.so
#scp libcode.so root@$IP:/usr/lib/libcode5.so
#scp libcode.so root@$IP:/usr/lib/libcode6.so

