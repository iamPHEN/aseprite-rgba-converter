// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#pragma once

#include <cstdint>
#include <vector>

#pragma pack(push)
#pragma pack(1)
namespace aseprite {
using BYTE = uint8_t;
using WORD = uint16_t;
using SIGNED_WORD = int16_t;
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
  WORD w, h;
  std::vector<Tag> tags;
  std::vector<Frame> frames;
};
} //namespace aseprite

#pragma pack(pop)
