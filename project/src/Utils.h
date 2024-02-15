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

#ifndef __UTILS
#define __UTILS

#include "Image.h"
#include "defines.h"

/// <summary>
/// Contains useful basic tools.
/// </summary>
static class Utils {
 public:
  /// <summary>
  /// Reads the image from memory and decides which kernel and other parameters
  /// are used on the intensity image. In one case the blurAndTreshold is not
  /// applied.
  /// </summary>
  /// <param name="image">Intensity image</param>
  /// <param name="varianceLevel">Level of blurAndTreshold</param>
  /// <param name="filename">Name of the loaded file</param>
  static void blurImage(Image<float>& image, BYTE varianceLevel,
                        std::string filename);

  /// <summary>
  /// Reads the image from memory and decides which kernel and other parameters
  /// are used on the intensity image. In one case the blurAndTreshold is not
  /// applied.
  /// </summary>
  /// <param name="image">Intensity image</param>
  /// <param name="width"></param>
  /// <param name="height"></param>
  static void blurAndTreshold(float* image, int width, int height);

  /// <summary>
  /// Reads the image from memory and decides which kernel and other parameters
  /// are used on the intensity image. In one case the blurAndTreshold is not
  /// applied.
  /// </summary>
  /// <param name="image">Intensity image</param>
  static void blurAndTreshold(Image<float>& img);

  /// <summary>
  /// Scale image.
  /// </summary>
  /// <param name="im"></param>
  static void scale(Image<float>& im, int width, int height);

  /// <summary>
  /// Invokes scaling and padding procedures.
  /// </summary>
  /// <param name="im"></param>
  static void scaleAndPad(Image<float>& im);

  /// <summary>
  /// Applies gamma correction and then treshold to the image.
  /// </summary>
  /// <param name="im"></param>
  static void gammaCorrectionPlusTreshold(float* im, int width, int height);

  /// <summary>
  /// Applies gamma correction to the image.
  /// </summary>
  /// <param name="im"></param>
  static void gammaCorrection(float* im, int width, int height, BYTE expL = 3);

  /// <summary>
  /// Applies gamma correction to the image.
  /// </summary>
  /// <param name="img"></param>
  static void gammaCorrection(Image<float>& img, BYTE expL = 3);

  /// <summary>
  /// Finds edges in the image.
  /// </summary>
  /// <param name="img"></param>
  /// <param name="width"></param>
  /// <param name="height"></param>
  static void edgeDetect(float* img, int width, int height);

  /// <summary>
  /// Computes blur using predefined values.
  /// </summary>
  /// <param name="img"></param>
  /// <param name="varianceLevel"></param>
  static void blur(Image<float>& im, float varianceLevel = 1.0f);

  /// <summary>
  /// Computes blur using predefined values.
  /// </summary>
  /// <param name="img"></param>
  /// <param name="width"></param>
  /// <param name="height"></param>
  /// <param name="varianceLevel"></param>
  static void blur(float* im, int width, int height, BYTE varianceLevel = 1);

  /// <summary>
  /// Prints hue image.
  /// </summary>
  static void printHue();

  /// <summary>
  /// Converts HSL color to RGB
  /// Computation at https://en.wikipedia.org/wiki/HSL_and_HSV
  /// </summary>
  /// <param name="hsl"></param>
  /// <returns></returns>
  static RGB HSL2RGB(HSL& hsl);

  /// <summary>
  /// Converts RGB color to HSL.
  /// Computation at
  /// https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/
  /// </summary>
  /// <param name="rgb"></param>
  /// <returns>HSL color</returns>
  static HSL RGB2HSL(RGB& rgb);

 private:
  /// <summary>
  /// Computes 1D Gaussian kernel by the equation G(x, y, sigma) = 1 / (sqrt(2 *
  /// pi) * sigma ^ 2) * e ^ (-x ^ 2 / (2 * sigma ^ 2))
  /// </summary>
  /// <param name="variance">Variance</param>
  /// <param name="radius">Kernel radius</param>
  /// <returns>Computed kernel</returns>
  static float* computeKernel(double variance, int radius);

  /// <summary>
  /// Convolution using separable 2D kernel saved as 1D kernel.
  /// </summary>
  /// <param name="kernel">Convolution kernel</param>
  /// <param name="img">Intensity image</param>
  /// <param name="radius">kernel radius</param>
  static void convolution_separableKernel(float* kernel, Image<float>& img,
                                          int radius);

  /// <summary>
  /// Convolutes image with 2D kernel.
  /// </summary>
  /// <param name="kernel"></param>
  /// <param name="img"></param>
  /// <param name="radius"></param>
  /// <param name="width"></param>
  /// <param name="height"></param>
  static void convolution(float* kernel, float* img, int radius, int width,
                          int height);

  /// <summary>
  /// Computes 2D kernel based on input values.
  /// </summary>
  /// <param name="sigma"></param>
  /// <param name="radius"></param>
  static float* computeKernel_2D(float sigma, int radius);

  /// <summary>
  /// Computes LoG value in kernel for certain position.
  /// </summary>
  /// <param name="x"></param>
  /// <param name="y"></param>
  /// <param name="sigma"></param>
  static float LoG(float x, float y, float sigma);
};

#endif  // !__UTILS