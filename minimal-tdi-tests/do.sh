

g++ -shared -pthread -O3 -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -fpermissive ../minimal-tdi/libtdim.cpp -fpic -ldl -o libtdim.so
g++ -pthread -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-aggressive-loop-optimizations ../minimal-tdi/tdim.cpp -ldl -lstdc++ -fPIC -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 -o tdim

g++ -pthread sleeper.cpp -O0 -Wall -Wno-unused-variable -static -o sleeper

exit

g++ -m32 -shared -pthread -O3 -Wall -Wextra -Wno-unused-result -Wno-unused-parameter ../minimal-tdi/libtdim.cpp -ldl -o libtdim.so && cp -v libtdim.so /home/rev/nfs
g++ -m32 -pthread -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-aggressive-loop-optimizations ../minimal-tdi/tdim.cpp -ldl -lstdc++ -fPIC -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 -o tdim && cp -v tdim /home/rev/nfs

g++ -m32 -pthread sleeper.cpp -O0 -Wall -Wno-unused-variable -static -o sleeper && cp -v sleeper /home/rev/nfs
g++ -m32 simserver.cpp -Wall -Wno-unused-variable -ldl -o simserver && cp -v simserver /home/rev/nfs
g++ -m32 simclient.cpp -Wall -Wno-unused-variable -o simclient && cp -v simclient /home/rev/nfs

g++ -m32 membench.cpp -Wall -Wno-unused-variable -o membench && cp -v membench /home/rev/nfs
#g++ -m32 memspeed.cpp -Wall -o memspeed && cp memspeed /home/rev/nfs

#gcc -m32 smemcap.c -Wall -o smemcap && cp -v smemcap /home/rev/nfs
#gcc -m32 fincore.c -Wall -lm -o fincore && cp -v fincore /home/rev/nfs
#gcc -m32 memlock.c -Wall -o memlock && cp memlock /home/rev/nfs

#g++ -m32 -O2 procdiskstats.cpp -Wall -o procdiskstats && cp -v procdiskstats /home/rev/nfs
#g++ -m32 -O2 procnetdev.cpp -Wall -o procnetdev && cp -v procnetdev /home/rev/nfs
#g++ -m32 -O2 procselfstat.cpp -Wall -o procselfstat && cp -v procselfstat /home/rev/nfs
g++ -m32 -O2 procsmaps.cpp -Wall -o procsmaps && cp -v procsmaps /home/rev/nfs
#g++ -m32 -O2 procstat.cpp -Wall -Wno-unused-result -o procstat && cp -v procstat /home/rev/nfs


#gcc -m32 -shared -O0 libcode.c -o libcode.so
#cp -v libcode.so /home/rev/nfs/
#cp -v libcode.so /home/rev/nfs/libcode1.so
#cp -v libcode.so /home/rev/nfs/libcode2.so
#cp -v libcode.so /home/rev/nfs/libcode3.so
#cp -v libcode.so /home/rev/nfs/libcode4.so
#cp -v libcode.so /home/rev/nfs/libcode5.so
#cp -v libcode.so /home/rev/nfs/libcode6.so
