/**
 * @file app/Types.h
 * @author Tomas Polasek
 * @brief Types and base includes.
 */

#ifndef SIMPLE_GAME_TYPES_H
#define SIMPLE_GAME_TYPES_H

#include <iostream>
#include <chrono>
#include <functional>
#include <map>
#include <vector>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <utility>
#include <cstring>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Types.h"

#include "testing/Testing.h"
#include "ECS.h"

static constexpr std::size_t SHORT_FRAC_BIAS{14};
using ShortFrac = float;
static constexpr std::size_t FIXED_BIAS{16};
using Fixed = float;
using FWord = i16;
using UFWord = u16;
static constexpr std::size_t F2DOT14_BIAS{14};
using F2Dot14 = float;

/// How many character should be mapped.
static constexpr std::size_t CHAR_MAPPING{256u};

static constexpr float PI{3.141f};
static constexpr float PI_4{PI / 4.0f};
static constexpr float PI_8{PI / 8.0f};

#endif //ECS_FIT_TYPES_H
