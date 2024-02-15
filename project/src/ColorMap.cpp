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

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG
#include "ColorMap.h"

ColorMap::ColorMap(const int width, const int height) {
  _data = new short[(size_t)width * (size_t)height];
  _scribbleCount[0] = 1;
  _scribbleCount[1] = 0;
  _width = width;
  _height = height;
  _active = 0;
  _colors.resize(256);
  _colors[0] = RGB(DEFAULT_COLOR);
  for (int i = 0; i < _width * _height; i++) _data[i] = -1;
}

ColorMap::~ColorMap() {
  if (_data) delete[] _data;
  _data = nullptr;
}

short ColorMap::getMaskAt(const int x, const int y) const {
  if (x < 0 || x >= _width || y < 0 || y >= _height) return 0;
  return _data[y * _width + x];
}

short ColorMap::getMaskAt(const vec2<int>& v) const {
  return getMaskAt(v.x, v.y);
}

RGB ColorMap::getColorAt(const int x, const int y) const {
  if (x < 0 || x >= _width || y < 0 || y >= _height) return RGB(DEFAULT_COLOR);
  return _data[y * _width + x] == -1 ? RGB(DEFAULT_COLOR)
                                     : _colors[_data[y * _width + x]];
}

RGB ColorMap::getColorAt(const int idx) const {
  if (idx < 0 || idx >= _width * _height) return RGB(DEFAULT_COLOR);
  return _data[idx] == -1 ? RGB(DEFAULT_COLOR) : _colors[_data[idx]];
}

RGB ColorMap::getColorAt(const vec2<int> pos) const {
  return getColorAt(pos.x, pos.y);
}

const std::vector<RGB>& ColorMap::getColors() const { return _colors; }

bool ColorMap::newSegment(const RGB& color, const unsigned char flags) {
  int flag = flags & MASK_SCRIBBLE_TYPE;
  if (_scribbleCount[flag] >= COLOR_MAX_COUNT)
    return false;  // return false if capacity is reached

  _active = COLOR_MAX_COUNT * flag +
            _scribbleCount[flag];  // set the index of the new segment as the
                                   // active one
  _colors[_active] = color;        // set its color
  _scribbleCount[flag] += 1;       // increase counter

  return true;
}

void ColorMap::segment2Data(const int x, const int y) {
  if (x < 0 || x >= _width || y < 0 || y >= _height) return;
  _data[y * _width + x] = _active;
}

short* ColorMap::data() { return _data; }

void ColorMap::reset() {
  for (int i = 0; i < _width * _height; i++) _data[i] = -1;
  std::fill(_colors.begin() + 1, _colors.end(), RGB(0, 0, 0));
  _scribbleCount[0] = 1;
  _scribbleCount[1] = 0;
  _active = 0;
}

void ColorMap::setColors(int i, RGB color) { _colors[i] = color; }

void ColorMap::newComputation() {
  for (int i = 0; i < _width * _height; i++) _data[i] = -1;
}

void ColorMap::printSegments(int param) const {
  std::string s = "";
  if (param > -1) {
    s = std::to_string(param);
  }
  for (int sc = 0; sc < 2; sc++) {
    for (int i = 0; i < _scribbleCount[sc]; i++) {
      int idx = i + 128 * sc;
      Image<RGB> img(_width, _height);
      for (int h = 0; h < _height; h++) {
        for (int w = 0; w < _width; w++) {
          if (getMaskAt(w, h) == idx) img(w, h) = getColorAt(w, h);
        }
      }
      imwrite(img, "pictures/segment" + s + std::to_string(idx) + ".png");
    }
  }
  Image<RGB> img(_width, _height);
  for (int h = 0; h < _height; h++) {
    for (int w = 0; w < _width; w++) {
      img(w, h) = getColorAt(w, h);
    }
  }
  imwrite(img, "pictures/segments" + s + ".png");
}

void ColorMap::consolidate(std::set<BYTE>& found, std::map<BYTE, BYTE>& changes,
                           BYTE t) {
  _scribbleCount[t] = found.size();
  // indices from 0-127 and 128 - 255 divide scribbles to categories of hard
  // and soft scribbles
  BYTE from = t * 128;
  BYTE to = found.size() + (size_t)t * (size_t)128;
  for (int i = from; i < to; i++) {
    BYTE sc = *found.begin();
    // if the first scribble equals index, then the scribble is placed correctly
    // and we can continue to another
    if (i == sc)
      found.erase(sc);
    else {
      // error in the index assingment, move the last scribble to current
      // position
      std::set<BYTE>::iterator it = found.end();
      it--;
      BYTE sc2 = *it;
      found.erase(sc2);
      _colors[i] = _colors[sc2];
      changes.insert(std::pair<BYTE, BYTE>(sc2, i));
    }
  }
  // reset the data, as they are no longer valid
  for (int i = 0; i < _width * _height; i++) _data[i] = -1;
  // set active scribble to the last one of the previous type
  if (_active >= t * 128 && (int)_active < (int)(t + 1) * 128)
    _active = _scribbleCount[t] - 1;
}

const int* ColorMap::getScribbleCount() const { return _scribbleCount; }

void ColorMap::setScribbleCount(int* count) {
  _scribbleCount[0] = count[0];
  _scribbleCount[1] = count[1];
}

void ColorMap::setActive(const char idx) { _active = idx; }

const unsigned char ColorMap::getActive() const { return _active; }

const int ColorMap::getHeight() const { return _height; }

const int ColorMap::getWidth() const { return _width; }
