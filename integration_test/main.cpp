// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#include "AsepriteRgbaConvert/loader.h"
#include <iostream>
#include <vector>
#include <cassert>

using aseprite::PIXEL_RGBA;
using aseprite::WORD;

// Dumps RGBA array to stdout as ascii
void print_as_ascii(const std::vector<PIXEL_RGBA>& pixels, WORD w, WORD h) {
  std::cout << std::endl;
  for ( size_t x = 0; x < h; ++x ) {
    for ( size_t y = 0; y < w; ++y ) {
      if ( pixels.at(y + (x*w)).r != 0 ||
          pixels.at(y + (x*w)).g != 0 ||
          pixels.at(y + (x*w)).b != 0 ||
          pixels.at(y + (x*w)).a != 0 ) {
        std::cout << "o";
      } else {
        std::cout << ".";
      }
    }
    std::cout << std::endl;
  }
}


void print_struct(void *s, size_t structSize) {
  size_t i;
  unsigned char *printme = (unsigned char *)s;
  int formatter = 1;
  for ( i = 0; i < structSize; i++ ) {
    printf("%02x", *printme++);
    if ( (i+1) % 2 == 0 ) {
      printf(" ");
    }
  }
}

int main(int argc, char* const argv[]) {
  aseprite::Sprite s1 = aseprite::load_sprite_from_file("Sprite-0001.ase");
  aseprite::Sprite s2 = aseprite::load_sprite_from_file("Sprite-0002.ase");
  aseprite::Sprite s3 = aseprite::load_sprite_from_file("Sprite-0003.ase");
  aseprite::Sprite s4 = aseprite::load_sprite_from_file("Sprite-0005-Layers.ase");

  assert(s1.frames.size() == 4);
  assert(s1.w == 32);
  assert(s1.h == 32);
  assert(s2.frames.size() == 5);
  assert(s2.tags.size() == 2);
  assert(s3.w == 320);
  assert(s3.h == 320);
  assert(s4.frames.size() == 2);
  assert(s4.tags.size() == 0);
  assert(s4.w == 64);
  assert(s4.h == 75);

  for ( auto& f : s1.frames ) {
     print_as_ascii(f.pixels, s1.w, s1.h);
  }

  for ( auto& f : s4.frames ) {
    print_as_ascii(f.pixels, s4.w, s4.h);
  }

  //aseprite::Sprite s4 = aseprite::load_sprite_from_file("Sprite-0004.ase");
  //assert(s4.w == 3200);
  //assert(s4.h == 3200);
  return 0;
}
