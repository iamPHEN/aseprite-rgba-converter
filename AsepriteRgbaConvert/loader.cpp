// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#include "loader.h"
#include "loader-detail.hpp"
#include "blend.h"
#include "zlib.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <algorithm>

namespace aseprite {
using namespace aseprite::details;
using aseprite::blend::combine_blend_cels;

// zlib decompress ase byte buffer
std::vector<BYTE> decompress(const std::vector<BYTE>& str) {
  z_stream zs;
  zs.zalloc = (alloc_func)0;
  zs.zfree = (free_func)0;
  zs.opaque = (voidpf)0;

  if ( inflateInit(&zs) != Z_OK )
    throw(std::runtime_error("inflateInit failed while decompressing."));

  zs.next_in = (Bytef*)&str[0];
  zs.avail_in = str.size();

  int ret;
  char outbuffer[32768];
  std::vector<BYTE> outstring;

  size_t uncompressed_offset = 0;
  // get the decompressed bytes blockwise using repeated calls to inflate
  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = inflate(&zs, Z_NO_FLUSH);

    size_t uncompressed_bytes = 32768 - zs.avail_out;
    if ( outstring.size() < zs.total_out ) {
      outstring.resize(zs.total_out);
    }
    if ( uncompressed_bytes > 0 ) {
      std::copy(outbuffer, outbuffer + uncompressed_bytes,
                outstring.begin() + uncompressed_offset);
    }

    uncompressed_offset += uncompressed_bytes;

  } while ( ret == Z_OK );

  inflateEnd(&zs);

  if ( ret != Z_STREAM_END ) {          // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib decompression: (" << ret << ") "
      << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  return outstring;
}

Sprite load_sprite(const char* char_iter) {
  ase_header a;
  char_iter = read_object(char_iter, a);
#ifdef DEBUG_VERBOSE
  print_struct((void*)&a, sizeof(a));
#endif

  std::vector<frame> frames;
  for ( size_t f = 0; f < a.frames; ++f ) {
    frame_header header;
    char_iter = read_object(char_iter, header);
#ifdef DEBUG_VERBOSE
    print_struct((void*)&header, sizeof(header));
#endif

    std::vector<frame_chunk> chunks;
    for ( size_t i = 0; i < header.chunks; ++i ) {
      frame_chunk c;
      char_iter = read_object(char_iter, c.size);
      char_iter = read_object(char_iter, c.type);
      c.data.resize(c.size - sizeof(c.size) - sizeof(c.type));
      char_iter = read_object(char_iter, c.data);
#ifdef DEBUG_VERBOSE
      print_struct((void*)&c.size, sizeof(c.size));
      print_struct((void*)&c.type, sizeof(c.type));
      print_struct((void*)c.data.data(), c.data.size());
#endif
      chunks.push_back(c);
    }
    frames.emplace_back(frame{ header, chunks });
  }

  // Parse chunk data.
  Sprite s;
  s.w = a.width;
  s.h = a.height;
  std::vector<Layer> layers;
  for ( auto& frame : frames ) {
    std::vector<frame_cel> cels;
    for ( auto& chunk : frame.chunks ) {
      auto char_iter = reinterpret_cast<const char*> (chunk.data.data());
      auto begin_ptr = char_iter;
      frame_cel frame_cel;
      Layer layer;
      switch ( static_cast<chunk_type>(chunk.type) ) {
      case chunk_type::cel:
        char_iter = read_object(char_iter, frame_cel.c);
        assert(a.depth == 32);
        if ( frame_cel.c.cell_type == 0 ) {
          char_iter = read_object(char_iter, frame_cel.w);
          char_iter = read_object(char_iter, frame_cel.h);
          frame_cel.pixels.resize(frame_cel.w * frame_cel.h);
          char_iter = read_object(char_iter, frame_cel.pixels);
          // Check that we've read all the data.
          assert(char_iter == begin_ptr + chunk.size);
        } else if ( frame_cel.c.cell_type == 1 ) {
          char_iter = read_object(char_iter, frame_cel.linked);
          // Check that we've read all the data.
          assert(char_iter == begin_ptr + chunk.size);
        } else if ( frame_cel.c.cell_type == 2 ) {
          char_iter = read_object(char_iter, frame_cel.w);
          char_iter = read_object(char_iter, frame_cel.h);
          frame_cel.pixels.resize(frame_cel.w * frame_cel.h);
          std::vector<BYTE> compressed;
          compressed.resize(chunk.size - sizeof(frame_cel.c) - sizeof(WORD) * 2);
          char_iter = read_object(char_iter, compressed);
          // Check that we've read all the data.
          assert(char_iter == begin_ptr + chunk.size);

          std::vector<BYTE> decompressed = decompress(compressed);
          auto pixel_iter = reinterpret_cast<const char*> (decompressed.data());
          pixel_iter = read_object(pixel_iter, frame_cel.pixels);
        } else {
          //Something horrible has gone wrong here.
          assert("Something horrible has happened");
        }
        cels.push_back(frame_cel);
        break;
      case chunk_type::frame_tags:
        assert(s.tags.size() == 0);
        char_iter = read_object(char_iter, s.tags);
        // Why does chunk.size include its own size of its own header? Why does char_iter begin
        // at a point including this header? I don't know :(. Math is hard, but lets
        // assume this is right.
        assert(char_iter == begin_ptr + chunk.size - sizeof(chunk.type) - sizeof(chunk.size));
        break;
      case chunk_type::layer:
         char_iter = read_object(char_iter, layer.header);
         char_iter = read_object(char_iter, layer.name);
         layers.push_back(layer);
         break;
      case chunk_type::pallet:
      case chunk_type::old_pallet:
      case chunk_type::old_pallet2:
      case chunk_type::mask:
      case chunk_type::path:
        break;
      default:
        // Should throw a error here, we found a new header
        assert("New Chunk type is unsupported");
        break;
      }
    }

    if ( cels.size() > 0 ) {
      assert(frame.header.duration >= 1);
      // Now that we've parsed all of the chunks we should be 
      // able to render the final cell.

      // Create temparay frame_cel for final render
      frame_cel c;
      c.c.x = 0;
      c.c.y = 0;
      c.w = s.w;
      c.h = s.h;
      c.pixels.resize(c.w * c.h);

      Frame f;
      f.duration = frame.header.duration;
      f.pixels.resize(s.h * s.w);

      // HACK(SMA) : assuming that these are still in reverse order.
      for ( auto& it = cels.rbegin(); it != cels.rend(); ++it ) {
        const auto& cel = *it;
        // FIXME(SMA) : No support for linked layers yet.
        assert(cel.linked == 0);
        // If index is non-zero.
        if ( layers.size() > 0 ) {
          assert(cel.c.layer_index < layers.size());
          const auto& blend = layers[cel.c.layer_index];
          const auto& blend_func = [&blend]() -> aseprite::blend::rgba_blend_func {
            switch( blend.header.blend_mode ) {
              case 0: return aseprite::blend::normal_blend;
              case 1: return aseprite::blend::multiply_blend;
              case 2: return aseprite::blend::screen_blend;
              case 3: return aseprite::blend::overlay_blend;
              case 4: return aseprite::blend::darken_blend;
              case 5: return aseprite::blend::lighten_blend;
              case 6: return aseprite::blend::color_dodge_blend;
              case 7: return aseprite::blend::color_burn_blend;
              case 8: return aseprite::blend::hard_light_blend;
              case 9: return aseprite::blend::soft_light_blend;
              case 10: return aseprite::blend::diffrence_blend;
              case 11: return aseprite::blend::exclusion_blend;
              // FIXME(SMA) : HSL blends don't actually work.
              //case 12: return aseprite::blend::hue_blend;
              //case 13: return aseprite::blend::saturation_blend;
              //case 14: return aseprite::blend::color_blend;
              //case 15: return aseprite::blend::luminosity_blend;
            }
            assert(false && "Unkown blending func passed in");
            return aseprite::blend::normal_blend;
          }();

          c.pixels = combine_blend_cels(c, cel, blend.header.opacity, blend_func);
        } else {
          // Render, Final frame by blending all frames together.
          c.pixels = combine_blend_cels(c, cel, 0xFF, aseprite::blend::dest);
        }
      }

      // FIXME(SMA) : Do a copy to pixels
      f.pixels = c.pixels;
      s.frames.push_back(f);
    } else {
      Frame f;
      f.duration = frame.header.duration;
      // TODO(SMA) : Save space by allowing 0 pixels in frame using 
      // f.link when its implemneted.
      f.pixels.resize(s.h * s.w);
      s.frames.push_back(f);
    }
  }

  return s;
}

Sprite load_sprite_from_file(const char * filename) {
  std::ifstream file;
  file.open(filename, std::ios::in | std::ios::binary);

  std::streampos fileSize;
  if ( !file ) {
    std::cerr << "Not able to read" << std::endl;
    throw(std::runtime_error("Not able to read file"));
    return Sprite();
  } else {
    file.seekg(0, std::ios_base::end);
    fileSize = file.tellg();
    if ( fileSize <= 128 ) {
      std::cerr << "File is malformed .ase format." << std::endl;
      throw(std::runtime_error("File is malformed .ase format.."));
      return Sprite();
    }
  }

  // HACK(SMA): Just load the entire file into memory rather than streaming it.
  // We're assuming .ase files won't become absurdly large.
  std::vector<char> vec;
  if ( !file.eof() && !file.fail() ) {
    vec.resize((size_t)fileSize);
    file.seekg(0, std::ios_base::beg);
    file.read(&vec[0], fileSize);
  }

  auto char_iter = reinterpret_cast<const char*> (vec.data());
  return load_sprite(char_iter);
}

} // namespace aseprite
