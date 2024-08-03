#!/usr/bin/env python3
#
# Source: https://github.com/BrianEnigma/BarbieExtract/issues/1#issuecomment-2265291139
#
# > I have the decoder working now. It is partly based on a decompile so it is a bit rough. It seems like WPK is 
# > actually lossy and rounds the samples to 7 bits. Decoding CREAK.WPK does not produce exactly the original 
# > WAV, but it is very close. The names all sound good anyway.
# >
# > The final thing left to do is figure out the string encoding for the handful of names that have non-ascii characters.
# >
# > Here is a complete tool to list, extract, or play any name. Depends on click, numpy, and optionally miniaudio, tqdm.
# >
# > `Run barbie.py --help`

import json
import pathlib
import struct
from collections import defaultdict

import numpy as np


class HUG:
    def __init__(self, path):
        self._path = pathlib.Path(path)
        _, wpks = self.read_lst()
        _, _, _, hugmap = self.read_hix()
        self._positions = {}
        reverse_tmp = {}
        for (offset, length), canonical_name in zip(sorted(hugmap.keys()), wpks):
            self._positions[canonical_name[:-4]] = (offset, length)
            reverse_tmp[(offset, length)] = canonical_name[:-4]

        self._names = {}
        self._homophones = defaultdict(list)
        for (offset, length), names in hugmap.items():
            for name in names:
                self._names[name] = reverse_tmp[offset, length]
                self._homophones[reverse_tmp[offset, length]].append(name)

        self._hug_data = open(self._path / 'pinames.hug', 'rb').read()

    def canonical(self, name):
        return self._names[name]

    def position(self, name):
        return self._positions[self.canonical(name)]

    def homophones(self, name):
        return self._homophones[self.canonical(name)]

    def raw_data(self, name):
        offset, length = self.position(name)
        return self._hug_data[offset:offset+length]

    def wav(self, name):
        raw = self.raw_data(name)
        table = struct.unpack("<16b", raw[:16])
        header_len, = struct.unpack("<I", raw[16:20])
        header = raw[20:20+header_len]
        assert len(header) == 44
        orig_len, = struct.unpack('<I', header[-4:])
        compressed_len, = struct.unpack('<I', raw[20 + header_len:20 + header_len + 4])
        compressed = raw[20 + header_len + 4:]
        original = np.empty((orig_len + 0x80,), dtype=np.uint8)
        original[:] = 0x80 # fill with silence
        assert len(compressed) == compressed_len
        bVar8 = 0x80
        local_14c = 0x80
        orig_pos = 0
        comp_pos = 0
        if compressed_len > 0:
            while True:
                if orig_pos >= orig_len:
                    break
                bVar3 = int(compressed[comp_pos] >> 4)
                if bVar3 == 0xf:
                    bVar3 = (compressed[comp_pos] << 4) & 0xff
                    iVar12 = comp_pos + 1
                    if (bVar3 != 0xf0) or ((compressed[iVar12] & 0xf0) != 0xf0):
                        bVar8 = bVar3 | compressed[iVar12] >> 4
                        original[orig_pos] = bVar8
                        local_14c = bVar8
                        orig_pos += 1
                    else:
                        bVar3 = ((compressed[iVar12] << 4) & 0xff) | (compressed[comp_pos + 2] >> 4)
                        iVar12 = comp_pos + 2
                        if bVar3 != 0:
                            original[orig_pos:orig_pos+bVar3] = bVar8
                            orig_pos += bVar3
                            bVar8 = local_14c
                else:
                    iVar2 = int(local_14c) + table[bVar3]
                    iVar12 = comp_pos
                    if iVar2 < 0x100:
                        if iVar2 < 0:
                            bVar8 = 0
                            local_14c = 0
                            original[orig_pos] = 0
                        else:
                            bVar8 = int(bVar8) + table[bVar3]
                            local_14c = bVar8
                            original[orig_pos] = bVar8
                    else:
                        bVar8 = 0xff
                        local_14c = 0xff
                        original[orig_pos] = 0xff
                    orig_pos += 1

                if (compressed[iVar12] & 0xf) == 0xf:
                    comp_pos = iVar12 + 1
                    bVar3 = compressed[comp_pos]
                    if bVar3 != 0xff:
                        original[orig_pos] = bVar3
                        local_14c = bVar3
                        orig_pos += 1
                        bVar8 = bVar3
                    else:
                        comp_pos = iVar12 + 2
                        bVar3 = compressed[comp_pos]
                        if bVar3 != 0:
                            original[orig_pos:orig_pos+bVar3] = bVar8
                            orig_pos += int(bVar3)
                            bVar8 = local_14c
                else:
                    comp_pos = iVar12
                    iVar2 = local_14c + table[compressed[comp_pos] & 0xf]
                    if iVar2 < 0x100:
                        if iVar2 < 0:
                            bVar3 = 0
                            local_14c = 0
                            original[orig_pos] = 0
                        else:
                            bVar3 = bVar8 + table[compressed[iVar12] & 0xf]
                            local_14c = bVar3
                            original[orig_pos] = bVar3
                    else:
                        bVar3 = 0xff
                        local_14c = 0xff
                        original[orig_pos] = 0xff
                    orig_pos += 1
                    bVar8 = bVar3
                comp_pos += 1
                if comp_pos >= compressed_len:
                    break

        return header, original


    def play(self, name):
        import miniaudio, time
        header, pcm = self.wav(name)
        with miniaudio.PlaybackDevice(output_format=miniaudio.SampleFormat.UNSIGNED8,
                                      nchannels=1, sample_rate=22050,
                                      buffersize_msec=500) as device:
            stream = miniaudio.stream_raw_pcm_memory(pcm, 1, 1)
            device.start(stream)
            time.sleep((len(pcm) + 1000) / 22050)

    def read_lst(self):
        with open(self._path / 'pinames.lst', 'rb') as lst:
            n, = struct.unpack('<I', lst.read(4))
            names = [name.rstrip(b'\x00') for name in struct.unpack("20s" * n, lst.read(20 * n))]
            n, = struct.unpack('<I', lst.read(4))
            wpks = [wpk.rstrip(b'\x00') for wpk in struct.unpack("20s" * n, lst.read(20 * n))]
            return names, wpks

    def read_hix(self):
        with open(self._path / 'pinames.hix', 'rb') as lst:
            n, = struct.unpack('<I', lst.read(4))
            digrams = struct.unpack("2s" * n, lst.read(2 * n))
            offsets = struct.unpack(f'<{n}I', lst.read(4 * n))
            counts = struct.unpack(f'<{n}I', lst.read(4 * n))
            total = sum(counts)
            hugmap = defaultdict(list)
            for i in range(total):
                name, hug_offset, hug_length = struct.unpack('<20sII', lst.read(28))
                hugmap[hug_offset, hug_length].append(name.rstrip(b'\x00'))
            return digrams, offsets, counts, hugmap

    @property
    def names(self):
        return sorted(self._positions.keys())


if __name__ == '__main__':
    import click
    default_encoding = "latin1"


    @click.group()
    def barbie():
        pass


    @barbie.command()
    @click.argument("name", type=str, default='zara')
    @click.option("-d", "--data", type=click.Path(exists=True), default="data/")
    @click.option("-e", "--encoding", type=str, default=default_encoding)
    def play(name, data, encoding):
        """Play a name using miniaudio"""
        print(name, data)
        name = name.encode(encoding)
        h = HUG(data)
        print('Canonical:', h.canonical(name).decode(encoding))
        print('Homophones:', ', '.join(n.decode(encoding) for n in h.homophones(name)))
        h.play(name)


    @barbie.command()
    @click.argument("name", type=str, default='zara')
    @click.option("-d", "--data", type=click.Path(exists=True), default="./")
    @click.option("-e", "--encoding", type=str, default=default_encoding)
    def lookup(name, data, encoding):
        """Look up a name in the index"""
        print(name, data)
        name = name.encode(encoding)
        h = HUG(data)
        print('Canonical:', h.canonical(name))
        print('Homophones:', h.homophones(name))
        print('Position:', h.position(name))


    @barbie.command()
    @click.argument("outdir", type=click.Path(writable=True), default="out/")
    @click.option("-d", "--data", type=click.Path(exists=True), default="./")
    @click.option("-e", "--encoding", type=str, default=default_encoding)
    @click.option("--wpk", is_flag=True, help="Dump compressed WPK files instead of WAV")
    def dump(outdir, data, encoding, wpk):
        """Dump all names to WAV files (or WPK)"""
        outdir = pathlib.Path(outdir)
        h = HUG(data)
        outdir.mkdir(parents=True, exist_ok=True)
        names = h.names
        try:
            from tqdm import tqdm
            names = tqdm(names)
        except ImportError:
            pass
        for name in names:
            fn = (outdir / name.decode(encoding))
            if wpk:
                fn.with_suffix('.wpk').write_bytes(h.raw_data(name))
            else:
                fn.with_suffix('.wav').write_bytes(b''.join(h.wav(name)))


    @barbie.command(name="list")
    @click.option("-d", "--data", type=click.Path(exists=True), default="./")
    @click.option("-e", "--encoding", type=str, default=default_encoding)
    @click.option("-j", "--as-json", is_flag=True)
    def _list(data, encoding, as_json):
        """List all names in the index"""
        h = HUG(data)
        if not as_json:
            for name in h.names:
                print(name.decode(encoding), '=', ', '.join(n.decode(encoding) for n in h.homophones(name)))
        else:
            output = {}
            for name in h.names:
                output[name.decode(encoding)] = [n.decode(encoding) for n in h.homophones(name)]
            print(json.dumps(output, indent=4))



    barbie()
