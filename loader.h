#pragma once

#include <cstdint>
#include <cassert>
#include <sstream>
#include <vector>
#include "zlib.h"
#pragma pack(push)
#pragma pack(1)

namespace aseprite {
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

namespace details {
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
}

using namespace aseprite::details;

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

/** Decompress a byte array read in. */
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

// TODO(SMA) : Move me to a blend header.
std::vector<PIXEL_RGBA> dest_blend_cels(const frame_cel& src, const frame_cel& dst) {
  std::vector<PIXEL_RGBA> pixels;
  pixels.resize(src.w * src.h);
  for ( size_t x = 0; x < src.h; ++x ) {
    for ( size_t y = 0; y < src.w; ++y ) {
      if ( x >= dst.c.x  && x < (size_t)(dst.c.x + dst.h) &&
          y >= dst.c.y  && y < (size_t)(dst.c.y + dst.w) ) {
        auto& pixel = dst.pixels.at((y - dst.c.y) + ((x - dst.c.x)*dst.w));
        pixels[y + (x*src.w)] = pixel;
      } else {
        pixels[y + (x*src.w)] = src.pixels[y + (x*src.w)];
      }
    }
  }
  return pixels;
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
  for ( auto& frame : frames ) {
    std::vector<frame_cel> cels;
    for ( auto& chunk : frame.chunks ) {
      auto char_iter = reinterpret_cast<const char*> (chunk.data.data());
      auto begin_ptr = char_iter;
      frame_cel frame_cel;
      switch ( chunk.type ) {
      case cel:
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
      case frame_tags:
        assert(s.tags.size() == 0);
        char_iter = read_object(char_iter, s.tags);
        // Why does chunk.size include its own size of its own header? Why does char_iter begin
        // at a point including this header? I don't know :(. Math is hard, but lets
        // assume this is right.
        assert(char_iter == begin_ptr + chunk.size - sizeof(chunk.type) - sizeof(chunk.size));
        break;
      case pallet:
      case layer:
      case old_pallet:
      case old_pallet2:
      case mask:
      case path:
        break;
      default:
        // Should throw a error here, we found a new header
        assert("New Chunk type is unsupported");
        break;
      }
    }

    // FIXME(SMA) : No support for layering yet.
    assert(cels.size() == 1);
    // Now that we've parsed all of the chunks we should be 
    // able to render the final cell.
    auto& cel = cels.at(0);
    // FIXME(SMA) : No support for linked layers, nor do I know how to produce one.
    assert(cel.linked == 0);

    // Create temparay frame_cel for final render
    frame_cel c;
    c.c.x = 0;
    c.c.y = 0;
    c.w = s.w;
    c.h = s.h;
    c.pixels.resize(c.w * c.h);

    // Render Final frame by blending all frames together.
    Frame f;
    f.duration = frame.header.duration;
    f.pixels.resize(s.h * s.w);
    f.pixels = dest_blend_cels(c, cel);
    s.frames.push_back(f);
  }

  return s;
}

}
#pragma pack(pop)