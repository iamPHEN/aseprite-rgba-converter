#pragma once

#include <cstdint>
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

#pragma pack(pop)

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
