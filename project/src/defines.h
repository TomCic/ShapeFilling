// Copyright (c) 2022 - 2023 Tomáš Cicvárek, CTU in Prague, FEE
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef __DEFINES
#define __DEFINES

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#include <iostream>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#define RADIUS 3 //5 // 3  // 10
#define CHANNELS 3

#define EXTENTION ".png"
#define FOLDER "pictures/"
#define MM_PROJECT "pictures/mm_project.zip"

// masks
// 1 idicates soft scribbles, 0 hard scribbles
#define MASK_SCRIBBLE_TYPE 1
#define MASK_OTHER_1 14

// 2 idicates colors to be locked, 0 unlocked
#define MASK_LOCK 2
#define MASK_OTHER_2 13

#define FILENAME_LENGHT 30

#define ARROW_COLOR_R 147.0 / 255.0
#define ARROW_COLOR_G 255.0 / 255.0
#define ARROW_COLOR_B 111.0 / 255.0

#define ARROW2_COLOR_R 255.0 / 255.0
#define ARROW2_COLOR_G 147.0 / 255.0
#define ARROW2_COLOR_B 111.0 / 255.0

#define MM_WIDTH 1000
#define MM_HEIGHT 800

#define ARROW_THICKNESS 4
#define ARROW_HEAD_SIZE 8

#define DEFAULT_COLOR \
  { 1, 1, 1 }

#define MAX_LEVEL_MOD 4
#define VARIANCE_BASE 0.5

#define ERR_CONSTANT 0.01f
#define ITERATIONS 20

#define ORIG_WHITE_ERR 0.985f

#define EXPONENT 9.0f

// modal window params
#define HUE_LOC "data/hue.png"
#define FONT_LOC "data/cour/cour.ttf"

#define MW_W 500
#define MW_H 200
#define MW_X 250
#define MW_Y 300

#define M_X 5
#define M_Y 20
#define M_W 30
#define M_H 160

#define M_SL1 32
#define M_SL2 57
#define M_SL3 82
#define M_SL4 117
#define M_SL5 142
#define M_SL6 167
#define SL_X1 130
#define SL_X2 490
#define SL_W 7

#define M_CH1 20
#define M_CH2 45
#define M_CH3 70
#define M_CH4 105
#define M_CH5 130
#define M_CH6 155
#define M_CHX 40

#define BTN_RI 2
#define BTN_RO 5
#define BTN_TH 2

#define FONT_S 20
#define FONT_X M_X + M_W + 45

typedef unsigned char BYTE;

/// <summary>
/// Structure representing vector of 2 real numbers.
/// </summary>
template <typename T>
struct vec2 {
  T x, y;

  vec2() {
    x = 0;
    y = 0;
  }
  vec2(T x, T y) {
    this->x = x;
    this->y = y;
  }
  vec2(vec2 const& v) {
    x = v.x;
    y = v.y;
  }
  vec2 operator+(vec2 const& v) const { return vec2(x + v.x, y + v.y); }
  vec2 operator-(vec2 const& v) const { return vec2(x - v.x, y - v.y); }
  vec2 operator*(float const& f) const { return vec2(x * f, y * f); }
  vec2 operator/(float const& f) const { return vec2(x / f, y / f); }
  bool operator==(const vec2& v) const { return x == v.x && y == v.y; }
  bool operator!=(const vec2& v) const { return x != v.x || y != v.y; }
  friend std::ostream& operator<<(std::ostream& os, const vec2& v) {
    return (os << v.x << " " << v.y << "\n");
  }
  vec2 operator=(const vec2 v) {
    x = v.x;
    y = v.y;
    return v;
  }

  float norm() { return std::sqrt(x * x + y * y); }
};

/// <summary>
/// Structure representing two image coordinates.
/// </summary>
struct Coordinates {
  int x1, y1, x2, y2;
};

struct HSL {
  float H, S, L;

  HSL operator=(const HSL hsl) {
    H = hsl.H;
    S = hsl.S;
    L = hsl.L;
    return hsl;
  }
  friend std::ostream& operator<<(std::ostream& os, const HSL& hsl) {
    os << hsl.H << ' ' << hsl.S << ' ' << hsl.L << '\n';
    return os;
  }
};

/// <summary>
/// Tracks values fot modal window.
/// </summary>
struct ModalState {
  float vals[6];

  ModalState operator=(ModalState& ms) {
    for (int i = 0; i < 6; i++) vals[i] = ms.vals[i];
    return ms;
  }
};

/// <summary>
/// Enum comprising of all directions
/// </summary>
enum Directions { RIGHT, DOWN, LEFT, UP, ALL, NONE };

/// <summary>
/// Enum used for mouse buttons management
/// </summary>
enum MouseButtons { REL = 0, LMB, RMB };

/// <summary>
/// Application states
/// </summary>
enum Mode { DRAW = 0, DEPTH, PICK, BLOCK };
#define ANIM 4
#define COL 8
#define INIT_INCR 0.02f

enum ModalFlags { FSL = 0, FH, FRGB };

#endif  // !__DEFINES