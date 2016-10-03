/// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "loader-detail.hpp"

namespace aseprite {
namespace details {
template<typename T>
const char * read_object(const char * buf, T & target) {
  target = *reinterpret_cast<const T*>(buf);
  return buf + sizeof(T);
}

template
const char* read_object(const char* buf, ase_header& target);

template
const char* read_object(const char* buf, frame_header& target);

template
const char* read_object(const char* buf, cel_header& target);

template
const char* read_object(const char* buf, PIXEL_RGBA& target);

template
const char* read_object(const char* buf, layer_header& target);

template
const char* read_object(const char* buf, unsigned int& target);

template
const char* read_object(const char* buf, unsigned char& target);

template<typename CT>
const char* read_object(const char* buf, std::vector<CT>& target) {
  size_t size = target.size();
  CT const* buf_start = reinterpret_cast<const CT*>(buf);
  std::copy(buf_start, buf_start + size, target.begin());
  return buf + size * sizeof(CT);
}

template
const char* read_object(const char* buf, std::vector<unsigned char>& target);

template
const char* read_object(const char* buf, std::vector<PIXEL_RGBA>& target);


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
#pragma pack(push)
#pragma pack(1)
  // HACK(SMA) : This class is never used, just define a inline class to discard
  // bytes.
  struct tag_header {
    WORD tag_count;
    BYTE z0[8];
  } header;
  static_assert(sizeof(header) == 10, "Sizeof tag_header doesn't match header size of 10 bytes.");
  buf = read_object(buf, header);

  // HACK(SMA) : Define some inline tag here that will be converted to 
  // a doc-lite Tag.
  struct t_read_struct {
    WORD from;
    WORD to;
    BYTE loop_dir;
    BYTE z0[8];
    BYTE RGB[3];
    BYTE z1;
  } t;
  static_assert(sizeof(t) == 17, "Sizeof t_read_struct doesn't match header size of 17 bytes.");

#pragma pack(pop)

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

} // namespace aseprite::details
} // namespace aseprite
