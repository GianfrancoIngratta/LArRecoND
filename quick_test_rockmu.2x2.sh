#!/bin/bash

type=CRRemHitsSliceNu
# type=AllHitsSliceNu
# type=AllHitsCR

file=$1
geom=$2

echo "input file is: ${file}" 

# xml_source=settings/PandoraSettings_LArRecoND_ThreeD_cheated_w_rockmuTagging.xml
xml_source=settings/PandoraSettings_LArRecoND_ThreeD_w_rockmuTagging.xml

./build/PandoraInterface -r $type -i $xml_source -e $file -g $geom -f SPMC -j Both -v volTPCActive -n 10 &> quick_test_rockmu.2x2.out 
