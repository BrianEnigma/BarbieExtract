all: barbie_extract rle_test

barbie_extract: barbie_extract.cpp pinames.hug
	g++ -g -o barbie_extract barbie_extract.cpp

iterate: barbie_extract
	./barbie_extract

rle_test: rle_test.cpp
	g++ -g -o rle_test rle_test.cpp

# Extract from the first WAV header to the next one.
.PHONY: test.wav
test.wav: pinames.hug
	dd if=pinames.hug bs=1 skip=20 count=9135 of=test.wav

# Extract only the samples from the first WAV, without header.
.PHONY: test.bin
test.bin: pinames.hug
	dd if=pinames.hug bs=1 skip=60 count=9091 of=test.bin

.PHONY: clean
clean:
	rm -f barbie_extract rle_test output/*.wav

