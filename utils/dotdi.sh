#!/bin/bash

for i in `find -name '*.tdi' | sort -V` ; do /home/rev/git/tditracer/utils/tdiprune $i  'VK_|mi.' > ${i%%.tdi}_prune.tdi ; done
