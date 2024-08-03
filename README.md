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

## Extracting Audio

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

These fils all appear in the `./out/` folder.

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

### pinames.hix

This is a list of every name the game is aware of. This includes all spellings/homonyms. Each entry (spelling) points to a WAV within the `hug` file. Multiple entries can point to the same WAV, which is how we get the alternate-spellings/homonyms. It consists of three main sections. Everything is little endian, because — come on — this is a 90s game running on Intel processors.

```
table_length : 32 bits
for (i = 0; i < table_length; i++)
    letter_1 : 8 bits
    letter_2 : 8 bits
for (i = 0; i < table_length; i++)
    hix_file_offset : 32 bits
    tbd : 32 bits
for (...)
    players_name : 24 BYTES
    hug_offset : 32 bits
```

- `table_length` is the number of entries in the first and second tables
- Table 1:
    - `letter_1` and `letter_2` are the first two letters (characters, actually) of the player's name. This is a list such as `a'` (a apostrophe, for names like “A'leeshana”), `aa`, `ab`, `ac`, and so on. This is used so that, as the player types their name, the software can quickly index into the list of names, staring at names that begin with those two letters.
- Table 2:
    - Based on the index of where `letter_1` and `letter_2` are found in the first table, you jump to the same index in this second table.
    - `hix_file_offset` is the byte offset, within this very same file, where names staring with those two letters begin.
    - `tbd` (unknown)
- Table 3:
    - `players_name` is a null-terminated string for the player's name.
    - `hug_offset` is the byte offset, within the `pinames.hug` (concatenated WAV archive) of where to find audio that corresponds with that name.

### pinames.hug

This is where all of the raw audio lives.

```
0x0000 ... 0x0x23AF is header content of some sort.
0x23B0 is where the name content starts
for (...)
    entry_prefix : 16 BYTES
    wav_header_length : 32 bits
    // From here down to the compressed data is the standard WAV header.
    riff_marker : 32 bits
    file_size : 32 bits
    wave_marker : 32 bits
    fmt_marker : 32 bits
    fmt_length: 32 bits
    format_enum : 16 bits
    channel_count : 16 bits
    sample_rate : 32 bits
    sample_calc1 : 32 bits
    sample_calc2 : 16 bits
    bits_per_sample : 16 bits
    data_marker : 32 bits
    data_length: 32 bits
    // Compressed data starts here.
    for (...)
        data_byte : 8 bit
```

- `entry_prefix` is the literal {0x00, 0x01, 0xFF, 0x03, 0xFD, 0x06, 0xFA, 0x0A, 0xF6, 0x0F, 0xF1, 0x15, 0xEB, 0x1C, 0xE4, 0x7F}. Likely, it serves as a magic prefix to ensure that the offset jump was correct?
- `wav_header_length` is the literal 0x2C (the length of a standard WAV header)
- `riff_marker` is the literal string RIFF.
- `file_size` is the size the WAV file _WOULD_ be, when uncompressed.
- `wave_marker` is the literal string `WAVE`.
- `fmt_marker` is the literal string `fmt `.
- `fmt_length` is the literal value 0x10 (the previous 16 bytes)
- `format_enum` is the literal 0x01 (PCM)
- `channel_count` is the literal 0x01 (mono)
- `sample_rate` is the literal 22,050
- `sample_calc1` is (Sample Rate * BitsPerSample * Channels) / 8, which is the literal 22,050 again.
- `sample_calc2` is another calculation, but the 0x01 in this case.
- `bits_per_sample` is the literal 0x08 (but this is a lie)
- `data_marker` is the literal `data`
- `data_length` is the the size the data section _WOULD_ be, when uncompressed.

### WAV Compression

Once the WAV file is extracted, decompression has to happen on the `data` section of samples. The compressed data is in 4-bit chunks, with 0xf being a special escape code that controls how subsequent chunks are decoded.

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
