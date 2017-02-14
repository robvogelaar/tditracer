
mips-TiVo-linux-gnu-g++ -shared -pthread -O2 -Wall libtdi.cpp -o libtdi.so -ldl && cp libtdi.so /home/rev/nfs
mips-TiVo-linux-gnu-g++ -pthread -O2 -Wall tdi.cpp -ldl -lrt -o tdi && cp tdi /home/rev/nfs 


mips-TiVo-linux-gnu-g++ membench.cpp -Wall -Wno-unused-variable -o membench && cp membench /home/rev/nfs
#mips-TiVo-linux-gnu-g++ memspeed.cpp -Wall -o memspeed && cp memspeed /home/rev/nfs


mips-TiVo-linux-gnu-gcc smemcap.c -Wall -o smemcap && cp smemcap /home/rev/nfs
mips-TiVo-linux-gnu-gcc fincore.c -Wall -lm -o fincore && cp fincore /home/rev/nfs
#mips-TiVo-linux-gnu-gcc memlock.c -Wall -o memlock && cp memlock /home/rev/nfs

mips-TiVo-linux-gnu-g++ -O2 procdiskstats.cpp -Wall -o procdiskstats && cp procdiskstats /home/rev/nfs
mips-TiVo-linux-gnu-g++ -O2 procnetdev.cpp -Wall -o procnetdev && cp procnetdev /home/rev/nfs
mips-TiVo-linux-gnu-g++ -O2 procselfstat.cpp -Wall -o procselfstat && cp procselfstat /home/rev/nfs
mips-TiVo-linux-gnu-g++ -O2 procsmaps.cpp -Wall -o procsmaps && cp procsmaps /home/rev/nfs
mips-TiVo-linux-gnu-g++ -O2 procstat.cpp -Wall -o procstat && cp procstat /home/rev/nfs

mips-TiVo-linux-gnu-g++ sleeper.cpp -Wall -Wno-unused-variable -o sleeper && cp sleeper /home/rev/nfs

mips-TiVo-linux-gnu-g++ simserver.cpp -Wall -Wno-unused-variable -ldl -o simserver && cp simserver /home/rev/nfs
mips-TiVo-linux-gnu-g++ simclient.cpp -Wall -Wno-unused-variable -o simclient && cp simclient /home/rev/nfs

#mips-TiVo-linux-gnu-gcc -shared -O0 libcode.c -o libcodemips.so

#scp libcode.so root@$IP:/usr/lib/
#scp libcode.so root@$IP:/usr/lib/libcode1.so
#scp libcode.so root@$IP:/usr/lib/libcode2.so
#scp libcode.so root@$IP:/usr/lib/libcode3.so
#scp libcode.so root@$IP:/usr/lib/libcode4.so
#scp libcode.so root@$IP:/usr/lib/libcode5.so
#scp libcode.so root@$IP:/usr/lib/libcode6.so

