#include "bitwriter.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct BitWriter {
    FILE *underlying_stream;
    uint8_t byte;
    uint8_t bit_position;
};

BitWriter *bit_write_open(const char *filename) {
    BitWriter *b = (BitWriter *) malloc(sizeof(BitWriter));
    if (b == NULL) {
        return NULL;
    }
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        return NULL;
    }
    b->underlying_stream = f;
    b->byte = 0;
    b->bit_position = 0;
    return b;
}

void bit_write_close(BitWriter **pbuf) {
    if (*pbuf != NULL) {
        if ((*pbuf)->bit_position > 0) {
            fputc((*pbuf)->byte, (*pbuf)->underlying_stream);
        }
        fclose((*pbuf)->underlying_stream);
        free(*pbuf);
        *pbuf = NULL;
    }
}

void bit_write_bit(BitWriter *buf, uint8_t bit) {
    if (buf->bit_position > 7) {
        //printf("%u ", buf->byte);
        fputc(buf->byte, buf->underlying_stream);
        buf->byte = 0;
        buf->bit_position = 0;
    }
    assert(bit == 0 || bit == 1);
    if (bit == 0) {
        buf->byte = buf->byte & ~(0x01 << buf->bit_position);
    }
    if (bit == 1) {
        buf->byte = buf->byte | (uint8_t) (0x01 << buf->bit_position);
    }
    buf->bit_position += 1;
}

void bit_write_uint8(BitWriter *buf, uint8_t x) {
    uint8_t res;
    for (int i = 0; i < 8; i++) {
        res = x & (0x01 << i);
        res = res >> i;
        bit_write_bit(buf, res);
    }
}

void bit_write_uint16(BitWriter *buf, uint16_t x) {
    uint16_t res;
    for (int i = 0; i < 16; i++) {
        res = x & (0x01 << i);
        res = res >> i;
        bit_write_bit(buf, (uint8_t) res);
    }
}

void bit_write_uint32(BitWriter *buf, uint32_t x) {
    uint32_t res;
    for (int i = 0; i < 32; i++) {
        res = x & (0x01 << i);
        res = res >> i;
        bit_write_bit(buf, (uint8_t) res);
    }
}
