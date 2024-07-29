#!/bin/bash

if [ ! -f codec_names.txt ]; then
    make codecs.txt
fi

if [ ! output/file-000003-54252.wav ]; then
    make iterate
fi

COUNTER=1
mkdir -p reencodes
while read CODEC; do
    OUTPUT_FILENAME=reencodes/$COUNTER-$CODEC.wav
    echo "Decoding as $CODEC to $OUTPUT_FILENAME"
    ffmpeg -hide_banner -loglevel panic -vn -acodec $CODEC -i output/file-000003-54252.wav -y $OUTPUT_FILENAME
    COUNTER=$((COUNTER+1))
done <codec_names.txt

