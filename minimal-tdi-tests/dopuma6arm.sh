
#
#arm-rdk-linux-gnueabi-gcc
#-march=armv7ve
#-mthumb
#-mfpu=neon-vfpv4
#-mfloat-abi=hard
#-mcpu=cortex-a15
#--sysroot=/home/rev/yocto/master-onemw-sprint13/build-dcx960-debug/tmp/sysroots/dcx960-debug
#-O2
#-pipe
#-g
#-feliminate-unused-debug-types
#-fdebug-prefix-map=/home/rev/yocto/master-onemw-sprint13/build-dcx960-debug/tmp/work/cortexa15t2hf-neon-vfpv4-rdk-linux-gnueabi/tditracer/1.0-r0=/usr/src/debug/tditracer/1.0-r0 -fdebug-prefix-map=/home/rev/yocto/master-onemw-sprint13/build-dcx960-debug/tmp/sysroots/x86_64-linux= -fdebug-prefix-map=/home/rev/yocto/master-onemw-sprint13/build-dcx960-debug/tmp/sysroots/dcx960-debug=  -fstack-protector-strong
#-D_FORTIFY_SOURCE=2
#-fno-delete-null-pointer-checks
#-Wl,-O1 -Wl,--hash-style=gnu
#-Wl,--as-needed -fstack-protector-strong
#-Wl,-z,relro,-z,now -Wl,-rpath,/lib,-rpath,/usr/lib
#
#/home/rev/yocto/master-onemw-sprint13/build-dcx960-debug/tmp/work/cortexa15t2hf-neon-vfpv4-rdk-linux-gnueabi/tditracer/1.0-r0/git/minimal-tdi-tests/fincore.c -Wall -lm -o fincore
#
#

export SR="--sysroot=/opt/puma6-gw-6.1-toolchain/usr/armeb-buildroot-linux-uclibcgnueabi/sysroot"

#arm-rdk-linux-gnueabi-g++ $SR -fPIC -shared -pthread -O3 -Wall -Wextra -Wno-unused-result -Wno-unused-parameter libtdim.cpp -ldl -o libtdim.so && cp -v libtdim.so /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ $SR -pthread -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-aggressive-loop-optimizations tdim.cpp -ldl -lstdc++ -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 -o tdim && cp -v tdim /home/rev/nfs

armeb-linux-gcc $SR -fPIC -shared -pthread -O3 -Wall -Wextra -Wno-unused-result -Wno-unused-parameter libtdim.cpp -ldl -o libtdim.so
armeb-linux-gcc $SR -pthread -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-aggressive-loop-optimizations tdim.cpp -ldl -lstdc++ -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 -o tdim


#arm-rdk-linux-gnueabi-g++ $SR -pthread sleeper.cpp -O0 -Wall -Wno-unused-variable -o sleeper && cp -v sleeper /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ $SR simserver.cpp -Wall -Wno-unused-variable -ldl -o simserver && cp -v simserver /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ $SR simclient.cpp -Wall -Wno-unused-variable -o simclient && cp -v simclient /home/rev/nfs

#arm-rdk-linux-gnueabi-g++ $SR membench.cpp -Wall -Wno-unused-variable -o membench && cp -v membench /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ memspeed.cpp -Wall -o memspeed && cp memspeed /home/rev/nfs

#arm-rdk-linux-gnueabi-gcc smemcap.c -Wall -o smemcap && cp -v smemcap /home/rev/nfs
armeb-linux-gcc smemcap.c -Wall -o smemcap

#arm-rdk-linux-gnueabi-gcc fincore.c -Wall -lm -o fincore && cp -v fincore /home/rev/nfs
armeb-linux-gcc fincore.c -Wall -lm -o fincore

#arm-rdk-linux-gnueabi-gcc memlock.c -Wall -o memlock && cp memlock /home/rev/nfs

#arm-rdk-linux-gnueabi-g++ -O2 procdiskstats.cpp -Wall -o procdiskstats && cp -v procdiskstats /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ -O2 procnetdev.cpp -Wall -o procnetdev && cp -v procnetdev /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ -O2 procselfstat.cpp -Wall -o procselfstat && cp -v procselfstat /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ $SR -O2 procsmaps.cpp -Wall -o procsmaps && cp -v procsmaps /home/rev/nfs
#arm-rdk-linux-gnueabi-g++ -O2 procstat.cpp -Wall -Wno-unused-result -o procstat && cp -v procstat /home/rev/nfs

#gcc -m32 -shared -O0 libcode.c -o libcode.so
#cp -v libcode.so /home/rev/nfs/
#cp -v libcode.so /home/rev/nfs/libcode1.so
#cp -v libcode.so /home/rev/nfs/libcode2.so
#cp -v libcode.so /home/rev/nfs/libcode3.so
#cp -v libcode.so /home/rev/nfs/libcode4.so
#cp -v libcode.so /home/rev/nfs/libcode5.so
#cp -v libcode.so /home/rev/nfs/libcode6.so
