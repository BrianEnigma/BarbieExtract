# Detective Barbie 2: The Vacation Mystery

Prototyping some extraction code... extracts some files that are ALMOST audible wav voices, but there's a lot of static.

## References

- [TikTok](https://www.tiktok.com/t/ZTNQ1jY9S/)
- [wav file format](https://docs.fileformat.com/audio/wav/)

## Source Files

These are needed. Remove the prefix so they are just `pinames.*`.

- pinames.hug: <https://archive.org/download/BarbieDetective2VacationMystery/23466.iso/DATA%2Fpinames.hug>
- pinames.hix: <https://archive.org/download/BarbieDetective2VacationMystery/23466.iso/DATA%2Fpinames.hix>
- pinames.lst: <https://archive.org/download/BarbieDetective2VacationMystery/23466.iso/DATA%2Fpinames.lst>

## Weirdness

- The `hug` file appears to be a bunch of WAV files concatenated together. This would match with how I'd put something like this together using 1999 technology. The name index files would point to offsets and lengths within the audio archive, and you'd just play out that chunk of file, without having to worry about first decompressing the whole thing. (I haven't yet examined the name index files.)
- The first audio file extracted is `output/file-000001-9156.wav`
- The make target `test.wav` also extracts the same, but using the `dd` command.
- The weird thing about this first file is that actual file size is `9135`, but the wav header within the file proclaims that the length should be `19682`. Other interesting things in the header:
    - PCM format
    - Single channel (mono)
    - Sample rate 22,050
    - 8 bits per sample
- Based on these headers, the file should be 0.9 seconds long, but it's really 0.4 seconds long, due to the truncation.

```
General
Complete name                            : test.wav
Format                                   : Wave
File size                                : 8.92 KiB
Duration                                 : 412 ms
Overall bit rate mode                    : Constant
Overall bit rate                         : 177 kb/s
IsTruncated                              : Yes

Audio
Format                                   : PCM
Format settings                          : Unsigned
Codec ID                                 : 1
Duration                                 : 412 ms
Bit rate mode                            : Constant
Bit rate                                 : 176.4 kb/s
Channel(s)                               : 1 channel
Sampling rate                            : 22.05 kHz
Bit depth                                : 8 bits
Stream size                              : 8.88 KiB (100%)
```

- The `test.bin` make target extracts just the binary payload (the audio samples) without the wav header. You can use Audacity's File > Import > Raw Data... command to try to import it, but I can't find settings that seem to work.

## License

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of this license, visit: <https://creativecommons.org/licenses/by-nc-sa/4.0/>
