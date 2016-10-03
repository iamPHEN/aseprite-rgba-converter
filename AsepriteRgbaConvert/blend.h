// Aseprite Convert to RGBA library
// Copyright (c) 2016 Bablawn3d5 - <stephen.ma@bablawn.com>
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "doc-lite.h"
#include "loader-detail.hpp"
#include <functional>

namespace aseprite {
namespace blend {

using aseprite::details::frame_cel;

using rgba_blend_func = std::function<PIXEL_RGBA(const PIXEL_RGBA&, const PIXEL_RGBA&, const BYTE&)>;

PIXEL_RGBA dest(const PIXEL_RGBA& bg, const PIXEL_RGBA& fg, const BYTE& opacity = 0xFF);
PIXEL_RGBA src(const PIXEL_RGBA& bg, const PIXEL_RGBA& fg, const BYTE& opacity = 0xFF);

PIXEL_RGBA normal_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA multiply_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA screen_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA overlay_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA darken_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA lighten_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA color_dodge_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA color_burn_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA hard_light_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA soft_light_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA diffrence_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA exclusion_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);

// TODO(SMA) : These are currently stubs.
PIXEL_RGBA hue_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA saturation_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA color_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);
PIXEL_RGBA luminosity_blend(const PIXEL_RGBA& lhs, const PIXEL_RGBA& rhs, const BYTE& opacity = 0xFF);

std::vector<PIXEL_RGBA> combine_blend_cels(const frame_cel& src, const frame_cel& dst,
                                   const BYTE& opacity,
                                   rgba_blend_func blend_func);

} // namespace asesprite::blend
} // namespace aseprite
