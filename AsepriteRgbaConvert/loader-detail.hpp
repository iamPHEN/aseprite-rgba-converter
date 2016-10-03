/// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#pragma once

#include "doc-lite.h"
#include <vector>
#include <string>

#pragma pack(push)
#pragma pack(1)
namespace aseprite{
namespace details {

static_assert(sizeof(BYTE) == 1, "Sizeof BYTE doesn't match header size of 1 bytes.");
static_assert(sizeof(WORD) == 2, "Sizeof WORD doesn't match header size of 2 bytes.");
static_assert(sizeof(DWORD) == 4, "Sizeof DWORD doesn't match header size of 4 bytes.");

struct ase_header {
  DWORD fsize;
  WORD magic;
  WORD frames;
  WORD width;
  WORD height;
  WORD depth;
  DWORD flags;
  WORD speed;
  DWORD z0;
  DWORD z1;
  BYTE transparent_index;
  BYTE z2[3];
  WORD color_count;
  BYTE z3[94];
};
static_assert(sizeof(ase_header) == 128, "Sizeof ase_header doesn't match header size of 128 bytes.");

struct frame_header {
  DWORD size;
  WORD  magic;
  WORD  chunks;
  WORD duration;
  BYTE z0[6];
};
static_assert(sizeof(frame_header) == 16, "Sizeof frame_header doesn't match header size of 16 bytes.");

enum chunk_type {
  old_pallet  = 0x0004,
  old_pallet2 = 0x0011,
  layer = 0x2004,
  cel = 0x2005,
  mask = 0x2016,
  path = 0x2017,
  frame_tags = 0x2018,
  pallet  = 0x2019,
  user_data = 0x2020,
};

struct frame_chunk {
  DWORD size;
  WORD type;
  std::vector<BYTE> data;
};

struct frame {
  frame_header header;
  std::vector<frame_chunk> chunks;
};

struct cel_header {
  WORD layer_index;
  SIGNED_WORD x;
  SIGNED_WORD y;
  BYTE opacity;
  WORD cell_type;
  BYTE z0[7];
};
static_assert(sizeof(cel_header) == 16, "Sizeof cel_header doesn't match header size of 16 bytes.");

struct frame_cel {
  cel_header c;
  WORD w, h;
  WORD linked = 0;
  // Assume RGBA
  std::vector<PIXEL_RGBA> pixels;
};

struct layer_header {
  WORD flags;
  WORD type;
  WORD child_level;
  WORD z0[2];
  WORD blend_mode;
  BYTE opacity;
  BYTE z1[3];
};

struct Layer {
  layer_header header;
  std::string name;
};

template<typename T>
const char* read_object(const char* buf, T& target);

extern template
const char* read_object(const char* buf, ase_header& target);

extern template
const char* read_object(const char* buf, frame_header& target);

extern template
const char* read_object(const char* buf, cel_header& target);

extern template
const char* read_object(const char* buf, PIXEL_RGBA& target);

extern template
const char* read_object(const char* buf, layer_header& target);

extern template
const char* read_object(const char* buf, unsigned int& target);

extern template
const char* read_object(const char* buf, unsigned char& target);


template<typename CT>
const char* read_object(const char* buf, std::vector<CT>& target);

extern template
const char* read_object(const char* buf, std::vector<unsigned char>& target);

extern template
const char* read_object(const char* buf, std::vector<PIXEL_RGBA>& target);

template<>
const char* read_object(const char* buf, std::string& target);

template<>
const char* read_object(const char* buf, std::vector<Tag>& target);

template <typename T>
T swap_endian(T u);

} // namespace aseprite::details
} // namespace aseprite

#pragma pack(pop)
