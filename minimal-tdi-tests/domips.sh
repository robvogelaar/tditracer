mips-TiVo-linux-gnu-g++ -shared -fPIC -pthread -O2 -Wall -Wextra -Wno-unused-parameter libtdi.cpp -o libtdi.so -ldl -lrt && cp -v libtdi.so /home/rev/nfs
mips-TiVo-linux-gnu-g++ -pthread -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function tdi.cpp -ldl -lrt -o tdi && cp -v tdi /home/rev/nfs 

mips-TiVo-linux-gnu-g++ sleeper.cpp -pthread -O0 -Wall -Wno-unused-variable -o sleeper && cp -v sleeper /home/rev/nfs
mips-TiVo-linux-gnu-g++ simserver.cpp -Wall -Wno-unused-variable -ldl -o simserver && cp -v simserver /home/rev/nfs
mips-TiVo-linux-gnu-g++ simclient.cpp -Wall -Wno-unused-variable -o simclient && cp -v simclient /home/rev/nfs

mips-TiVo-linux-gnu-g++ membench.cpp -Wall -Wno-unused-variable -o membench && cp -v membench /home/rev/nfs
mips-TiVo-linux-gnu-gcc smemcap.c -Wall -o smemcap && cp -v smemcap /home/rev/nfs
mips-TiVo-linux-gnu-gcc fincore.c -Wall -lm -o fincore && cp -v fincore /home/rev/nfs
#mips-TiVo-linux-gnu-g++ memspeed.cpp -Wall -o memspeed && cp -v memspeed /home/rev/nfs
#mips-TiVo-linux-gnu-gcc memlock.c -Wall -o memlock && cp -v memlock /home/rev/nfs

mips-TiVo-linux-gnu-g++ -O2 procsmaps.cpp -Wall -o procsmaps && cp -v procsmaps /home/rev/nfs
#mips-TiVo-linux-gnu-g++ -O2 procdiskstats.cpp -Wall -o procdiskstats && cp -v procdiskstats /home/rev/nfs
#mips-TiVo-linux-gnu-g++ -O2 procnetdev.cpp -Wall -o procnetdev && cp -v procnetdev /home/rev/nfs
#mips-TiVo-linux-gnu-g++ -O2 procselfstat.cpp -Wall -o procselfstat && cp -v procselfstat /home/rev/nfs
#mips-TiVo-linux-gnu-g++ -O2 procstat.cpp -Wall -o procstat && cp -v procstat /home/rev/nfs


#mips-TiVo-linux-gnu-gcc -shared -O0 libcodemips.c -o libcodemips.so

#cp -v libcode.so /home/rev/nfs/
#cp -v libcode.so /home/rev/nfs/libcode1.so
#cp -v libcode.so /home/rev/nfs/libcode2.so
#cp -v libcode.so /home/rev/nfs/libcode3.so
#cp -v libcode.so /home/rev/nfs/libcode4.so
#cp -v libcode.so /home/rev/nfs/libcode5.so
#cp -v libcode.so /home/rev/nfs/libcode6.so
