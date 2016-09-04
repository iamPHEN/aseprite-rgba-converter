// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#pragma once
#include <cassert>
#include <vector>
#include <string>
#include "doc-lite.h"

#pragma pack(push)
#pragma pack(1)
namespace aseprite {
// Decompress a byte array read in.
std::vector<BYTE> decompress(const std::vector<BYTE>& str);

// Loads sprite from byte array
Sprite load_sprite(const char* char_iter);

// Loads sprite from file.
Sprite load_sprite_from_file(const char* filename);

} // namespace aseprite
#pragma pack(pop)