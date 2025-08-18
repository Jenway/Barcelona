// core/constants.hpp
#pragma once
namespace RTConstants {
constexpr double EPSILON_RAY_HIT = 0.001; // For ray start offset and t_min
constexpr double EPSILON_SHADOW_OFFSET = 1e-4; // For shadow ray origin offset
constexpr double EPSILON_GEOMETRY = 1e-6; // For comparisons against small values in geometry
constexpr double EPSILON_DENOM = 1e-12; // For denominators near zero
}