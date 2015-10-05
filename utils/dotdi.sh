#!/bin/bash

for i in `find . -regex '\.\/[0-9]+.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdira2 $i  libs > ${i%%.tdi}ra.tdi ; done
for i in `find . -regex '\.\/[0-9]+ra.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdiprune $i 'VK|]m$|]r$' > ${i%%.tdi}pruneVKmr.tdi ; done

/home/rev/git/tditracer/utils/tdimerge *rapruneVKmr.tdi > combined1-199-rapruneVKmr.tdi

for i in `find . -regex '\.\/[0-9]+.tdi' | sort -V` ; do echo $i-\>${i%%.tdi}VKarenaFont.tdi && /home/rev/git/tditracer/utils/tdiprune $i 'VK|arena|Font' > ${i%%.tdi}VKarenaFont.tdi ; done
for i in `find . -regex '\.\/[0-9]+ra.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdiprune $i 'VK|arena|Font|1312|]m$|]r$' > ${i%%.tdi}pruneVKmr.tdi ; done

/home/rev/git/tditracer/utils/tdimerge *.tdi > combined1-499.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-499.tdi 'MAPS|VK|arena|Font|1312|]m$|]r$' > combined1-499prune.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-499.tdi 'MAPS|VK|arena|Font|1312' > combined1-499-prune.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-499.tdi '|VK|arena|' > combined1-499-prune.tdi

for i in `find . -regex '\.\/[0-9]+.tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdiprune $i 'VK|add_|prune' > ${i%%.tdi}pr.tdi ; done

################
/home/rev/git/tditracer/utils/tdimerge 1.tdi 2.tdi 3.tdi 4.tdi 5.tdi 6.tdi 7.tdi  > combined1-7.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-7.tdi 'MAPS|VK|arena|]m$|]r$' > combined1-7pr.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-7.tdi 'VK|arena' > combined1-7prVKarena.tdi

################
/home/rev/git/tditracer/utils/tdimerge *.tdi  > combined1-12.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-12.tdi 'MAPS|VK|arena|]m$|]r$' > combined1-12pr.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-12.tdi 'VK|arena' > combined1-12prVKarena.tdi

################
for i in `find . -regex 'tdi' | sort -V` ; do echo $i && /home/rev/git/tditracer/utils/tdira2 $i libs > ${i%%.tdi}ra.tdi ; done

################
for i in `find . -name 'tditracebuffer*' | sort -V` ; do echo $i && tdidump $i > ${i%%.tdi}.tdi ; done
/home/rev/git/tditracer/utils/tdimerge *.tdi  > combined1-18.tdi
/home/rev/git/tditracer/utils/tdiprune combined1-18.tdi 'arena' > combined1-18-prarena.tdi


/home/rev/git/tditracer/utils/tdiprune combined1-45.tdi 'OK|arena' > combined1-45-prOKarena.tdi
