all: barbie_extract rle_test analyze

barbie_extract: barbie_extract.cpp pinames.hug
	g++ -g -o $@ barbie_extract.cpp

iterate: barbie_extract
	./barbie_extract

rle_test: rle_test.cpp
	g++ -g -o $@ rle_test.cpp

analyze: analyze.cpp
	g++ -g --std=c++11 -o $@ analyze.cpp

# Extract from the first WAV header to the next one.
.PHONY: test.wav
test.wav: pinames.hug
	dd if=pinames.hug bs=1 skip=20 count=9135 of=test.wav

# Extract only the samples from the first WAV, without header.
.PHONY: test.bin
test.bin: pinames.hug
	dd if=pinames.hug bs=1 skip=60 count=9091 of=test.bin

codecs.txt:
	ffmpeg -codecs | grep '^ D.A' > codecs.txt
	# blinkaudio is broken and goes into an endless loop
	cat codecs.txt| cut -d ' ' -f 3 | grep -v binkaudio_rdft > codec_names.txt


.PHONY: clean
clean:
	rm -f barbie_extract rle_test analyze output/*.wav

