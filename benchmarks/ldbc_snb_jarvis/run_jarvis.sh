#!/bin/bash

set -e
export LD_LIBRARY_PATH=/opt/intel/vdms/pmgd/lib:/opt/intel/pmgd-evaluation/LDBC/ldbc_snb_jarvis/src/jni
#make -C src/queries clean
#make -C src/jni clean
make -C src/queries 
make -C src/jni 
gradle build
#sh loadgraph.sh ../datasets/sf10/data/social_network_jarvis/ ../databases/jarvis/sf10-graph ../jarvis/
gradle -q ldbc
#queries=( "" )
