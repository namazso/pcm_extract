#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_bit_arg(const char* arg) {
  char* end = NULL;
  long v = strtol(arg, &end, 10);
  if (end != arg + strlen(arg))
    return -1;
  if (v < 0 || v >= 8)
    return -1;
  return v;
}

int main(int argc, char** argv) {
#ifdef WIN32
  setmode(0, O_BINARY);
  setmode(1, O_BINARY);
#endif

  if (argc != 4) {
    fprintf(stderr, "Incorrect arguments! Usage: %s <BCK> <DOUT> <LRCK>\n", argv[0]);
    return 1;
  }

  int bck_shift = parse_bit_arg(argv[1]);
  int dout_shift = parse_bit_arg(argv[2]);
  int lrck_shift = parse_bit_arg(argv[3]);

  if (bck_shift == -1 || dout_shift == -1 || lrck_shift == -1 || bck_shift == dout_shift || bck_shift == lrck_shift || dout_shift == lrck_shift) {
    fprintf(stderr, "Invalid bit index\n");
    return 2;
  }

  uint8_t buf[4096];
  int ret = read(0, buf, sizeof(buf));
  if (ret == -1) {
    int err = errno;
    fprintf(stderr, "Bad initial read: %d (%s)\n", err, strerror(err));
    return 3;
  }

  uint8_t bck_mask = 1 << bck_shift;
  uint8_t dout_mask = 1 << dout_shift;
  uint8_t lrck_mask = 1 << lrck_shift;

  size_t begin_offset = 0;

  while (buf[begin_offset] & lrck_mask || !(buf[begin_offset] & bck_mask)) {
    ++begin_offset;
    if (begin_offset == sizeof(buf)) {
      fprintf(stderr, "Bad data: Always left channel or no BCK\n");
      return 4;
    }
  }

  while (!(buf[begin_offset] & lrck_mask) || !(buf[begin_offset] & bck_mask)) {
    ++begin_offset;
    if (begin_offset == sizeof(buf)) {
      fprintf(stderr, "Bad data: Always right channel\n");
      return 5;
    }
  }

  size_t len = sizeof(buf) - begin_offset;
  memmove(buf, buf + begin_offset, len);

  uint32_t lsamp = 0, rsamp = 0;
  size_t lsampc = 0, rsampc = 0;
  int lastlrck = 0, lastbck = 0;

  uint8_t out[8 * 128];
  size_t outc = 0;

  do {
    for (size_t i = 0; i < len; ++i) {
      uint8_t s = buf[i];

      int bck = !!(s & bck_mask);
      int lrck = !!(s & lrck_mask);
      int dout = !!(s & dout_mask);

      if (!bck) {
        lastbck = 0;
        continue;
      }
      if (lastbck) {
        continue;
      }

      lastbck = 1;

      if (lrck && !lastlrck) {
        if (lsampc == 0 && rsampc == 0) {
          (void)0; // first frame
        } else if (lsampc != 32 || rsampc != 32) {
          fprintf(stderr, "Warning: dropped frame (l: %zu, r: %zu)\n", lsampc, rsampc);
        } else {
          out[outc++] = lsamp & 0xFF;
          out[outc++] = (lsamp >> 8) & 0xFF;
          out[outc++] = (lsamp >> 16) & 0xFF;
          out[outc++] = (lsamp >> 24) & 0xFF;
          out[outc++] = rsamp & 0xFF;
          out[outc++] = (rsamp >> 8) & 0xFF;
          out[outc++] = (rsamp >> 16) & 0xFF;
          out[outc++] = (rsamp >> 24) & 0xFF;

          if (outc == sizeof(out)) {
            size_t written = 0;
            while (written != outc) {
              ret = write(1, out + written, outc - written);
              if (ret == -1) {
                int err = errno;
                fprintf(stderr, "Cannot write: %d (%s)\n", err, strerror(err));
                return 7;
              }
              written += ret;
            }
            outc = 0;
          }
        }
        lsamp = 0;
        rsamp = 0;
        lsampc = 0;
        rsampc = 0;
      }

      if (lrck) {
        lsamp <<= 1;
        lsamp |= dout;
        ++lsampc;
      } else {
        rsamp <<= 1;
        rsamp |= dout;
        ++rsampc;
      }

      lastlrck = lrck;
    }

    ret = read(0, buf, sizeof(buf));
    if (ret == -1) {
      int err = errno;
      fprintf(stderr, "Cannot read: %d (%s)\n", err, strerror(err));
      return 6;
    }
    len = ret;
  } while (len != 0);

  outc -= outc % 8;

  size_t written = 0;
  while (written != outc) {
    ret = write(1, out + written, outc - written);
    if (ret == -1) {
      int err = errno;
      fprintf(stderr, "Cannot write: %d (%s)\n", err, strerror(err));
      return 7;
    }
    written += ret;
  }

  return 0;
}
