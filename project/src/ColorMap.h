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

#ifndef __COLOR_MAP
#define __COLOR_MAP

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "Image.h"
#include "defines.h"

#define COLOR_MAX_COUNT 128

/// <summary>
/// Class used for mapping of the colors according to the user's strokes.
/// </summary>
class ColorMap {
 public:
  ColorMap(int width, int height);

  ~ColorMap();

  /// <summary>
  /// Retireves the data pointer
  /// </summary>
  /// <returns>Color mask map</returns>
  short* data();

  /// <summary>
  /// Gets mask index on specific location
  /// </summary>
  /// <param name="x">X coordinate</param>
  /// <param name="y">Y coordinate</param>
  /// <returns> color index at position</returns>
  short getMaskAt(const int x, const int y) const;

  /// <summary>
  /// Gets mask index on specific location
  /// </summary>
  /// <param name="v">coordinate</param>
  /// <returns> color index at position</returns>
  short getMaskAt(const vec2<int>& v) const;

  /// <summary>
  /// Gets color at specific location
  /// </summary>
  /// <param name="x">X coordinate</param>
  /// <param name="y">Y coordinate</param>
  /// <returns> color at position</returns>
  RGB getColorAt(const int x, const int y) const;

  /// <summary>
  /// Gets color at specific index
  /// </summary>
  /// <param name="idx">Index in the data array</param>
  /// <returns> color at position</returns>
  RGB getColorAt(const int idx) const;

  /// <summary>
  /// Gets color at specific location
  /// </summary>
  /// <param name="pos">coordinate</param>
  /// <returns> color at position</returns>
  RGB getColorAt(const vec2<int> pos) const;

  /// <summary>
  /// Retrieves the color palette
  /// </summary>
  /// <returns>Colors of indices</returns>
  const std::vector<RGB>& getColors() const;

  /// <summary>
  /// Creates a new segment if possible. Active mask in the one created.
  /// </summary>
  /// <param name="color">Selected color</param>
  /// <param name="flags">Scribble parameters</param>
  /// <returns>True if segment was created. False otherwise.</returns>
  bool newSegment(const RGB& color, const unsigned char flags);

  /// <summary>
  /// Index map at position is set to active mask.
  /// </summary>
  /// <param name="x">X coordinate</param>
  /// <param name="y">Y coordinate</param>
  void segment2Data(const int x, const int y);

  /// <summary>
  /// Resets all data.
  /// </summary>
  void reset();

  /// <summary>
  /// Sets color to index. Decaprated
  /// </summary>
  /// <param name="i">Color index</param>
  /// <param name="color">Selected color color</param>
  void setColors(int i, RGB color);

  /// <summary>
  /// Printing function. Prints segments separately and as a whole.
  /// </summary>
  /// <param name="param"></param>
  void printSegments(int param = -1) const;

  /// <summary>
  /// Resets the map of indices
  /// </summary>
  void newComputation();

  /// <summary>
  /// Consolidation function to shrink used ids.
  /// </summary>
  /// <param name="found">Found ids</param>
  /// <param name="changes">Switched ids</param>
  /// <param name="t">Scribble type type</param>
  void consolidate(std::set<BYTE>& found, std::map<BYTE, BYTE>& changes,
                   BYTE t);

  /// <summary>
  /// Scribble count getter
  /// </summary>
  /// <returns>Vector of two numbers indicating hard and soft scribble
  /// count</returns>
  const int* getScribbleCount() const;

  /// <summary>
  /// Sets scribble counters of hard and soft scribbles
  /// </summary>
  /// <param name="count">New count vector of two numbers</param>
  void setScribbleCount(int* count);

  /// <summary>
  /// Setter of active segment
  /// </summary>
  /// <param name="idx">Active segment index</param>
  void setActive(const char idx);

  /// <summary>
  /// Getter of active segment
  /// </summary>
  /// <returns>Index of active segment</returns>
  const unsigned char getActive() const;

  /// <summary>
  /// Height getter
  /// </summary>
  /// <returns>Height</returns>
  const int getHeight() const;

  /// <summary>
  /// Width getter.
  /// </summary>
  /// <returns>Width</returns>
  const int getWidth() const;

 private:
  short* _data;              /// mask containing color indices
  std::vector<RGB> _colors;  /// colors referenced to indices
  int _scribbleCount[2];     /// hard scribble colors start at 0 and end at 127,
                          /// soft scribble colors start at 128 and end at 255 ;
                          /// background is expected at 0
  int _width;
  int _height;
  unsigned char _active;  /// active color index
};

#endif
