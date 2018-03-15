#!/bin/sh
#clear

meminfo() {
    cat /proc/meminfo | grep ^$1 | awk '{printf "%-15s %8.1f %-5s\n", $1, $2/1024, "MB"}'

    if [ -n "$2" ]; then
        local  __resultvar=$2
        local  myresult=$(cat /proc/meminfo | grep ^$1 | awk '{printf "%d", $2}')
        eval $__resultvar="'$myresult'"
    fi
}

mount | grep -q "on /proc"
if [ $? -eq 1 ]; then
  echo "/proc not mounted"
  exit 1
fi


[[ "$1" == "drop" ]] && echo dropping cache ; sync; echo 3 > /proc/sys/vm/drop_caches ; sync

cat /proc/cpuinfo | grep 'model name.*QEMU' 1> /dev/null && is_qemu=1 || is_qemu=0
cat /proc/cpuinfo | grep 'model name.*ARM' 1> /dev/null && is_arm=1 || is_arm=0
cat /proc/cpuinfo | grep 'model name.*Atom' 1> /dev/null && is_atom=1 || is_atom=0

if [ $is_arm == 1 ]; then
echo '                        ARM'
echo '---------------------------'
echo 'Physical:          252.0 MB'
#echo 'Available:         252.0 MB (due to : 256M - 4M for DSP)'
#echo "Memory: 250120K/258048K available (3355K kernel code, 175K rwdata, 964K rodata, 150K init, 982K bss, 7928K reserved, 0K highmem)"
memtot=$(cat /proc/meminfo | grep MemTotal: | grep -o '[0-9]*')
echo $memtot | awk '{printf "Kernel Static:     %5.1f MB\n", 252 - ($1 / 1024)}'
echo $memtot | awk '{printf "Available:         %5.1f MB\n", ($1 / 1024) - 0.2}'
echo 'init:                0.2 MB'
fi

if [ $is_atom == 1 ]; then
echo '                       ATOM'
echo '---------------------------'
#echo 'Available:         243.1 MB (due to : 128M + 115M + 128K exactmap in kernel command line)'
cat /var/log/messages* | grep -o 'Memory:.*' | grep -o '/[0-9]*' | grep -o '[0-9]*' | awk '{printf "Physical:          %5.1f MB\n", $1 / 1024}'
cat /var/log/messages* | grep -o 'Memory:.*' | grep -o '[0-9]*K reserved' | grep -o '[0-9]*' | awk '{printf "Kernel Static:     %5.1f MB\n", $1 / 1024}'
cat /var/log/messages* | grep -o 'Memory:.*' | grep -o '[0-9]*K/' | grep -o '[0-9]*' | awk '{printf "Available:         %5.1f MB\n", $1 / 1024}'
cat /var/log/messages* | grep -o 'Memory:.*' | grep -o '[0-9]*K init' | grep -o '[0-9]*' | awk '{printf "init:              %5.1f MB\n", $1 / 1024}'
fi 

if [ $is_qemu == 1 ]; then
echo '                       QEMU'
echo '---------------------------'
dmesg | grep -o 'Memory:.*' | cut -d/ -f2 | grep -o [0-9]* | awk '{printf "Physical:          %5.1f MB\n", $1 / 1024}'
dmesg | grep -o 'Memory:.*' | grep -o '[0-9]*K reserved' | grep -o '[0-9]*' | awk '{printf "Kernel Static:     %5.1f MB\n", $1 / 1024}'
dmesg | grep -o 'Memory:.*' | grep -o '[0-9]*K/' | grep -o '[0-9]*' | awk '{printf "Available:         %5.1f MB\n", $1 / 1024}'
dmesg | grep -o 'Memory:.*' | grep -o '[0-9]*K init' | grep -o [0-9]* | awk '{printf "init:              %5.1f MB\n", $1 / 1024}'
fi 

echo '---------------------------'
meminfo MemTotal _memtotal
meminfo MemFree _memfree
echo '                   ------ -'
echo $_memtotal $_memfree | awk '{printf "                %8.1f MB\n", ($1 - $2) / 1024 }'
echo '---------------------------'

meminfo KernelStack _kernelstack
meminfo PageTables _pagetables
meminfo Slab _slab
meminfo SReclaimable _sreclaimable
meminfo SUnreclaim _sunreclaim

cat /proc/slabinfo | awk '{ SUM += ($3 * $4 / (1024 * 1024))} END { printf "Slabinfo:          %5.1f MB\n", SUM }'

lsmod | awk '{SUM += $2} END {printf "Kernel Modules: %8.1f MB\n", SUM / (1024*1024) }'
_kmodules=$(lsmod | awk '{SUM += $2} END {printf SUM / 1024}')

cat /proc/vmallocinfo | grep vmalloc | awk '{ SUM += ($2 / (1024 * 1024))} END { printf "Kernel VMalloced:%7.1f MB\n", SUM }'
_vmalloced=$(cat /proc/vmallocinfo | grep vmalloc | awk '{ SUM += ($2 / (1 * 1024))} END { print SUM }')

echo '                   ------ +'
echo $_kernelstack $_pagetables $_slab $_kmodules $_vmalloced | awk '{printf "                %8.1f MB\n", ($1 + $2 + $3 + $4 + $5) / 1024 }'

echo '---------------------------'

meminfo Buffers _buffers
meminfo Cached _cached
meminfo AnonPages _anonpages

echo '                   ------ +'
echo $_buffers $_cached $_anonpages | awk '{printf "                %8.1f MB\n", ($1 + $2 + $3) / 1024 }'
#find / -path /sys -prune -o -path /proc -prune -o -type f -exec fincore --pages=false --only-cached {} \; | grep -E '^/' | awk '{ SUM += $5} END { print SUM/1024 }'

echo '---------------------------'
meminfo "Active:" _active
meminfo "Active(anon):"
meminfo "Active(file):"
meminfo "Inactive:" _inactive
meminfo "Inactive(anon):"
meminfo "Inactive(file):"
meminfo Mlocked _mlocked

echo '                   ------ +'
echo $_active $_inactive $_mlocked | awk '{printf "                %8.1f MB\n", ($1 + $2 + $3) / 1024 }'

echo '---------------------------'

#meminfo Unevictable
#meminfo VmallocUsed:
#meminfo Committed_AS:

