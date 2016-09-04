
#include "loader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cassert>
#include "zlib.h"

/** Decompress an STL string using zlib and return the original data. */
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

// Dumps RGBA array to stdout as ascii
void print_as_ascii(std::vector<PIXEL_RGBA> pixels, WORD w, WORD h) {
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
   ase_header a;
   char_iter = read_object(char_iter, a);
   print_struct((void*)&a, sizeof(a));

   std::vector<frame> frames;
   for ( size_t f = 0; f < a.frames; ++f ) {
     frame_header header;
     char_iter = read_object(char_iter, header);
     print_struct((void*)&header, sizeof(header));

     std::vector<frame_chunk> chunks;
     for ( size_t i = 0; i < header.chunks; ++i ) {
       frame_chunk c;
       char_iter = read_object(char_iter, c.size);
       char_iter = read_object(char_iter, c.type);
       c.data.resize(c.size-sizeof(c.size)-sizeof(c.type));
       char_iter = read_object(char_iter, c.data);
       print_struct((void*)&c.size, sizeof(c.size));
       print_struct((void*)&c.type, sizeof(c.type));
       print_struct((void*)c.data.data(), c.data.size());
       chunks.push_back(c);
     }
     frames.emplace_back(frame{ header, chunks });
   }
  
  // Parse chunk data.
  Sprite s;
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
         } else if( frame_cel.c.cell_type == 1 ) {
           char_iter = read_object(char_iter, frame_cel.linked);
           // Check that we've read all the data.
           assert(char_iter == begin_ptr + chunk.size);
         } else if ( frame_cel.c.cell_type == 2 ) {
           char_iter = read_object(char_iter, frame_cel.w);
           char_iter = read_object(char_iter, frame_cel.h);
           frame_cel.pixels.resize(frame_cel.w * frame_cel.h);
           std::vector<BYTE> compressed;
           compressed.resize(chunk.size - sizeof(frame_cel.c) - sizeof(WORD)*2);
           char_iter = read_object(char_iter, compressed);
           // Check that we've read all the data.
           assert(char_iter == begin_ptr+chunk.size);

           std::vector<BYTE> decompressed = decompress(compressed);
           auto pixel_iter = reinterpret_cast<const char*> (decompressed.data());
           pixel_iter = read_object(pixel_iter, frame_cel.pixels);
         } else {
           //Something horrible has gone wrong here.
           assert("Something horrible has happened");
           return 1;
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
         std::cerr << "Ignored type" << std::endl;
         break;
       default:
         std::cerr << "Unsupported type" << std::endl;

       }
    }
    
    // Now that we've parsed all of the chunks we should be 
    // able to render the final cell.
    auto& cel = cels.at(0);
    print_as_ascii(cel.pixels, cel.w, cel.h);

  }

   return 0;
}
