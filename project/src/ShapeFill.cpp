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

#include <assert.h>
#include <chrono>

#define SPACE 10
#define ERR_VAL 0.00001f

//#define TIME_MEASURE

#include <stdio.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <list>

#include "../dependencies/dt/dt.h"
#include "MatriceSolve.h"
#include "ShapeFill.h"
#include "Utils.h"
#include "zip.h"

ShapeFill::ShapeFill() {
  strength = 1.0f;
  scale = 2.0f;
}

ShapeFill::~ShapeFill() {}

/// <summary>
/// Debugging function for visualizing data in floating point arrays
/// </summary>
/// <param name="image">Image represented by floating point array</param>
/// <param name="width">Image width</param>
/// <param name="height">Image height</param>
/// <param name="fileName">Filename</param>
/// <returns>Success/Feasibility of saving the image</returns>
bool floatWrite(float* image, int width, int height,
                const std::string& fileName) {
  if (width < 1 || height < 1) {
    return false;
  }

  Image<RGB> rgbImage(width, height);

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      float grey = image[x + y * width];
      if (grey < 0.0f && grey != -1.0f) grey = 0.0f;
      if (grey == -1.0f) grey = 0.3f;
      if (grey > 1.0f) grey = 1.0f;
      rgbImage(x, y) = RGB(grey, grey, grey);
    }

  return imwrite(rgbImage, fileName);
}

void ShapeFill::GaussSeidelVar(float* compSpace, float* dists, float* borders,
                               int width, int height) {
  int iter = 0;       // count of passed iterations
  bool done = false;  // condition to end the computetion
  while (!done || iter < ITERATIONS) {
    done = true;
    iter++;

    float scale = 1.0f;            // scaling factor of the variable kernel
    if (iter >= ITERATIONS / 2) {  // condition to the shrink half strategy
      scale = 1.0f - (float)iter / (float)ITERATIONS;
    }
    for (int h = 0; h < height; h++) {
      for (int w = 0; w < width; w++) {
        int pos = w + h * width;

        // skip boundary positions and empty space
        if (dists[pos] == 0.0f || compSpace[pos] == -1.0f) continue;

        int dist = std::max(dists[pos] * scale, 1.0f);
        float denum = 0.0f, sum = 0.0f;
        int dirUp = h - dist, dirDown = h + dist, dirLeft = w - dist,
            dirRight = w + dist;

        // manage width / height coordinates
        if (dirUp >= 0 && dirUp < height) {
          sum += compSpace[w + dirUp * width];
          denum += 1;
        }
        if (dirDown < height && dirDown >= 0) {
          sum += compSpace[w + dirDown * width];
          denum += 1;
        }
        if (dirLeft >= 0 && dirLeft < width) {
          sum += compSpace[dirLeft + h * width];
          denum += 1;
        }
        if (dirRight < width && dirRight >= 0) {
          sum += compSpace[dirRight + h * width];
          denum += 1;
        }

        float at = compSpace[w + h * width];  // save the current value to
                                              // observe change in the pixel
        compSpace[pos] = sum / denum;
        if (std::abs(at - compSpace[w + h * width]) > ERR_VAL)
          done = false;  // condition to determine the end of iterations
      }
    }
  }
#ifdef TIME_MEASURE
  std::cout << "VK iterations " << iter << std::endl;
#endif
}


void ShapeFill::GaussSeidel(float* compSpace, float* borders,
                               int width, int height) {
  int iter = 0;       // count of passed iterations
  bool done = false;  // condition to end the computetion
  while (!done || iter < ITERATIONS) {
    done = true;
    iter++;
    for (int h = 0; h < height; h++) {
      for (int w = 0; w < width; w++) {
        int pos = w + h * width;

        // skip boundary positions and empty space
        if (borders[pos] != 0.5f || compSpace[pos] == -1.0f) continue;

        float denum = 0.0f, sum = 0.0f;
        int dirUp = h - 1, dirDown = h + 1, dirLeft = w - 1,
            dirRight = w + 1;

        // manage width / height coordinates
        if (dirUp >= 0 && dirUp < height) {
          sum += compSpace[w + dirUp * width];
          denum += 1;
        }
        if (dirDown < height && dirDown >= 0) {
          sum += compSpace[w + dirDown * width];
          denum += 1;
        }
        if (dirLeft >= 0 && dirLeft < width) {
          sum += compSpace[dirLeft + h * width];
          denum += 1;
        }
        if (dirRight < width && dirRight >= 0) {
          sum += compSpace[dirRight + h * width];
          denum += 1;
        }

        float at = compSpace[w + h * width];  // save the current value to
                                              // observe change in the pixel
        compSpace[pos] = sum / denum;
        if (std::abs(at - compSpace[w + h * width]) > ERR_VAL)
          done = false;  // condition to determine the end of iterations
      }
    }
  }
  std::cout << "NK iterations " << iter << std::endl;
}

float* ShapeFill::scaleDown(float* img, int width, int height, int& newWidth,
                            int& newHeight) {
  // pad the new image so it can be interpolated later up to its boundaries
  newWidth = std::ceil((float)width / scale) + 1;
  newHeight = std::ceil((float)height / scale) + 1;
  float* newim = new float[newWidth * newHeight];
  assert(newim != nullptr);

  // go through the bigger image and find positions in the scaled image
  for (int h = 0; h < newHeight; h++) {
    int hStart = h * scale;
    for (int w = 0; w < newWidth; w++) {
      int wStart = w * scale;
      // initialize counter for black and white pixels in the interpolated area
      int cnts[2] = {0, 0};
      for (int i = 0; i < scale; i++) {
        int hCoord = i + hStart;
        if (hCoord >= height) continue;
        for (int j = 0; j < scale; j++) {
          int wCoord = j + wStart;
          if (wCoord >= width) continue;
          // increase counter at valid pixels if possible
          if (img[hCoord * width + wCoord] != 0.5f)
            cnts[(int)img[hCoord * width + wCoord]] += 1;
        }
      }
      // if the number of non-grey pixels is smaller than half of the
      // interpolated area, set the pixel as grey
      if ((cnts[0] == 0 && cnts[1] == 0) ||
          cnts[0] + cnts[1] < scale * scale / 2) {
        newim[w + h * newWidth] = 0.5f;
        continue;
      }
      // othervise choose the color of majority of the pixels in the
      // interpolated area
      if (cnts[0] > cnts[1]) {
        newim[w + h * newWidth] = 0.0f;
        continue;
      }
      newim[w + h * newWidth] = 1.0f;
    }
  }
  return newim;
}

void ShapeFill::scaleUp(float* img, float* scaled, int width, int height,
                        int scaledWidth) {
  int prevW, prevH, coord;
  float coeffs[2];
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      // skip the already known background pixels
      if (img[w + h * width] != 0.0f) {
        img[w + h * width] = -1.0f;
        continue;
      }
      // position of the first pixels of the square interpolated area in the
      // smaller image
      prevW = w / scale;
      prevH = h / scale;
      // scale coefficients used in the interpolation
      coeffs[0] = (float)(w % (int)scale) / scale;
      coeffs[1] = (float)(h % (int)scale) / scale;
      coord = prevW + prevH * scaledWidth;

      // linear interpolation of the pixel in the new image depending on four
      // pixels in the smaller image
      img[w + h * width] =
          (scaled[coord] < 0.0f ? 0.0f : scaled[coord]) * (1.0f - coeffs[0]) *
              (1.0f - coeffs[1]) +
          (scaled[coord + 1] < 0.0f ? 0.0f : scaled[coord + 1]) * coeffs[0] *
              (1.0f - coeffs[1]) +
          (scaled[coord + scaledWidth] < 0.0f ? 0.0f
                                              : scaled[coord + scaledWidth]) *
              (1.0f - coeffs[0]) * coeffs[1] +
          (scaled[coord + 1 + scaledWidth] < 0.0f
               ? 0.0f
               : scaled[coord + 1 + scaledWidth]) *
              coeffs[0] * coeffs[1];
    }
  }
}

std::string ShapeFill::settingsContent() {
  return "v. 230112\n\
manipulationMode draw\n\
animRecMode overwrite\n\
playAnimation 1\n\
playAnimWhenSelected 1\n\
showControlPoints 1\n\
showTemplateImg 1\n\
showBackgroundImg 0\n\
showTextureUseMatcapShading 1\n\
enableArmpitsStitching 1\n\
enableNormalSmoothing 1\n\
middleMouseSimulation 0\n\
defaultInflationAmount 2\n";
}

std::string ShapeFill::layersContent(int n) {
  std::string ret = "v. 230112\n";

  ret += std::to_string(n) + "\n";
  for (int i = 0; i < n; i++) {
    ret += std::to_string(i) + "\n";
  }
  ret += "\n" + std::to_string(n) + "\n";
  for (int i = 0; i < n; i++) {
    ret += std::to_string(i) + "\n";
  }

  return ret;
}

void ShapeFill::boldBorder(Image<float>& im, std::string& number) {
  Image<BYTE> imOrg(im.width(), im.height());
  // set default values
  for (int i = 0; i < im.width() * im.height(); i++) imOrg.data()[i] = 255;
  // run the boldering phase
  for (int h = 0; h < im.height(); h++) {
    for (int w = 0; w < im.width(); w++) {
      if (im(w, h) != 1.0f) {
        // imOrg(w, h) = im(w, h) == 0.0f ? 0 : 64;
        for (int i = -1; i <= 1; i++) {
          if (h + i < 0 || h + i > im.height() - 1) continue;
          for (int j = -1; j <= 1; j++) {
            if (w + j < 0 || w + j > im.width() - 1) continue;
            imOrg(w + j, h + i) = im(w, h) == 0.0f ? 0 : 64;
          }
        }
      }
    }
  }

  int len;
  unsigned char* data = memFile(imOrg, &len);

  zip_t* zip = zip_open(MM_PROJECT, ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
  zip_entry_open(zip, std::string("_org_" + number + ".png").c_str());
  zip_entry_write(zip, data, len);
  zip_entry_close(zip);
  zip_close(zip);

  free(data);
}

void ShapeFill::setBoundary(float* estimate, float* orig, BYTE* block,
                            const ColorMap& c_map, const Depth& depth,
                            int width, int height, const vec2<int>& minCoord,
                            BYTE num, BYTE seg) {
  std::stringstream ss;
  ss << std::setw(3) << std::setfill('0') << (int)num;
  std::string number = ss.str();

  Image<float> im(c_map.getWidth(), c_map.getHeight());

  for (int h = 0; h < im.height(); h++) {
    int eh = h - minCoord.y;
    for (int w = 0; w < im.width(); w++) {
      int ew = w - minCoord.x;
      // out of bounds or background pixels are marked white
      if (ew < 0 || eh < 0 || ew >= width || eh >= height ||
          estimate[ew + eh * width] == 0.0f) {
        im(w, h) = 0.0f;
        continue;
      }
      im(w, h) = 1.0f;
    }
  }
  int len;
  unsigned char* data = memFile(im, &len);

  zip_t* zip = zip_open(MM_PROJECT, ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
  zip_entry_open(zip, std::string("_seg_" + number + ".png").c_str());
  zip_entry_write(zip, data, len);
  zip_entry_close(zip);
  zip_close(zip);
  free(data);

  for (int h = 0; h < im.height(); h++) {
    int eh = h - minCoord.y;
    for (int w = 0; w < im.width(); w++) {
      int ew = w - minCoord.x;
      // out of bounds or background pixels are marked to be out of the segment
      if (ew < 0 || eh < 0 || ew >= width || eh >= height ||
          estimate[ew + eh * width] == 0.0f) {
        im(w, h) = 1.0f;
        continue;
      }

      // coordinates in the estimated image of the pixel surroundings
      int dU = eh - 1, dD = eh + 1, dL = ew - 1, dR = ew + 1;

      // pixels at the border between the white and black pixels
      // or those that are at the border of the estimated area
      // are to be marked black or grey
      if (ew == 0 || eh == 0 || ew == width - 1 || eh == height - 1 ||
          (dU >= 0 && estimate[ew + dU * width] == 0.0f) ||
          (dL >= 0 && estimate[dL + eh * width] == 0.0f) ||
          (dR < width && estimate[dR + eh * width] == 0.0f) ||
          (dD < height && estimate[ew + dD * width] == 0.0f)) {
        if (block[w + h * c_map.getWidth()] == 1) {
          im(w, h) = 0.0f;
          continue;
        }
        // neighborhood in the whole image
        int oDU = h - 1, oDD = h + 1, oDL = w - 1, oDR = w + 1;

        // check boundary
        if (oDU < 0 || oDL < 0 || oDR >= c_map.getWidth() ||
            oDD >= c_map.getHeight()) {
          im(w, h) = 0.0f;
          continue;
        }

        // force merge only in areas that are not connected to the background
        if (block[w + h * c_map.getWidth()] == 2 &&
            c_map.getMaskAt(w, oDU) != 0 && c_map.getMaskAt(oDL, h) != 0 &&
            c_map.getMaskAt(oDR, h) != 0 && c_map.getMaskAt(w, oDD) != 0) {
          im(w, h) = 0.25f;
          continue;
        }

        // Smoothe out error made in the segmenting step, where the segment
        // boundary misses the image boundaries. Search the sourrounding pixels
        // for the lower intensities and set new boundary if necessary.
        if (c_map.getMaskAt(w, h) == seg &&
            (orig[w + oDU * c_map.getWidth()] < ORIG_WHITE_ERR ||
             orig[w + oDD * c_map.getWidth()] < ORIG_WHITE_ERR ||
             orig[oDL + h * c_map.getWidth()] < ORIG_WHITE_ERR ||
             orig[oDR + h * c_map.getWidth()] < ORIG_WHITE_ERR)) {
          im(w, h) = 0.0f;
          continue;
        }

        // ----- MERGE -----

        // merging boundaries are only with neighbouring segment that is not
        // background and does not belong to the image boundary
        if (c_map.getMaskAt(w, oDU) != 0 && c_map.getMaskAt(oDL, h) != 0 &&
            c_map.getMaskAt(oDR, h) != 0 && c_map.getMaskAt(w, oDD) != 0) {
          BYTE arrType = 0;
          // Look at the neighbouring segments. They form a set of indices.
          std::set<short> neighs;
          neighs.insert(c_map.getMaskAt(w, oDU));
          neighs.insert(c_map.getMaskAt(w, oDD));
          neighs.insert(c_map.getMaskAt(oDL, h));
          neighs.insert(c_map.getMaskAt(oDR, h));
          neighs.erase(seg);
          // Look through all arrows from current segment and compare all their
          // types. There are three solution, there is red arrow, green arrow or
          // none.
          for (std::list<TopologicalSorting::Edge>::iterator it =
                   depth.nodes[seg]->edgesOut.begin();
               it != depth.nodes[seg]->edgesOut.end(); it++) {
            BYTE to = (*it).to;
            if (neighs.find(to) != neighs.end()) {
              arrType = std::max(arrType, (BYTE)((*it).type + 1));
            }
          }
          // Default is no arrow (0), arrow that supports merge (1) is green,
          // arrow that denies merge is red (2). Default uses heuristics further
          // in the code, the other two have immediate results. This is expected
          // in situations where current segment is overlapped by another.
          if (arrType == 2) {
            im(w, h) = 0.0f;
            continue;
          }
          if (arrType == 1) {
            im(w, h) = 0.25f;
            continue;
          }

          // The current segment is overlapped by another one in this
          // coordinate. We merge when the difference in depth level is equal
          // to 1. The hole in the colour can have slightly darker colour than
          // white, so a small error is accounted for.
          if (depth.nodes[c_map.getMaskAt(w, oDU)]->depth -
                      depth.nodes[seg]->depth ==
                  1 ||
              depth.nodes[c_map.getMaskAt(oDL, h)]->depth -
                      depth.nodes[seg]->depth ==
                  1 ||
              depth.nodes[c_map.getMaskAt(oDR, h)]->depth -
                      depth.nodes[seg]->depth ==
                  1 ||
              depth.nodes[c_map.getMaskAt(w, oDD)]->depth -
                      depth.nodes[seg]->depth ==
                  1) {
            im(w, h) = 0.25f;
            continue;
          }

          // ----- OPENED CONTOUR -----

          // We expect the neighboring segments to be overlapped by the current
          // one and there is an opened contour in the image.
          if (orig[w + h * c_map.getWidth()] >= ORIG_WHITE_ERR &&
              c_map.getMaskAt(w, h) == seg &&
              (depth.nodes[c_map.getMaskAt(w, oDU)]->depth <
                   depth.nodes[seg]->depth ||
               depth.nodes[c_map.getMaskAt(oDL, h)]->depth <
                   depth.nodes[seg]->depth ||
               depth.nodes[c_map.getMaskAt(oDR, h)]->depth <
                   depth.nodes[seg]->depth ||
               depth.nodes[c_map.getMaskAt(w, oDD)]->depth <
                   depth.nodes[seg]->depth)) {
            im(w, h) = 0.25f;
            continue;
          }
        }

        im(w, h) = 0.0f;
        continue;
      }
      // inside of the foreground area is marked white
      im(w, h) = 1.0f;
    }
  }

  boldBorder(im, number);
}

void ShapeFill::findBorder(float* src, float* dst, int width, int height,
                           int* ids, int* n) {
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      int idx = w + h * width;
      // set default as an empty space
      dst[idx] = -1.0f;
      if (ids) ids[idx] = -1;
      if (src[idx] == 0.0f) {  // if there is closer segment
        dst[idx] = 0.5f;       // set the pixel as unknown by default
        // then check for all neighbours
        if (h > 0) {
          if (src[idx - width] ==
              0.5f)           // whether they are bordering the background
            dst[idx] = 0.0f;  //   then set the output at position to 0
          if (src[idx - width] == 1.0f) {  // or currently estimated segment
            dst[idx] = 1.0f;  //  and set the output at position to 1
            continue;
          }
        }
        if (w > 0) {
          if (src[idx - 1] == 0.5f) dst[idx] = 0.0f;
          if (src[idx - 1] == 1.0f) {
            dst[idx] = 1.0f;
            continue;
          }
        }
        if (h < height - 1) {
          if (src[idx + width] == 0.5f) dst[idx] = 0.0f;
          if (src[idx + width] == 1.0f) {
            dst[idx] = 1.0f;
            continue;
          }
        }
        if (w < width - 1) {
          if (src[idx + 1] == 0.5f) dst[idx] = 0.0f;
          if (src[idx + 1] == 1.0f) {
            dst[idx] = 1.0f;
            continue;
          }
        }
        if (dst[idx] == 0.5f &&
            n != nullptr) {  // the unknown pixels will be estimated later
          ids[idx] = *n;     // save their index and
          *n += 1;           // increase their count
        }
      }
    }
  }
}

void ShapeFill::treshold(const ColorMap& c_map, float* img, int width,
                         int height, const vec2<int>& coord,
                         const std::set<short>& incidences, BYTE seg) {
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      int idx = w + h * width;
      short neigh = c_map.getMaskAt(w + coord.x, h + coord.y);

      // Set white all pixels belonging to current segment and those estimated
      // to belong to it.
      if (neigh == seg || (img[w + h * width] >= 0.5f &&
                           incidences.find(neigh) != incidences.end())) {
        img[idx] = 1.0f;
        continue;
      }
      img[idx] = 0.0f;
    }
  }
}

void ShapeFill::saveByBorders(char* borders, BYTE* block, BYTE seg, float* orig,
                              const ColorMap& c_map,
                              std::vector<std::set<short>>& incidences,
                              int& number) {
  std::stringstream ss;
  ss << std::setw(3) << std::setfill('0') << number;
  std::string num = ss.str();

  Image<float> im(c_map.getWidth(), c_map.getHeight());

  for (int h = 0; h < im.height(); h++) {
    for (int w = 0; w < im.width(); w++) {
      // out of bounds or background pixels are marked white
      im(w, h) = c_map.getMaskAt(w, h) == seg ? 1.0f : 0.0f;
    }
  }
  int len;
  unsigned char* data = memFile(im, &len);

  zip_t* zip = zip_open(MM_PROJECT, ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
  zip_entry_open(zip, std::string("_seg_" + num + ".png").c_str());
  zip_entry_write(zip, data, len);
  zip_entry_close(zip);
  zip_close(zip);
  free(data);

  for (int h = 0; h < c_map.getHeight(); h++) {
    for (int w = 0; w < c_map.getWidth(); w++) {
      if (borders[w + h * c_map.getWidth()] == 1 &&
          c_map.getMaskAt(w, h) == seg) {
        int dU = h - 1, dD = h + 1, dL = w - 1, dR = w + 1;
        // hitting image boundary or selected areas
        if (dU < 0 || dL < 0 || dD >= c_map.getHeight() ||
            dR >= c_map.getWidth() || block[w + h * c_map.getWidth()] == 1) {
          im(w, h) = 0.0f;
          continue;
        }
        // Do not connect to the background (id 0)
        if (c_map.getMaskAt(w, dU) != 0 && c_map.getMaskAt(w, dD) != 0 &&
            c_map.getMaskAt(dL, h) != 0 && c_map.getMaskAt(dR, h) != 0) {
          // force the merge
          if (block[w + h * c_map.getWidth()] == 2 ||
              // or detect mergable area
              (orig[w + h * c_map.getWidth()] >= ORIG_WHITE_ERR &&
               // smoothe out errors from segmentation process
               // segmentation can set the segment next to the drawn boundary
               orig[dL + h * c_map.getWidth()] >= ORIG_WHITE_ERR &&
               orig[dR + h * c_map.getWidth()] >= ORIG_WHITE_ERR &&
               orig[w + dU * c_map.getWidth()] >= ORIG_WHITE_ERR &&
               orig[w + dD * c_map.getWidth()] >= ORIG_WHITE_ERR)) {
            im(w, h) = 0.25f;
            continue;
          }
        }

        im(w, h) = 0.0f;
        continue;
      }
      im(w, h) = 1.0f;
    }
  }
  boldBorder(im, num);
}

void ShapeFill::SFRun(const ColorMap& c_map, const Depth& depth, char* borders,
                      BYTE* block, float* orig, std::vector<vec2<int>>& minCs,
                      std::vector<vec2<int>>& maxCs, BYTE seg,
                      std::vector<std::set<short>>& incidences, int& number) {
  if (maxCs[seg].x == 0 && maxCs[seg].y == 0) return;
  // Segments without neighbours closer to the user are saved immediately
  if (incidences[seg].empty()) {
    saveByBorders(borders, block, seg, orig, c_map, incidences, number);
    number += 1;
    return;
  }

  // Set neccessary variables of the selected area in the image represented by
  // the c_map
  vec2<int> minC = minCs[seg], maxC = maxCs[seg];
  int width = maxC.x - minC.x + 1, height = maxC.y - minC.y + 1;
  float* compImg = new float[width * height];
  assert(compImg != nullptr);

  // mark segments and prepare them to scaling down
  for (int h = minC.y, i = 0; h <= maxC.y; h++, i++) {
    for (int w = minC.x, j = 0; w <= maxC.x; w++, j++) {
      // Current segment white
      if (c_map.getMaskAt(w, h) == seg) {
        compImg[j + i * width] = 1.0f;
        continue;
      }
      // Its neighbours black
      if (incidences[seg].find(c_map.getMaskAt(w, h)) !=
          incidences[seg].end()) {
        compImg[j + i * width] = 0.0f;
        continue;
      }
      // others grey
      compImg[j + i * width] = 0.5f;
    }
  }
  //floatWrite(compImg, width, height, "pictures/_com_img_" +
  //  std::to_string(seg) + ".png");

  float* tmpBorder = new float[width * height];
  assert(tmpBorder != nullptr);
  // find boundary conditions in the selection in normal size
  findBorder(compImg, tmpBorder, width, height);

#ifdef TIME_MEASURE
  std::cout << "\n\nSEGMENT : " << (int)seg << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
#endif  // TIME_MEASURE

  int scaledW, scaledH;
  // Create scaled down version of the selected area
  float* scaleieq = scaleDown(compImg, width, height, scaledW, scaledH);
  float* toCompute = new float[width * height];
  assert(toCompute != nullptr);
  int* ids = new int[scaledW * scaledH];
  assert(ids != nullptr);
  int n = 0;

  // Find boundary conditions
  findBorder(scaleieq, toCompute, scaledW, scaledH, ids, &n);
  delete[] scaleieq;
  floatWrite(toCompute, scaledW, scaledH,
             "pictures/_com_sc_img_" + std::to_string(seg) + ".png");

  // Solve a linear system on the scaled down image to estimate the boundary
  // beneath closer segments
  Matrices::solve(toCompute, scaledW, scaledH, ids, n);
  //floatWrite(toCompute, scaledW, scaledH,
  //           "pictures/_com_sc_img2_" + std::to_string(seg) + ".png");
  delete[] ids;

  // scale up the estimate
  scaleUp(compImg, toCompute, width, height, scaledW);
  delete[] toCompute;

  // find distance transform of the boundaries
  float* dists = dt(tmpBorder, width, height);
  //visualizeDist(dists, width, height, seg);

  // apply boundary conditions to the scaled up estimate as they can be blurred
  for (int i = 0; i < width * height; i++) {
    if (tmpBorder[i] == 0.0f || tmpBorder[i] == 1.0f) {
      compImg[i] = tmpBorder[i];
    }
  }
  // smooth out the estimated data
  GaussSeidelVar(compImg, dists, tmpBorder, width, height);
  delete[] dists;

#ifdef TIME_MEASURE
  auto end = std::chrono::high_resolution_clock::now();
  auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "Time for segment " << (int)seg << " with VK is " << dur.count()
            << " us" << std::endl;
  floatWrite(compImg, width, height,
            "pictures/_com_gs_img_" + std::to_string(seg) + ".png");

  float* space = new float[width * height];
  memcpy(space, tmpBorder, width * height * sizeof(float));
  // for gauss seidel iteration we can expect that most of the space will belong solely
  // to the overlapping segment, should the other occur, the algorithm should converge correctly 
  for (int i = 0; i < width * height; i++) {
    space[i] = space[i] == 0.5f ? 0.0f : space[i];
  }

  start = std::chrono::high_resolution_clock::now();
  GaussSeidel(space, tmpBorder, width, height);
  end = std::chrono::high_resolution_clock::now();
  dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "Time for segment " << (int)seg << " with NK is " << dur.count()
            << " us" << std::endl;
  floatWrite(tmpBorder, width, height,
             "pictures/_GS_bor_seg" + std::to_string((int)seg) + ".png");
  floatWrite(space, width, height,
             "pictures/_GS_out_seg" + std::to_string((int)seg) + ".png");
  delete[] space;
#endif // TIME_MEASURE


  floatWrite(compImg, width, height,
             "pictures/_com_gs_img_" + std::to_string(seg) + ".png");

  // trashold the estimate
  treshold(c_map, compImg, width, height, minC, incidences[seg], seg);

  // find the new boundary and save it
  vec2<int> size = {c_map.getWidth(), c_map.getHeight()};
  setBoundary(compImg, orig, block, c_map, depth, width, height, minC, number,
              seg);

  number += 1;
  delete[] compImg;
  delete[] tmpBorder;
}

void ShapeFill::handleNeighborhood(char* borders, int w, int h, int dW, int dH,
                        const Depth& depth, const ColorMap& c_map, BYTE seg,
                        bool* alone, bool* neighHigher, short* firstNeigh,
                        float* im) {
  borders[w + h * c_map.getWidth()] = 1;
  if (depth.nodes[seg]->depth < depth.nodes[c_map.getMaskAt(dW, dH)]->depth)
    neighHigher[seg] = true;
  if (firstNeigh[c_map.getMaskAt(w, h)] == -1) {
    firstNeigh[c_map.getMaskAt(w, h)] = c_map.getMaskAt(dW, dH);
  } else if (firstNeigh[c_map.getMaskAt(w, h)] != c_map.getMaskAt(dW, dH) ||
             depth.nodes[seg]->depth < depth.nodes[c_map.getMaskAt(dW, dH)]->depth) {
    alone[c_map.getMaskAt(w, h)] = false;
  }
  // opened connection to another segment
  if (im[w + h * c_map.getWidth()] != 0 &&
      // consider error in segmentation
      im[dW + dH * c_map.getWidth()] != 0) {
    alone[c_map.getMaskAt(w, h)] = false;
  }
}


bool* ShapeFill::createBorders(char* borders, const Depth& depth, float* im,
                               const ColorMap& c_map,
                               std::vector<vec2<int>>& minC,
                               std::vector<vec2<int>>& maxC,
                               std::vector<std::set<short>>& incidences,
                               BYTE* block) {
  bool* alone = new bool[256];
  bool* neighHigher = new bool[256];
  short* firstNeigh = new short[256];
  std::vector<std::set<short>> localIncidences;
  localIncidences.resize(256);

  for (int i = 0; i < 256; i++) {
    firstNeigh[i] = -1;
    neighHigher[i] = false;
    alone[i] = true;
  }

  for (int h = 0; h < c_map.getHeight(); h++) {
    for (int w = 0; w < c_map.getWidth(); w++) {
      short seg = c_map.getMaskAt(w, h);
      if (seg == 0) continue;

      // set minimal and maximal coordinates of the segment in pixel [w,h]
      minC[seg] = {std::min(minC[seg].x, w), std::min(minC[seg].y, h)};
      maxC[seg] = {std::max(maxC[seg].x, w), std::max(maxC[seg].y, h)};

      // directions
      int dU = h - 1, dD = h + 1, dL = w - 1, dR = w + 1;

      if (block[w + h * c_map.getWidth()] == RMB) {
        alone[seg] = false;
      }

      // find incident segments to the current one
      // bigger depth indicates closer distance
      // do it for all four directions
      // 1 - left size
      if (dL >= 0 && c_map.getMaskAt(dL, h) != seg) {
        handleNeighborhood(borders, w, h, dL, h, depth, c_map, seg, alone,
                           neighHigher, firstNeigh, im);
        localIncidences[seg].insert(c_map.getMaskAt(dL, h));
      }

      // 2 - right size
      if (dR < c_map.getWidth() && c_map.getMaskAt(dR, h) != seg) {
        handleNeighborhood(borders, w, h, dR, h, depth, c_map, seg, alone,
                           neighHigher, firstNeigh, im);
        localIncidences[seg].insert(c_map.getMaskAt(dR, h));
      }

      // 3 - upper side
      if (h > 0 && c_map.getMaskAt(w, dU) != seg) {
        handleNeighborhood(borders, w, h, w, dU, depth, c_map, seg, alone,
                           neighHigher, firstNeigh, im);
        localIncidences[seg].insert(c_map.getMaskAt(w, dU));
      }

      // 4 - lower side
      if (h < c_map.getHeight() - 1 && c_map.getMaskAt(w, dD) != seg) {
        handleNeighborhood(borders, w, h, w, dD, depth, c_map, seg, alone,
                           neighHigher, firstNeigh, im);
        localIncidences[seg].insert(c_map.getMaskAt(w, dD));
      }
    }
  }
  std::vector<vec2<int>> tmpMin = minC, tmpMax = maxC;
  int i = 1;
  for (std::list<BYTE>::const_iterator it = depth.order.begin();
       it != depth.order.end(); it++, i++) {
    BYTE id = *it;
    if (!neighHigher[id]) continue;
    for (std::list<BYTE>::const_iterator it2 =
             std::next(depth.order.begin(), i);
         it2 != depth.order.end(); it2++) {
      BYTE id2 = *it2;
      if (depth.nodes[id]->depth < depth.nodes[id2]->depth)
        incidences[id].insert(id2);
    }
  }

  // set the resolution of the selected areas to the size of the incident
  // segments, as the segments could be hidden anywhere behind their neighbours
  for (int i = 1; i < incidences.size(); i++) {
    if (incidences[i].empty()) continue;
    for (std::set<short>::iterator it = incidences[i].begin();
         it != incidences[i].end(); it++) {
      short neighbour = *it;

      tmpMin[i].x = std::max(std::min(tmpMin[i].x, minC[neighbour].x - 1), 0);
      tmpMin[i].y = std::max(std::min(tmpMin[i].y, minC[neighbour].y - 1), 0);

      tmpMax[i].x = std::min(std::max(tmpMax[i].x, maxC[neighbour].x + 1),
                             c_map.getWidth() - 1);
      tmpMax[i].y = std::min(std::max(tmpMax[i].y, maxC[neighbour].y + 1),
                             c_map.getHeight() - 1);
    }
  }

  minC = tmpMin;
  maxC = tmpMax;

  delete[] neighHigher;
  delete[] firstNeigh;
  return alone;
}

Image<RGB> ShapeFill::templateData(const ColorMap& c_map,
                                   std::string& filename) {
  Image<float> im = imread<float>(FOLDER + filename);
  Utils::scaleAndPad(im);
  Utils::gammaCorrection(im);
  Image<RGB> ret(im.width(), im.height());
  // Original contains intensity, and color map contains color at pixel
  // location.
  for (int i = 0; i < c_map.getHeight() * c_map.getWidth(); i++) {
    ret.data()[i] = im.data()[i] * c_map.getColorAt(i);
  }
  return ret;
}

void ShapeFill::shapeFill(const Depth& depth, const ColorMap& c_map,
                          float* _orig, std::string& filename, BYTE* block,
                          std::string name) {
  if (depth.order.size() == 0) return;
  float* orig = new float[c_map.getWidth() * c_map.getHeight()];
  memcpy(orig, _orig, c_map.getWidth() * c_map.getHeight() * sizeof(float));
  char* borders = new char[c_map.getWidth() * c_map.getHeight()];
  std::vector<vec2<int>> mins, maxs;
  std::vector<std::set<short>> incidences;
  mins.resize(256);
  maxs.resize(256);
  incidences.resize(256);
  {
    // Prepare zip file
    FILE* f =
        fopen(std::string(std::string(FOLDER) + "mm_project.zip").c_str(), "r");
    if (f) {
      fclose(f);
      remove(std::string(std::string(FOLDER) + "mm_project.zip").c_str());
    }
    // Add settings file
    std::string sc = settingsContent();
    zip_t* zip = zip_open(MM_PROJECT, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    zip_entry_open(zip, "settings.txt");
    zip_entry_write(zip, sc.c_str(), sc.size());
    zip_entry_close(zip);
    zip_close(zip);
  }

  // prepare borders, minimal and maximal coordinates of the segments
  for (int i = 0; i < 256; i++) {
    mins[i] = {c_map.getWidth(), c_map.getHeight()};
    maxs[i] = {0, 0};
  }

  /*Utils::blur(orig, c_map.getWidth(), c_map.getHeight());
  Utils::edgeDetect(orig, c_map.getWidth(), c_map.getHeight());*/
  Utils::gammaCorrection(orig, c_map.getWidth(), c_map.getHeight());
  Utils::blurAndTreshold(orig, c_map.getWidth(), c_map.getHeight());

  for (int i = 0; i < c_map.getWidth() * c_map.getHeight(); i++) borders[i] = 0;
  bool* separateSegs =
      createBorders(borders, depth, orig, c_map, mins, maxs, incidences, block);
  int number = 0;

  std::vector<BYTE> segs = {depth.order.begin(), depth.order.end()};

  // run the operation for all segments that are not separated from the rest
  for (int i = 1; i < segs.size(); i++) {
    if (separateSegs[segs[i]]) continue;
    SFRun(c_map, depth, borders, block, orig, mins, maxs, segs[i], incidences,
          number);
    std::cout << "Segment " << (int)segs[i] << " done" << std::endl;
  }

  // Save template data and layers data
  Image<RGB> Template = templateData(c_map, filename);
  int len;
  unsigned char* td = memFile(Template, &len);
  std::string lc = layersContent(number);

  zip_t* zip = zip_open(MM_PROJECT, ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
  zip_entry_open(zip, "layers.txt");
  zip_entry_write(zip, lc.c_str(), lc.size());
  zip_entry_close(zip);

  zip_entry_open(zip, "template.png");
  zip_entry_write(zip, td, len);
  zip_entry_close(zip);
  zip_close(zip);

  free(td);
  delete[] separateSegs;
  delete[] borders;
  delete[] orig;
}

void ShapeFill::visualizeDist(float* im, int width, int height, int cnt) {
  Image<float> im2(width, height);
  for (int i = 0; i < im2.width() * im2.height(); i++) {
    im2.data()[i] = im[i];
  }
  float max = 0.0f;
  float min = 255.0f;
  for (int i = 0; i < im2.width() * im2.height(); i++) {
    max = std::max(im2.data()[i], max);
    min = std::min(im2.data()[i], min);
  }

  for (int i = 0; i < width * height; i++) {
    im2.data()[i] -= min;
  }
  for (int i = 0; i < width * height; i++) {
    im2.data()[i] /= (max - min);
  }
  imwrite(im2, "pictures/__dist" + std::to_string(cnt) + ".png");
}
