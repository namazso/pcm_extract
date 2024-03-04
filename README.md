# pcm_extract

A tool for extracting PCM encoded data from a bruteforce-sampled bitstream, primarily made for using the PCM1802 in combination with the [MSIRC](https://github.com/Stefan-Olt/MISRC/). Note that the sample rate must be more than twice the bandwidth, so that both low and high states of the bit clock can be observed.

## Usage

```
pcm_extract <BCK> <DOUT> <LRCK>
```

Provide the input on stdin, and the decoded audio is provided on stdout as signed 32 bit. Example usage with a MISRC capture:

```
misrc_extract -i test.bin -x - | pcm_extract 2 3 4 | ffmpeg -y -f s32le -ar 78125 -ac 2 -i - aux.flac 
```

This assumes the following pin setup:

- 0: BCK
- 1: DOUT
- 2: LRCK

misrc_extract pads the bottom by default, so each pin number had to be offset by 2.

Up to two PCM1802s can be connected to the aux bits, and you can extract their data individually by providing the correct pin numbers to pcm_extract.

## License

    MIT License
    
    Copyright (c) namazso 2024
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
