#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#pragma pack(push)
#pragma pack(1)

using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using LONG = int32_t;
using STRING_LEN = WORD;

static_assert(CHAR_BIT == 8, "sizeof CHAR_BIT != 8.");
static_assert(sizeof(BYTE)* CHAR_BIT == 8, "sizeof BYTE != 8.");

struct  PIXEL_RGBA {
  BYTE r;
  BYTE g;
  BYTE b;
  BYTE a;
};

struct  PIXEL_GREY {
  BYTE v;
  BYTE a;
};

struct  PIXEL_INDEX {
  BYTE i;
};

struct RECT {
  LONG x;
  LONG y;
  LONG w;
  LONG h;
};

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
  old_pallet  = 0x0004ui16,
  old_pallet2 = 0x0011ui16,
  layer = 0x2004ui16,
  cel = 0x2005ui16,
  mask = 0x2016ui16,
  path = 0x2017ui16,
  frame_tags = 0x2018ui16,
  pallet  = 0x2019ui16,
  user_data = 0x2020ui16,
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
  WORD x;
  WORD y;
  BYTE opacity;
  WORD cell_type;
  BYTE z0[7];
};

struct frame_cel {
  cel_header c;
  WORD w, h;
  WORD linked = 0;
  // Assume RGBA
  std::vector<PIXEL_RGBA> pixels;
};

struct Tag {
  WORD from;
  WORD to;
  BYTE loop_direction;
  std::string name;
};

struct Frame {
  WORD duration;
  std::vector<PIXEL_RGBA> pixels;
};

struct Sprite {
  WORD w,h;
  std::vector<Tag> tags;
  std::vector<Frame> frames;
};


template<typename T>
const char* read_object(const char* buf, T& target) {
  target = *reinterpret_cast<const T*>(buf);
  return buf + sizeof(T);
}

template<typename CT>
const char* read_object(const char* buf, std::vector<CT>& target) {
  size_t size = target.size();
  CT const* buf_start = reinterpret_cast<const CT*>(buf);
  std::copy(buf_start, buf_start + size, target.begin());
  return buf + size * sizeof(CT);
}

template<>
const char* read_object(const char* buf, std::string& target) {
  WORD s_len;
  buf = read_object(buf, s_len);
  target.resize(s_len);
  size_t size = target.size();
  const char* buf_start = reinterpret_cast<const char*>(buf);
  std::copy(buf_start, buf_start + size, target.begin());
  return buf + size;
}

template<>
const char* read_object(const char* buf, std::vector<Tag>& target) {
  struct tag_header {
    WORD tag_count;
    BYTE z0[8];
  } header;
  buf = read_object(buf, header);

  struct t_read_struct {
    WORD from;
    WORD to;
    BYTE loop_dir;
    BYTE z0[8];
    BYTE RGB[3];
    BYTE z1;
  } t;
  
  for ( size_t i = 0; i < header.tag_count; ++i ) {
    buf = read_object(buf, t);
    std::string s;
    buf = read_object(buf, s);
    target.emplace_back(Tag{t.from, t.to, t.loop_dir, s });
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

#pragma pack(pop)