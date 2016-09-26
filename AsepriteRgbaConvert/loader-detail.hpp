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

template<typename T>
inline const char* read_object(const char* buf, T& target) {
  target = *reinterpret_cast<const T*>(buf);
  return buf + sizeof(T);
}

template<typename CT>
inline const char* read_object(const char* buf, std::vector<CT>& target) {
  size_t size = target.size();
  CT const* buf_start = reinterpret_cast<const CT*>(buf);
  std::copy(buf_start, buf_start + size, target.begin());
  return buf + size * sizeof(CT);
}

template<>
inline const char* read_object(const char* buf, std::string& target) {
  WORD s_len;
  buf = read_object(buf, s_len);
  target.resize(s_len);
  size_t size = target.size();
  const char* buf_start = reinterpret_cast<const char*>(buf);
  std::copy(buf_start, buf_start + size, target.begin());
  return buf + size;
}

template<>
inline const char* read_object(const char* buf, std::vector<Tag>& target) {
  struct tag_header {
    WORD tag_count;
    BYTE z0[8];
  } header;
  static_assert(sizeof(header) == 10, "Sizeof tag_header doesn't match header size of 10 bytes.");
  buf = read_object(buf, header);

  struct t_read_struct {
    WORD from;
    WORD to;
    BYTE loop_dir;
    BYTE z0[8];
    BYTE RGB[3];
    BYTE z1;
  } t;
  static_assert(sizeof(t) == 17, "Sizeof t_read_struct doesn't match header size of 17 bytes.");

  for ( size_t i = 0; i < header.tag_count; ++i ) {
    buf = read_object(buf, t);
    std::string s;
    buf = read_object(buf, s);
    target.emplace_back(Tag{ t.from, t.to, t.loop_dir, s });
    // Zero our inplace struct so we don't be so confused on next read.
    t = t_read_struct{};
  }

  return buf;
}

template <typename T>
T swap_endian(T u) {
  union {
    T u;
    BYTE u8[sizeof(T)];
  } source, dest;

  source.u = u;

  for ( size_t k = 0; k < sizeof(T); k++ )
    dest.u8[k] = source.u8[sizeof(T) - k - 1];

  return dest.u;
}

std::vector<PIXEL_RGBA> dest_blend_cels(const frame_cel& src, const frame_cel& dst);

} // namespace aseprite::details
} // namespace aseprite

#pragma pack(pop)