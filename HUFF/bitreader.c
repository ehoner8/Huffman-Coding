#include "bitreader.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct BitReader {
    FILE *underlying_stream;
    uint8_t byte;
    uint8_t bit_position;
};

BitReader *bit_read_open(const char *filename) {
    BitReader *b = (BitReader *) malloc(sizeof(BitReader));
    if (b == NULL) {
        return NULL;
    }
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        return NULL;
    }
    b->underlying_stream = f;
    b->byte = 0;
    b->bit_position = 8;
    return b;
}

void bit_read_close(BitReader **pbuf) {
    if (*pbuf != NULL) {
        int fc = fclose((*pbuf)->underlying_stream);
        if (fc != 0) {
            exit(1);
        }
        free(*pbuf);
        *pbuf = NULL;
    }
}

uint8_t bit_read_bit(BitReader *buf) {
    if (buf->bit_position > 7) {
        int fr = fgetc(buf->underlying_stream);
        if (fr == EOF) {
            exit(1);
        }
        buf->byte = (uint8_t) fr;
        buf->bit_position = 0;
    }
    uint8_t res;
    res = buf->byte & (0x01 << buf->bit_position);
    res = res >> buf->bit_position;
    buf->bit_position += 1;
    return res;
}

uint8_t bit_read_uint8(BitReader *buf) {
    uint8_t byte = 0x00;
    uint8_t bit_read;
    for (int i = 0; i < 8; i++) {
        bit_read = bit_read_bit(buf);
        if (bit_read == 0) {
            byte = byte & (~(0x01 << i));
        }
        if (bit_read == 1) {
            byte = byte | (uint8_t) (0x01 << i);
        }
    }
    return byte;
}

uint16_t bit_read_uint16(BitReader *buf) {
    uint16_t word = 0x0000;
    uint8_t bit_read;
    for (int i = 0; i < 16; i++) {
        bit_read = bit_read_bit(buf);
        if (bit_read == 0) {
            word = word & (~(0x01 << i));
        }
        if (bit_read == 1) {
            word = word | (uint16_t) (0x01 << i);
        }
    }
    return word;
}

uint32_t bit_read_uint32(BitReader *buf) {
    uint32_t word = 0x00000000;
    uint8_t bit_read;
    for (int i = 0; i < 32; i++) {
        bit_read = bit_read_bit(buf);
        if (bit_read == 0) {
            word = word & (~(0x01 << i));
        }
        if (bit_read == 1) {
            word = word | (0x01 << i);
        }
    }
    return word;
}
