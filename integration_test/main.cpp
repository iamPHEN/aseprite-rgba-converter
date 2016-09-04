// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#include "loader.h"
#include <iostream>
#include <sstream>
#include <fstream>
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
          pixels.at(y + (x*w)).b != 0 ) {
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
  std::ifstream file;
  file.open("Sprite-0002.ase", std::ios::in | std::ios::binary);

  std::streampos fileSize;
  if ( !file ) {
    std::cerr << "Not able to read" << std::endl;
    return 1;
  } else {
    file.seekg(0, std::ios_base::end);
    fileSize = file.tellg();
    if ( fileSize <= 128 ) {
      std::cerr << "File is malformed .ase format." << std::endl;
      return 1;
    }
  }

  std::vector<char> vec;
  // HACK(SMA): Just load the entire file into memory rather than streaming it.
  // We're assuming .ase files won't become absurdly large.
  if ( !file.eof() && !file.fail() ) {
    vec.resize((size_t)fileSize);
    file.seekg(0, std::ios_base::beg);
    file.read(&vec[0], fileSize);
  }

  auto char_iter = reinterpret_cast<const char*> (vec.data());
  aseprite::Sprite s = aseprite::load_sprite(char_iter);

  for ( auto& f : s.frames ) {
     print_as_ascii(f.pixels, s.w, s.h);
  }
  return 0;
}
