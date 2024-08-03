# Detective Barbie 2: The Vacation Mystery

Prototyping some extraction code... extracts some files that are ALMOST audible wav voices, but there's a lot of static.

More information and background at this blog post: <https://netninja.com/2024/08/01/the-mystery-of-detective-barbies-audio/>

## Acknowledgements

Although I did a lot of initial exploration, reverse-engineering the file format in broad strokes, and some extraction and analysis tools, I never broke the custom compression format. Much respect and esteem goes out to the folks who created rallied behind the [GitHub issue](https://github.com/BrianEnigma/BarbieExtract/issues/1) around adding decoding: [Alistair Buxton](https://github.com/ali1234) and [Mark Steward](https://github.com/marksteward).

And credit to [Andy Baio](https://waxy.org) for nerdsniping a bunch of us with this whole thing.

## References

- [TikTok](https://www.tiktok.com/t/ZTNQ1jY9S/)
- [WAV file format](https://docs.fileformat.com/audio/wav/)

## Source Game Files

These are needed. The `make download` Makefile target will download them for you from archive.org.

- pinames.hug: <https://archive.org/download/BarbieDetective2VacationMystery/23466.iso/DATA%2Fpinames.hug>
    - This is a concatenated list of WAV files of all the spoken names. The WAV headers indicate file lengths that are actually longer than the given data, meaning the samples are compressed. This contains 9,342 “compressed” WAV files.
- pinames.hix: <https://archive.org/download/BarbieDetective2VacationMystery/23466.iso/DATA%2Fpinames.hix>
    - This is an index, both internally and into the `hug` file. It actually contains 50,014 (homonym) names that map to the 9,342 WAV files.
- pinames.lst: <https://archive.org/download/BarbieDetective2VacationMystery/23466.iso/DATA%2Fpinames.lst>
    - The assumption is that the `lst` file is an artifact of authoring tools and is unused by the game itself. It lists the 9,342 canonical names.

## Extracting

With much thanks to [Alistair](https://github.com/ali1234), who posted a Python script to the [GitHub issue](https://github.com/BrianEnigma/BarbieExtract/issues/1), we now have `barbie.py` that can successfully extract *AND* decompress the WAV files from the `hug` archive.

Install the prerequisites:

```
python3 -m pip install --user -r requirements.txt
```

List the canonical names and their homonyms:

```
./barbie.py list
```

Alternately, list them as json:

```
./barbie.py list -j
```

Extract all of the wav files:

```
./barbie.py dump
```

## Weirdness In The Files

- The `hug` file appears to be a bunch of WAV files concatenated together. This would match with how I'd put something like this together using 1999 technology. The name index files would point to offsets and lengths within the audio archive, and you'd just play out that chunk of file, without having to worry about first decompressing the whole thing.
- The first audio file extracted is `output/file-000001-9156.wav`
- The make target `test.wav` also extracts the same, but using the `dd` command.
- The weird thing about this first file is that actual file size is `9135`, but the wav header within the file proclaims that the length should be `19682`. Other interesting things in the header:
    - PCM format
    - Single channel (mono)
    - Sample rate 22,050
    - 8 bits per sample
- Based on these headers, the file should be 0.9 seconds long, but it's really 0.4 seconds long, due to the truncation.
- Some of them are _almost_ listenable. You can make out some vocalization, inflection, and diction in there behind varying amounts of digital noise.

## File Formats

(to be here)

## Makefile Targets

### `make barbie_extract`

Builds the `barbie_extract` program. This looks for WAV headers in the `hug` file and uses those as boundary points. It then starts extracting each file into the `./output/` folder. These filenames take the format `file-<counter>-<offset within hug file>.wav`. The code currently has a hard-coded throttle to limit it to the first 50 WAV files, but that can be edited. Note that these are the _compressed_ WAV files and can sound like garbage.

### `make rle_test`

Builds the `rle_test` program. This takes `test.wav` and attempts 9 possible decompression algorithms on it. These include assorted run-length encodings, some 4-bit to 8-bit expansions, and one that combines the two. None of them produce listenable audio.

### `make analyze`

Builds the `analyze` program, which does two things. First, it performs a frequency analysis on `test.wav`, showing there are a lot of zeros. Here are the first few lines:

```
FREQUENCY ANALYSIS
~~~~~~~~~ ~~~~~~~~
238	[0x00]
209	[0x11]
186	[0x02]
173	[0x22]
172	[0x20]
```

Secondly, with the assumption that `0x00` and `0x00 0x00` are special RLE tokens, it locates each zero in the sample bytes and prints the 3 bytes immediately before and after, to help aid in looking for patterns. For example, here are the first few lines:

```
ZERO NEIGHBOR ANALYSIS
~~~~ ~~~~~~~~ ~~~~~~~~
[0x6c, 0x23, 0x00, 0x00, 0x0f, 0xff, 0xfe]
[0x12, 0x20, 0x22, 0x00, 0x01, 0x30, 0x31]
[0x30, 0x31, 0x10, 0x00, 0x22, 0x40, 0x00]
[0x00, 0x22, 0x40, 0x00, 0x22, 0x02, 0x22]
[0x02, 0x22, 0x20, 0x00, 0x02, 0x22, 0x02]
```

### `make test.wav`

Without compiling, it uses the standard `dd` Unix command to extract the first WAV file as `test.wav`.

### `make test.bin`

Without compiling, it uses the standard `dd` Unix command to extract only the samples of the first WAV file (i.e. sans header) as `test.bin`.

### `make codecs.txt`

Produces a list of ffmpeg audio decoder codecs as `codecs.txt` and `codec_names.txt`. You'll need ffmpeg in your path. This gets used by `force_decode_all.sh` to do a brute-force decode of `test.wav` using every possible ffmpeg codec available. (This yielded no useful results.)

### `make downloads`

Downloads the three game files from `archive.org`

## License

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of this license, visit: <https://creativecommons.org/licenses/by-nc-sa/4.0/>
