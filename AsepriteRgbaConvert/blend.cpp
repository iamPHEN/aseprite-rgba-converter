// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "blend.h"
#include <math.h>
#include <algorithm>

namespace aseprite {
namespace blend {

using aseprite::details::frame_cel;

PIXEL_RGBA dest(const PIXEL_RGBA& bg, const PIXEL_RGBA& fg, const BYTE& opacity) { return fg; };
PIXEL_RGBA src(const PIXEL_RGBA& bg, const PIXEL_RGBA& fg, const BYTE& opacity) { return bg; };

PIXEL_RGBA normal_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity) {

  // Bitwise multiplication applicator. Makes sure if we overflow we cap out at 
  // BYTE maximum 255, or undeflow to byte minumium of 0.
  // Intentionally using integer math because decmial pixels are dumb.
  const auto& apply = [](const BYTE& p, const float& f) -> const BYTE{
    // Assuming int is big enough to hold byte * float.
    return std::max(std::min(int(p * f), 255), 0);
  };

  if ( lhs.a == 0 ) {
    // Set opacity
    const PIXEL_RGBA ret{ rhs.r, rhs.g, rhs.b, apply(rhs.a, opacity / 255.f) };
    return ret;
  } else if ( rhs.a == 0 ) {
    return lhs;
  } else if ( rhs.a == 0 && lhs.a == 0 ) {
    return PIXEL_RGBA{ 0,0,0,0 };
  }

  // Calculate output alpha / 2
  const int dem = 0xFF - apply((lhs.a), opacity / 255.f);
  const int alpha = int(lhs.a + apply(rhs.a, dem));
  // Calculate alpha blending
  const PIXEL_RGBA r{
    static_cast<BYTE>(int(lhs.r + apply(rhs.r,dem) / alpha)),
    static_cast<BYTE>(int(lhs.g + apply(rhs.g,dem) / alpha)),
    static_cast<BYTE>(int(lhs.b + apply(rhs.b,dem) / alpha)),
    static_cast<BYTE>(alpha) };
  return r;
};

PIXEL_RGBA multiply_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity) {
  const PIXEL_RGBA mul_rgba {
    static_cast<BYTE>(rhs.r * lhs.r),
    static_cast<BYTE>(rhs.g * lhs.g),
    static_cast<BYTE>(rhs.b * lhs.b),
    static_cast<BYTE>(rhs.a * lhs.a) };
  return normal_blend(lhs, mul_rgba, opacity);
};

PIXEL_RGBA screen_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity) {
  const PIXEL_RGBA screen_rgba{
    static_cast<BYTE>(rhs.r + lhs.r - rhs.r * lhs.r),
    static_cast<BYTE>(rhs.g + lhs.g - rhs.g * lhs.g),
    static_cast<BYTE>(rhs.b + lhs.b - rhs.b * lhs.b),
    static_cast<BYTE>(rhs.a + lhs.a - rhs.a * lhs.a) };
  return normal_blend(lhs, screen_rgba, opacity);
};

PIXEL_RGBA overlay_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity) {
  const PIXEL_RGBA screen_rgba = screen_blend(lhs, rhs, opacity);
  const PIXEL_RGBA multiply_rgba = multiply_blend(lhs, rhs, opacity);
  const PIXEL_RGBA overlay_rbga {
    static_cast<BYTE>( (rhs.r > 0x80) ? screen_rgba.r : multiply_rgba.r ),
    static_cast<BYTE>( (rhs.g > 0x80) ? screen_rgba.g : multiply_rgba.g ),
    static_cast<BYTE>( (rhs.b > 0x80) ? screen_rgba.b : multiply_rgba.b ),
    static_cast<BYTE>( (rhs.a > 0x80) ? screen_rgba.a : multiply_rgba.a ) };
  return normal_blend(lhs, overlay_rbga, opacity);
};

PIXEL_RGBA darken_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity) {
  const PIXEL_RGBA darken_rgba{
    static_cast<BYTE>(std::min(lhs.r, rhs.r)),
    static_cast<BYTE>(std::min(lhs.g, rhs.g)),
    static_cast<BYTE>(std::min(lhs.b, rhs.b)),
    static_cast<BYTE>(rhs.a) };
  return normal_blend(lhs, darken_rgba, opacity);
};

PIXEL_RGBA lighten_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity) {
  const PIXEL_RGBA lighten_rgba{
    static_cast<BYTE>(std::max(lhs.r, rhs.r)),
    static_cast<BYTE>(std::max(lhs.g, rhs.g)),
    static_cast<BYTE>(std::max(lhs.b, rhs.b)),
    static_cast<BYTE>(rhs.a) };
  return normal_blend(lhs, lighten_rgba, opacity);
}

PIXEL_RGBA color_dodge_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  const auto& dodge = [](const BYTE& bg,const BYTE& fg) -> const BYTE {
    if ( bg == 0 )
      return 0;
    auto inv = 0xFF - fg;
    if ( bg >= inv )
      return 0xFF;
    else
      return  fg/inv;
  };
  const PIXEL_RGBA dodge_rgba {
    dodge(lhs.r, rhs.r),
    dodge(lhs.g, rhs.g),
    dodge(lhs.b, rhs.b),
    rhs.a
  };
  return normal_blend(lhs, dodge_rgba, opacity);
}

PIXEL_RGBA color_burn_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  const auto& burn = [](const BYTE& bg, const BYTE& fg) -> const BYTE {
    if ( bg == 0xFF )
      return 0xFF;
    auto inv = 0xFF - bg;
    if ( inv >= fg )
      return 0;
    else
      return inv/fg;
  };
  const PIXEL_RGBA burn_rgba{
    burn(lhs.r, rhs.r),
    burn(lhs.g, rhs.g),
    burn(lhs.b, rhs.b),
    rhs.a
  };
  return normal_blend(lhs, burn_rgba, opacity);
}
PIXEL_RGBA hard_light_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  return overlay_blend(lhs, rhs, opacity);
}
PIXEL_RGBA soft_light_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  const auto& soft = [](const BYTE& bg, const BYTE& fg) -> const BYTE {
    const double b = bg / 255.0;
    const double s = fg / 255.0;
    const double d = ( b <= 0.25 ) ?
      ((16 * b - 12)*b + 4)*b :
      std::sqrt(b);

    const double r = ( s <= 0.5 ) ?
      b - (1.0 - 2.0*s) * b * (1.0 - b) :
      b - (1.0 - 2.0*s) * b * (1.0 - b);

    return (BYTE)(r * 255 + 0.5);
  };
  const PIXEL_RGBA burn_rgba{
    soft(lhs.r, rhs.r),
    soft(lhs.g, rhs.g),
    soft(lhs.b, rhs.b),
    rhs.a
  };
  return normal_blend(lhs, burn_rgba, opacity);
}
PIXEL_RGBA diffrence_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  const PIXEL_RGBA diff_rgba{
    static_cast<BYTE>(std::abs(lhs.r - rhs.r)),
    static_cast<BYTE>(std::abs(lhs.g - rhs.g)),
    static_cast<BYTE>(std::abs(lhs.b - rhs.b)),
    static_cast<BYTE>(rhs.a) };
  return normal_blend(lhs, diff_rgba, opacity);
}
PIXEL_RGBA exclusion_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  const auto& exclusion = [](const BYTE& bg, const BYTE& fg) -> const BYTE {
    WORD mul = bg * fg;
    return (BYTE)(bg + fg - 2 * mul);
  };
  const PIXEL_RGBA exclusion_rgba{
    exclusion(lhs.r, rhs.r),
    exclusion(lhs.g, rhs.g),
    exclusion(lhs.b, rhs.b),
    rhs.a
  };
  return normal_blend(lhs, exclusion_rgba, opacity);
}

// TODO(SMA) : Implement HSL blenders
PIXEL_RGBA hue_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  return normal_blend(lhs, rhs, opacity);
}
PIXEL_RGBA saturation_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  return normal_blend(lhs, rhs, opacity);
}
PIXEL_RGBA color_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  return normal_blend(lhs, rhs, opacity);
}
PIXEL_RGBA luminosity_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity){
  return normal_blend(lhs, rhs, opacity);
}

std::vector<PIXEL_RGBA> combine_blend_cels(const frame_cel& src, const frame_cel& dst,
                                   const BYTE& opacity,
                                   rgba_blend_func blend_func) {
  // FIXME(SMA) : Uh.. this might be a little memory inefficent, but it works.
  std::vector<PIXEL_RGBA> pixels;
  pixels.resize(src.w * src.h);
  for ( size_t y = 0; y < src.h; ++y ) {
    for ( size_t x = 0; x < src.w; ++x ) {
      // If we're within the offsets of dst frame, paint dst instead
      if ( x - dst.c.x < dst.w && y - dst.c.y < dst.h &&
          x - dst.c.x >= 0 && y - dst.c.y >= 0 ) {
        const auto& pixel = dst.pixels[(x - dst.c.x) + ((y - dst.c.y)*dst.w)];
        const auto& src_px = src.pixels[x + (y*src.w)];
        pixels[x + (y*src.w)] = blend_func(src_px, pixel, opacity);
      } else {
        pixels[x + (y*src.w)] = src.pixels[x + (y*src.w)];
      }
    }
  }
  return pixels;
}
} // namespace asesprite::blend
} // namespace aseprite
