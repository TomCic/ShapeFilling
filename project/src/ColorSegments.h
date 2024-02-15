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

#ifndef __COLOR_SEGMENTS
#define __COLOR_SEGMENTS

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <string>

#include "ColorMap.h"
#include "GridCut/include/GridCut/GridGraph_2D_4C.h"
#include "GridCut/include/Image.h"
#include "defines.h"

#define K_PARAM 4000.0f
#define SOFT_PARAMETER 16.0f  // 20.0f
#define K(soft) (short)(K_PARAM * (1 - soft) + K_PARAM / SOFT_PARAMETER * soft)

#define WEIGHT(A, B) (short)(1 + K_PARAM * std::pow(std::min(A, B), 2.0f))

#define WHITE RGB(1, 1, 1)
#define BLACK RGB(0, 0, 0)

namespace ColorSegments {

struct Coords {
  int x;
  int y;

  /// <summary>
  /// Basic constructor
  /// </summary>
  Coords() {
    x = 0;
    y = 0;
  }

  /// <summary>
  /// Constructor from values
  /// </summary>
  /// <param name="x"></param>
  /// <param name="y"></param>
  Coords(const int x, const int y) {
    this->x = x;
    this->y = y;
  }

  /// <summary>
  /// Copy constructor
  /// </summary>
  /// <param name="other"></param>
  Coords(const Coords& other) {
    x = other.x;
    y = other.y;
  }

  /// <summary>
  /// Assignemnt operator
  /// </summary>
  /// <param name="other"></param>
  /// <returns></returns>
  Coords& operator=(const Coords& other) {
    x = other.x;
    y = other.y;
    return *this;
  }
};

RGB foreground = RGB(1,0,0); // default red color

/// <summary>
/// Sets foreground color. Color cannot be one of those reserved for background
/// and foreground. These are automatically replaced.
/// </summary>
/// <param name="color"></param>
void setForeground(const RGB& color) { foreground = color; }

/// <summary>
/// Function to store the scribbles.
/// </summary>
/// <param name="scribbles"> Scribble data</param>
/// <param name="orig">Original image</param>
void printScribbles(const ColorMap& c_map, const short* scribbleData,
                    const Image<float>& orig, bool scrOnly) {
  Image<RGB> toPrint(orig.width(), orig.height());
  for (int h = 0; h < orig.height(); h++) {
    for (int w = 0; w < orig.width(); w++) {
      int idx = w + h * orig.width();
      if (!scrOnly && scribbleData[idx] == -1)
        toPrint.data()[idx] = {orig.data()[w + h * orig.width()],
                               orig.data()[idx], orig.data()[idx]};
      else if (scribbleData[idx] != -1)
        toPrint.data()[idx] = c_map.getColors()[scribbleData[idx]];
      else
        toPrint.data()[idx] = WHITE;
    }
  }
  imwrite(toPrint, "pictures/scribbles.png");
}

/// <summary>
/// Prints scribbles in relation to color mapping to indices
/// </summary>
/// <param name="scribbleData">scribbles</param>
/// <param name="c_map">Color map</param>
void printScribbleData(const short* scribbleData, const ColorMap& c_map) {
  Image<RGB> img = Image<RGB>(c_map.getWidth(), c_map.getHeight());
  for (int h = 0; h < c_map.getHeight(); h++) {
    for (int w = 0; w < c_map.getWidth(); w++) {
      img(w, h) =
          scribbleData[w + h * c_map.getWidth()] == -1
              ? BLACK
              : c_map.getColors()[scribbleData[w + h * c_map.getWidth()]];
    }
  }
  imwrite<RGB>(img, "pictures/scribbleData.png");
}

/// <summary>
/// Multilabel LazyBrush algorithm core. Computes the max-flow problem and
/// assigns background only.
/// </summary>
/// <param name="image">Intensity image</param>
/// <param name="scribbleData">Scribbles</param>
/// <param name="c_map">Color map</param>
/// <param name="minId">Currently segmented scribble</param>
/// <param name="min">Minimal coordinates of cropped working area in relation to
/// the whole image</param> <param name="max">Maximal coordinates of cropped
/// working area in relation to the whole image</param>
void runMultisegVersion(const Image<float>& image, const short* scribbleData,
                        ColorMap& c_map, BYTE minId, Coords& min, Coords& max) {
  typedef GridGraph_2D_4C<short, short, int> Grid;

  const int width = image.width();
  const int height = image.height();

  Grid* grid = new Grid(width, height);

  for (int y = min.y; y <= max.y; y++)
    for (int x = min.x; x <= max.x; x++) {
      grid->set_terminal_cap(
          grid->node_id(x, y),
          scribbleData[x + y * width] == minId
              ? K((short)((scribbleData[x + y * width] & 128) >> 7))
              : 0,
          scribbleData[x + y * width] > minId
              ? K((short)((scribbleData[x + y * width] & 128) >> 7))
              : 0);

      if (x < max.x) {
        const short cap = WEIGHT(image(x, y), image(x + 1, y));

        grid->set_neighbor_cap(grid->node_id(x, y), +1, 0, cap);
        grid->set_neighbor_cap(grid->node_id(x + 1, y), -1, 0, cap);
      }

      if (y < max.y) {
        const short cap = WEIGHT(image(x, y), image(x, y + 1));

        grid->set_neighbor_cap(grid->node_id(x, y), 0, +1, cap);
        grid->set_neighbor_cap(grid->node_id(x, y + 1), 0, -1, cap);
      }
    }

  grid->compute_maxflow();

  // c_map.newSegment(foreground, flags);
  c_map.setActive(minId);
  for (int y = min.y; y <= max.y; y++) {
    for (int x = min.x; x <= max.x; x++) {
      if (!grid->get_segment(grid->node_id(x, y)) &&
          c_map.getMaskAt(x, y) == -1)  // fore
        c_map.segment2Data(x, y);
    }
  }
  // c_map.printSegments(rounds++);
  delete grid;
}

/// <summary>
/// Resets scribble data
/// </summary>
/// <param name="scribbleData">Map of scribbles</param>
/// <param name="width">Image width</param>
/// <param name="height">Image height</param>
void resetScribbleData(short*& scribbleData, int width, int height) {
  if (scribbleData == nullptr)
    scribbleData = new short[(size_t)width * (size_t)height];
  for (int i = 0; i < width * height; i++) scribbleData[i] = -1;
}

/// <summary>
/// Creates default scribble setup. Prints background scribbles around the
/// scribble image border.
/// </summary>
/// <param name="scribbles"></param>
/// <param name="drawnScribbles"></param>
void createBackgroundScribbles(short*& scribbleData, int width, int height) {
  resetScribbleData(scribbleData, width, height);
  int offsetH = height - 1 - 2 * RADIUS;
  for (int dh = 0; dh <= 2 * RADIUS; dh++)
    for (int w = 0; w < width; w++) {
      scribbleData[w + dh * width] = 0;
      scribbleData[w + (offsetH + dh) * width] = 0;
    }

  int offsetW = width - 1 - 2 * RADIUS;
  for (int h = 0; h < height; h++)
    for (int dw = 0; dw <= 2 * RADIUS; dw++) {
      scribbleData[dw + h * width] = 0;
      scribbleData[dw + offsetW + h * width] = 0;
    }
}

/// <summary>
/// Sets all scribbles to the background ones.
/// </summary>
/// <param name="scribbles"></param>
void flipScribbles(Image<RGB>& scribbles) {
  for (int h = 2 * RADIUS; h < scribbles.height() - 2 * RADIUS - 1; h++) {
    for (int w = 2 * RADIUS; w < scribbles.width() - 2 * RADIUS - 1; w++) {
      if (scribbles(w, h) != BLACK) {
        scribbles(w, h) = WHITE;
      }
    }
  }
}

/// <summary>
/// Maps area that is covered only by one scribble to that scribble
/// </summary>
/// <param name="x">X coordinate in the original image</param>
/// <param name="y">Y coordinate in the original image</param>
/// <param name="c_map">Color map</param>
/// <param name="scribbleData">Scribbles</param>
/// <param name="tmp_map_data">Window of current segmentation state</param>
/// <param name="scribbles">Scribble indices</param>
/// <param name="min">Minimal coordinates</param>
/// <param name="max">Maximal coordinates</param>
/// <param name="minId">ID of the recently segmented scribble</param>
/// <param name="newMin">Cropped minimal coordinates</param>
/// <param name="newMax">Cropped maximal coordinates</param>
void areaTo1Scribble(int x, int y, ColorMap& c_map, short*& scribbleData,
                     short*& tmp_map_data, std::set<short>& remaining,
                     const Coords& min, const Coords& max, const BYTE minId,
                     Coords& newMin, Coords& newMax) {
  // initialize values
  std::list<Coords> coords;
  coords.push_back({x, y});
  int width = max.x - min.x + 1;
  short foundScribble = minId;
  bool twoScribblesFound = false;
  newMax = {min.x, min.y};
  newMin = {max.x, max.y};
  // search the area that does not belong to any scribble yet
  // if there are at least two scribbles, its bounding box will be neccessary
  while (!coords.empty()) {
    Coords xy = coords.front();
    coords.pop_front();

    int tmp_area_X =
            xy.x - min.x,  // temporary area is smaller than the whole image
        tmp_area_Y = xy.y - min.y;

    if (tmp_map_data[tmp_area_X + tmp_area_Y * width] == -1) {
      // mark visited
      tmp_map_data[tmp_area_X + tmp_area_Y * width] = -2;
      // add surrounding pixels
      if (xy.x > min.x) coords.push_back({xy.x - 1, xy.y});
      if (xy.x < max.x) coords.push_back({xy.x + 1, xy.y});
      if (xy.y > min.y) coords.push_back({xy.x, xy.y - 1});
      if (xy.y < max.y) coords.push_back({xy.x, xy.y + 1});

      // find bounding box of current area
      newMin.x = std::min(newMin.x, xy.x);
      newMin.y = std::min(newMin.y, xy.y);
      newMax.x = std::max(newMax.x, xy.x);
      newMax.y = std::max(newMax.y, xy.y);

      // scribble found in unoccupied area
      // skip if two or more scribbles are in the area
      if (scribbleData[xy.x + xy.y * c_map.getWidth()] > minId) {
        if (foundScribble == minId)
          foundScribble = scribbleData[xy.x + xy.y * c_map.getWidth()];
        else if (scribbleData[xy.x + xy.y * c_map.getWidth()] !=
                 foundScribble) {
          twoScribblesFound = true;
          remaining.insert(scribbleData[xy.x + xy.y * c_map.getWidth()]);
        }
      }
    }
  }
  // if there was found only one scribble, append the area to that scribble
  if (!twoScribblesFound) {
    coords.push_back({x, y});

    while (!coords.empty()) {
      Coords xy = coords.front();
      coords.pop_front();

      int tmp_area_X = xy.x - min.x, tmp_area_Y = xy.y - min.y;

      if (tmp_map_data[tmp_area_X + tmp_area_Y * width] == -2) {
        if (xy.x > newMin.x) coords.push_back({xy.x - 1, xy.y});
        if (xy.x < newMax.x) coords.push_back({xy.x + 1, xy.y});
        if (xy.y > newMin.y) coords.push_back({xy.x, xy.y - 1});
        if (xy.y < newMax.y) coords.push_back({xy.x, xy.y + 1});

        c_map.data()[xy.x + xy.y * c_map.getWidth()] = foundScribble;
        tmp_map_data[tmp_area_X + tmp_area_Y * width] = -3;
      }
    }
    // we dont need this area as it will be filled with found scribbles
    // thus the bounding box will be omitted in the next computation
    newMin = newMax = {-1, -1};
    return;
  }
  remaining.insert(
      foundScribble);  // two or more scribbles found, add the first one as well
}

/// <summary>
/// Detects areas unmapped to any scribbles. Calls areaTo1Scribble method for
/// its possible coloring.
/// </summary>
/// <param name="c_map">Color map</param>
/// <param name="scribbleData">Scribble map</param>
/// <param name="min">Minimal coordinates</param>
/// <param name="max">Maximal coordinates</param>
/// <param name="minId">ID of the recently segmented scribble</param>
/// <param name="scribbles">List of the remaining scribbles</param>
void colorDistinctAreas(ColorMap& c_map, short*& scribbleData, Coords& min,
                        Coords& max, BYTE minId, std::set<short>& scribbles) {
  int width = max.x - min.x + 1, height = max.y - min.y + 1;
  int size = (width) * (height);
  short* tmp_map_data = new short[size];
  assert(tmp_map_data != nullptr);
  Coords unionMin = max, unionMax = min;
  std::set<short> remainingScribbles;

  // get copy of mapped scribbles
  for (int h = min.y; h <= max.y; h++) {
    for (int w = min.x; w <= max.x; w++) {
      tmp_map_data[(w - min.x) + (h - min.y) * width] = c_map.getMaskAt(w, h);
    }
  }

  // find area that is unmapped
  for (int h = min.y; h <= max.y; h++) {
    for (int w = min.x; w <= max.x; w++) {
      if (tmp_map_data[(w - min.x) + (h - min.y) * width] == -1) {
        Coords newMin, newMax;
        areaTo1Scribble(w, h, c_map, scribbleData, tmp_map_data,
                        remainingScribbles, min, max, minId, newMin, newMax);
        if (newMin.x != -1) {
          unionMin.x = std::min(unionMin.x, newMin.x);
          unionMin.y = std::min(unionMin.y, newMin.y);
          unionMax.x = std::max(unionMax.x, newMax.x);
          unionMax.y = std::max(unionMax.y, newMax.y);
        }
      }
    }
  }

  scribbles = remainingScribbles;
  min = unionMin;
  max = unionMax;
  delete[] tmp_map_data;
}

/// <summary>
/// Applies scribbles with the multilabel LazyBrush algorithm.
/// </summary>
/// <param name="image">Intensity image</param>
/// <param name="c_map">Color map</param>
/// <param name="scribbleData">Scribbles</param>
/// <returns></returns>
bool applyScribbles(const Image<float>& image, ColorMap& c_map,
                    short*& scribbleData) {
  c_map.newComputation();
  std::set<short> scribbles;

  // skip adding 0 as it will run first
  for (short i = 1; i < c_map.getScribbleCount()[0]; i++) {
    scribbles.insert(i);
  }
  for (short i = 0; i < c_map.getScribbleCount()[1]; i++) {
    scribbles.insert(i + 128);
  }

  int scribbleId = 0;
  Coords min = {0, 0}, max = {image.width() - 1, image.height() - 1};

  // first background run
  runMultisegVersion(image, scribbleData, c_map, scribbleId, min, max);
  int k = 0;
  // in a while cycle
  while (true) {
    // find distinct areas that share border with only one scribble and remove
    // them alongside with scribbles
    colorDistinctAreas(c_map, scribbleData, min, max, scribbleId, scribbles);

    // stop if no scribbles are left
    if (scribbles.empty()) break;

    // select another scribble
    scribbleId = *scribbles.begin();
    scribbles.erase(scribbleId);

    // another background run
    runMultisegVersion(image, scribbleData, c_map, scribbleId, min, max);
  }

  return true;
}

}  // namespace ColorSegments

#endif  // !__COLOR_SEGMENTS
