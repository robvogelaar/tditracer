
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


export extra="-g0"

armeb-linux-gcc $extra -fPIC -shared -pthread -O3 -Wall -Wextra -Wno-unused-result -Wno-unused-parameter ../minimal-tdi/libtdim.cpp -ldl -o libtdim.arm.so
armeb-linux-gcc $extra -pthread -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function ../minimal-tdi/tdim.cpp -ldl -lstdc++ -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 -o tdim.arm

armeb-linux-gcc $extra -pthread sleeper.cpp -O0 -Wall -o sleeper.arm
armeb-linux-gcc $extra smemcap.c -O2 -Wall -o smemcap.arm
armeb-linux-gcc $extra fincore.c -O2 -Wall -lm -o fincore.arm

armeb-linux-strip *.arm*

