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

#ifndef SHAPE_FILL
#define SHAPE_FILL

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <map>
#include <set>

#include "ColorMap.h"
#include "Depth.h"
#include "MatriceSolve.h"
#include "defines.h"

/// <summary>
/// Class used for estimating shapes of hidden parts of all segments.
/// </summary>
class ShapeFill {
 private:
  float strength;
  float scale;

 public:
  ShapeFill();

  ~ShapeFill();

  /// <summary>
  /// Run function. Calls other functions to create estimated border.
  /// </summary>
  /// <param name="depth">Depth information</param>
  /// <param name="c_map">Color map containing segmentation information</param>
  /// <param name="orig">Original intensity image</param>
  ///  <param name="block">Merge blocking selection</param>
  /// <param name="name">Name of the original image</param>
  void shapeFill(const Depth& depth, const ColorMap& c_map, float* orig,
                 std::string& filename, BYTE* block, std::string name);

 private:
  /// <summary>
  /// Saves segment borders by the segment outline.
  /// </summary>
  /// <param name="borders">Segment borders</param>
  /// <param name="block">Merge blocking selection</param>
  /// <param name="seg">Segment number</param>
  /// <param name="orig">Original image</param>
  /// <param name="c_map">Color map</param>
  /// <param name="incidences">Incident segments</param>
  /// <param name="number">Segment number for saving</param>
  void saveByBorders(char* borders, BYTE* block, BYTE seg, float* orig,
                     const ColorMap& c_map,
                     std::vector<std::set<short>>& incidences, int& number);

  /// <summary>
  /// Scales down image with binary segmentation data
  /// </summary>
  /// <param name="img">Image</param>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="newWidth">Output scaled down width</param>
  /// <param name="newHeight">Output scaled down height</param>
  /// <returns>Scaled down image</returns>
  float* scaleDown(float* img, int width, int height, int& newWidth,
                   int& newHeight);

  /// <summary>
  /// Scales up grayscale image with bilinear interpolation
  /// </summary>
  /// <param name="img">Image data</param>
  /// <param name="scaled">Scaled down image</param>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="scaledWidth">Width of the scaled down image</param>
  void scaleUp(float* img, float* scaled, int width, int height,
               int scaledWidth);

  /// <summary>
  /// Makes selected border bolder and return file prepared to saving
  /// </summary>
  /// <param name="im">Border image</param>
  /// <param name="number">Segment number</param>
  void boldBorder(Image<float>& im, std::string& number);

  /// <summary>
  /// Gauss-Seidel iteration using variable kernel.
  /// Based on:
  /// https://www.cg.tuwien.ac.at/research/publications/2009/jeschke-09-solver/jeschke-09-solver-paper.pdf
  /// </summary>
  /// <param name="compSpace">Working space</param>
  /// <param name="dists">Distance transform</param>
  /// <param name="borders">Boundary conditions</param>
  /// <param name="width">Image selection width</param>
  /// <param name="height">Image selection height</param>
  void GaussSeidelVar(float* compSpace, float* dists, float* borders, int width,
                      int height);

  /// <summary>
  /// Gauss-Seidel iteration using variable kernel.
  /// Based on:
  /// https://www.cg.tuwien.ac.at/research/publications/2009/jeschke-09-solver/jeschke-09-solver-paper.pdf
  /// </summary>
  /// <param name="compSpace">Working space</param>
  /// <param name="borders">Boundary conditions</param>
  /// <param name="width">Image selection width</param>
  /// <param name="height">Image selection height</param>
  void GaussSeidel(float* compSpace, float* borders, int width, int height);

  /// <summary>
  /// Shape fil run function managing each step for computing the segment
  /// bordes.
  /// </summary>
  /// <param name="c_map">Color map containing each segment data</param>
  /// <param name="depth">Depth data</param>
  /// <param name="borders">Image containing uniform borders</param>
  /// <param name="block">Merge blocking selection</param>
  /// <param name="orig">Original image</param>
  /// <param name="mins">Array of minimal coordinates</param>
  /// <param name="maxs">Array of maximal coordinates</param>
  /// <param name="seg">Current segment ID</param>
  /// <param name="incidences">Array of neighbours for each segment</param>
  /// <param name="number">Number for export</param>
  void SFRun(const ColorMap& c_map, const Depth& depth, char* borders,
             BYTE* block, float* orig, std::vector<vec2<int>>& mins,
             std::vector<vec2<int>>& maxs, BYTE seg,
             std::vector<std::set<short>>& incidences, int& number);


  /// <summary>
  /// Handles the 4-neighborhood during the border creation.
  /// </summary>
  /// <param name="borders">Borders to be done</param>
  /// <param name="w">Current width</param>
  /// <param name="h">Current height</param>
  /// <param name="dW">Neighbor width</param>
  /// <param name="dH">Neighbor height</param>
  /// <param name="depth">Depth data</param>
  /// <param name="c_map">Color map</param>
  /// <param name="seg">Current segment</param>
  /// <param name="alone">Flag for segments with no computable border</param>
  /// <param name="neighHigher"></param>
  /// <param name="firstNeigh"></param>
  /// <param name="im"></param>
  void handleNeighborhood(char* borders, int w, int h, int dW, int dH,
                          const Depth& depth, const ColorMap& c_map, BYTE seg,
                          bool* alone, bool* neighHigher, short* firstNeigh,
                          float* im);

  /// <summary>
  /// Creates borders of each segment, finds their neighbours and sets minimal
  /// and maximal coordinates for computation space dedicated for each segment.
  /// </summary>
  /// <param name="borders">Output image containing uniform borders</param>
  /// <param name="depth">Depth data</param>
  /// <param name="im">Intensity image</param>
  /// <param name="c_map">Color map containing each segment data</param>
  /// <param name="mins">Array of minimal coordinates</param>
  /// <param name="maxs">Array of maximal coordinates</param>
  /// <param name="incidences">Array of neighbours for each segment</param>
  /// <param name="block">Selection data</param>
  /// <returns>Whether the segments are alone</returns>
  bool* createBorders(char* borders, const Depth& depth, float* im,
                      const ColorMap& c_map, std::vector<vec2<int>>& mins,
                      std::vector<vec2<int>>& maxs,
                      std::vector<std::set<short>>& incidences, BYTE* block);

  /// <summary>
  /// Saves segment and boundary images based on the estimation.
  /// </summary>
  /// <param name="estimate">Estimated image</param>
  /// <param name="orig">Original image</param>
  /// <param name="block">Merge blocking selection</param>
  /// <param name="c_map">Color map</param>
  /// <param name="depth">Depth data</param>
  /// <param name="width"></param>
  /// <param name="height"></param>
  /// <param name="minCoord">Minimal coordinate</param>
  /// <param name="num">Save number</param>
  /// <param name="seg">Segment identifier</param>
  void setBoundary(float* estimate, float* orig, BYTE* block,
                   const ColorMap& c_map, const Depth& depth, int width,
                   int height, const vec2<int>& minCoord, BYTE num, BYTE seg);

  /// <summary>
  /// Selects computing space and sets its boundary, i.e its conditions.
  /// </summary>
  /// <param name="src">Source image</param>
  /// <param name="dst">Output image</param>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="ids">IDs of each unknown pixel</param>
  /// <param name="n">Number of pixels to be computed</param>
  void findBorder(float* src, float* dst, int width, int height,
                  int* ids = nullptr, int* n = nullptr);

  /// <summary>
  /// Selects an area belonging to a single segment.
  /// </summary>
  /// <param name="c_map">Color map containing data for each segment</param>
  /// <param name="img">Input image containing estimated data</param>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="coord">Minimal coordinate in the original image</param>
  /// <param name="incidences">Neighbours IDs incident to current
  /// segment</param> <param name="seg">Current segment ID</param>
  void treshold(const ColorMap& c_map, float* img, int width, int height,
                const vec2<int>& coord, const std::set<short>& incidences,
                BYTE seg);

  /// <summary>
  /// Creates template image.
  /// </summary>
  /// <param name="c_map">Color map</param>
  ///  <param name="orig">Original intensity image</param>
  Image<RGB> templateData(const ColorMap& c_map, std::string& filename);

  /// <summary>
  /// Creates content for the settings.txt file.
  /// </summary>
  std::string settingsContent();

  /// <summary>
  /// Creates content for the layers in the Monster Mash project.
  /// </summary>
  /// <param name="n">Number of layers</param>
  /// <returns>Content forthe layers.txt file</returns>
  std::string layersContent(int n);

  /// <summary>
  /// Distance visualization function. For debug purposes.
  /// </summary>
  /// <param name="im"></param>
  /// <param name="width"></param>
  /// <param name="height"></param>
  /// <param name="cnt"></param>
  void visualizeDist(float* im, int width, int height, int cnt);
};

#endif  // !SHAPE_FILL
