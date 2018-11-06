
. ./dopuma6arm.sh
. ./dopuma6atom.sh
. ./do.sh

scp *.atom* rev@192.168.2.110:/home/rev/tftpboot/
scp *.arm* rev@192.168.2.110:/home/rev/tftpboot/

scp tdim rev@192.168.2.110:/home/rev/tftpboot/
scp libtdim.so rev@192.168.2.110:/home/rev/tftpboot/

scp tdim rev@192.168.2.140:/home/rev/tftpboot/
scp libtdim.so rev@192.168.2.140:/home/rev/tftpboot/

