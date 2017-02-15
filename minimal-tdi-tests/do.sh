g++ -m32 -shared -pthread -Os -Wall -Wno-unused-result libtdi.cpp -ldl -o libtdi.so && cp -v libtdi.so /home/rev/nfs
g++ -m32 -pthread -O2 -Wall tdi.cpp -ldl -o tdi && cp -v tdi /home/rev/nfs

g++ -m32 sleeper.cpp -Wall -Wno-unused-variable -o sleeper && cp -v sleeper /home/rev/nfs
g++ -m32 simserver.cpp -Wall -Wno-unused-variable -ldl -o simserver && cp -v simserver /home/rev/nfs
g++ -m32 simclient.cpp -Wall -Wno-unused-variable -o simclient && cp -v simclient /home/rev/nfs

#g++ -m32 membench.cpp -Wall -Wno-unused-variable -o membench && cp -v membench /home/rev/nfs
#g++ -m32 memspeed.cpp -Wall -o memspeed && cp memspeed /home/rev/nfs

#gcc -m32 smemcap.c -Wall -o smemcap && cp -v smemcap /home/rev/nfs
#gcc -m32 fincore.c -Wall -lm -o fincore && cp -v fincore /home/rev/nfs
#gcc -m32 memlock.c -Wall -o memlock && cp memlock /home/rev/nfs

#g++ -m32 -O2 procdiskstats.cpp -Wall -o procdiskstats && cp -v procdiskstats /home/rev/nfs
#g++ -m32 -O2 procnetdev.cpp -Wall -o procnetdev && cp -v procnetdev /home/rev/nfs
#g++ -m32 -O2 procselfstat.cpp -Wall -o procselfstat && cp -v procselfstat /home/rev/nfs
#g++ -m32 -O2 procsmaps.cpp -Wall -o procsmaps && cp -v procsmaps /home/rev/nfs
#g++ -m32 -O2 procstat.cpp -Wall -Wno-unused-result -o procstat && cp -v procstat /home/rev/nfs


#gcc -m32 -shared -O0 libcode.c -o libcode.so
#cp -v libcode.so /home/rev/nfs/
#cp -v libcode.so /home/rev/nfs/libcode1.so
#cp -v libcode.so /home/rev/nfs/libcode2.so
#cp -v libcode.so /home/rev/nfs/libcode3.so
#cp -v libcode.so /home/rev/nfs/libcode4.so
#cp -v libcode.so /home/rev/nfs/libcode5.so
#cp -v libcode.so /home/rev/nfs/libcode6.so
