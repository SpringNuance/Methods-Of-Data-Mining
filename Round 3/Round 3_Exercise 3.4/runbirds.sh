./namescodes/namescodes -n birdstrans.txt -t birdtable.txt -L
./namescodes/namescodes -n constraint.txt -t birdtable.txt
./kingfisher/kingfisher -i birdstrans.txt.codes -k 140 -M -5 -q 300 -o birdrules.txt - b constraint.txt.codes
./namescodes/namescodes -c birdrules.txt -t birdtable.txt
