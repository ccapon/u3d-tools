/*
 * Copyright (C) 2016 Hiroka Ihara
 *
 * This file is part of libU3D.
 *
 * libU3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libU3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libU3D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "u3d_internal.hh"
#include <iomanip>

namespace U3D
{

const uint8_t BitStreamReader::bit_reverse_table[256] =
{
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

BitStreamReader::BitStreamReader(const std::string& filename) : bit_position(0), high(0xFFFF), low(0), underflow(0), data_buffer(NULL), metadata_buffer(NULL)
{
    ifs.open(filename.c_str(), std::ifstream::binary);
    if(!ifs.is_open()) {
        U3D_WARNING << "Failed to open: " << filename << "." << std::endl;
    } else {
        U3D_LOG << filename << " opened." << std::endl;
    }
}

bool BitStreamReader::open_block()
{
    if(!ifs.is_open()) return false;
    std::cerr << "Reading block @x" << std::hex << std::setfill('0') << std::setw(4) << ifs.tellg() << std::dec << std::endl;
    reset();
    type = read_word_direct();
    if(ifs.eof()) return false;
    data_size = read_word_direct();
    metadata_size = read_word_direct();
    if(data_buffer != NULL) delete[] data_buffer;
    if(metadata_buffer != NULL) delete[] metadata_buffer;
    data_buffer = new uint32_t[(data_size + 3) / 4 + 1];
    metadata_buffer = new uint32_t[(metadata_size + 3) / 4 + 1];
    ifs.read(reinterpret_cast<char *>(data_buffer), (data_size + 3) / 4 * 4);
    ifs.read(reinterpret_cast<char *>(metadata_buffer), (metadata_size + 3) / 4 * 4);
    data_buffer[(data_size + 3) / 4] = 0;
    metadata_buffer[(metadata_size + 3) / 4] = 0;
    bit_position = 0;
    return true;
}



uint32_t BitStreamReader::read_static_symbol(uint32_t context)
{
    size_t checkpoint = bit_position;
    uint32_t code = read_bit() << 15;
    bit_position += underflow;
    uint32_t temp = read_bits(15);
    code |= (bit_reverse_table[temp & 0xFF] << 7) | (bit_reverse_table[temp >> 8] >> 1);
    bit_position = checkpoint;

    uint32_t range = high + 1 - low;
    uint32_t cum_freq = (context * (1 + code - low) - 1) / range;
    uint32_t value = cum_freq + 1;

    high = low + range * (cum_freq + 1) / context - 1;
    low = low + range * cum_freq / context;

    unsigned int bit_count = 0;
    while((low & 0x8000) == (high & 0x8000)) {
        low = (0x7FFF & low) << 1;
        high = ((0x7FFF & high) << 1) | 1;
        bit_count++;
    }
    if(bit_count > 0) {
        bit_count += underflow;
        underflow = 0;
    }
    while((low & 0x4000) && !(high & 0x4000)) {
        low = (low & 0x3FFF) << 1;
        high = ((high & 0x3FFF) << 1) | 0x8001;
        underflow++;
    }
    bit_position += bit_count;
    return value;
}

uint32_t BitStreamReader::read_dynamic_symbol(uint32_t context)
{
    size_t checkpoint = bit_position;
    uint32_t code = read_bit() << 15;
    bit_position += underflow;
    uint32_t temp = read_bits(15);
    code |= (bit_reverse_table[temp & 0xFF] << 7) | (bit_reverse_table[temp >> 8] >> 1);
    bit_position = checkpoint;

    uint32_t range = high + 1 - low;
    uint32_t total_freq = dynamic_contexts[context].get_total_symbol_frequency();
    uint32_t cum_freq = (total_freq * (1 + code - low) - 1) / range;
    uint32_t val_cum_freq;
    uint32_t symbol = dynamic_contexts[context].get_symbol_from_frequency(cum_freq, &val_cum_freq);
    uint32_t val_freq = dynamic_contexts[context].get_symbol_frequency(symbol);

    high = low - 1 + range * (val_cum_freq + val_freq) / total_freq;
    low = low + range * val_cum_freq / total_freq;

    dynamic_contexts[context].add_symbol(symbol);

    unsigned int bit_count = 0;
    while((low & 0x8000) == (high & 0x8000)) {
        low = (0x7FFF & low) << 1;
        high = ((0x7FFF & high) << 1) | 1;
        bit_count++;
    }
    if(bit_count > 0) {
        bit_count += underflow;
        underflow = 0;
    }
    while((low & 0x4000) && !(high & 0x4000)) {
        low = (low & 0x3FFF) << 1;
        high = ((high & 0x3FFF) << 1) | 0x8001;
        underflow++;
    }
    bit_position += bit_count;
    if(bit_position >= 8 * data_size) {
        U3D_ERROR << "Data buffer overrun.";
    }
    return symbol;
}

}
