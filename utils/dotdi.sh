#!/bin/bash

for i in `find . -regex '\.\/[0-9]+.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdira2 $i  libs > ${i%%.tdi}ra.tdi ; done

for i in `find . -regex '\.\/[0-9]+ra.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdiprune $i 'VK|]m$|]r$' > ${i%%.tdi}pruneVKmr.tdi ; done

/home/rev/git/tditracer/utils/tdimerge *rapruneVKmr.tdi > combined1-199-rapruneVKmr.tdi

for i in `find . -regex '\.\/[0-9]+.tdi' | sort -V` ; do echo $i-\>${i%%.tdi}VKarenaFont.tdi && /home/rev/git/tditracer/utils/tdiprune $i 'VK|arena|Font' > ${i%%.tdi}VKarenaFont.tdi ; done

for i in `find . -regex '\.\/[0-9]+ra.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdiprune $i 'VK|arena|Font|1312|]m$|]r$' > ${i%%.tdi}pruneVKmr.tdi ; done

/home/rev/git/tditracer/utils/tdimerge *.tdi > combined1-499.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-499.tdi 'MAPS|VK|arena|Font|1312|]m$|]r$' > combined1-499prune.tdi

/home/rev/git/tditracer/utils/tdiprune combined1-499.tdi 'MAPS|VK|arena|Font|1312' > combined1-499-prune.tdi
