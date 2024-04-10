#pragma once

#include "BgrColor.hpp"
#include <array>

static constexpr auto NumberOfLeds = 6;

/// array which should be filled by user/animation class with data
using LedSegmentArray = std::array<BgrColor, NumberOfLeds>;