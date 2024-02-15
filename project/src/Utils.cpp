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

#define _USE_MATH_DEFINES

#include "Utils.h"

#include <cmath>

void Utils::scale(Image<float>& im, int width, int height) {
  Image<float> tmp(width, height);
  float scaleX = (float)im.width() / (float)width;
  float scaleY = (float)im.height() / (float)height;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      tmp(w, h) = im(w * scaleX, h * scaleY);
    }
  }
  im = tmp;
}

void Utils::scaleAndPad(Image<float>& im) {
  if (im.width() == MM_WIDTH && im.height() == MM_HEIGHT) return;
  float MMratio = (float)MM_WIDTH / (float)MM_HEIGHT;  // 10 : 8 -> 1,25
  float ratio = (float)im.width() / (float)im.height();

  // image is proportionally wider than expected ratio
  // and scale is needed
  if (ratio > MMratio && im.width() != MM_WIDTH) {
    scale(im, MM_WIDTH, im.height() * MM_WIDTH / im.width());
  }
  // image is proportionally heigher than expected ratio
  // and scale is needed
  else if (ratio < MMratio && im.height() != MM_HEIGHT) {
    scale(im, im.width() * MM_HEIGHT / im.height(), MM_HEIGHT);
  }
  // ratio is the same but the sizes differ, no padding is needed
  else {
    scale(im, MM_WIDTH, MM_HEIGHT);
    return;
  }

  // prepare returning image
  Image<float> ret(MM_WIDTH, MM_HEIGHT);
  for (int i = 0; i < MM_WIDTH * MM_HEIGHT; i++) ret.data()[i] = 1.0f;

  // find the first coordinates of the scaled image
  int sw = (MM_WIDTH - im.width()) / 2.0f,
      sh = (MM_HEIGHT - im.height()) / 2.0f;

  for (int h = sh, y = 0; y < im.height(); h++, y++) {
    for (int w = sw, x = 0; x < im.width(); w++, x++) {
      ret(w, h) = im(x, y);
    }
  }
  im = ret;
}

void Utils::gammaCorrection(Image<float>& img, BYTE expL) {
  gammaCorrection(img.data(), img.width(), img.height(), expL);
}

void Utils::gammaCorrection(float* im, int width, int height, BYTE expL) {
  for (int h = 0; h < height; h += 1) {
    for (int w = 0; w < width; w += 1) {
      im[w + h * width] = std::powf(im[w + h * width], EXPONENT);
    }
  }
}

void Utils::gammaCorrectionPlusTreshold(float* im, int width, int height) {
  for (int h = 0; h < height; h += 1) {
    for (int w = 0; w < width; w += 1) {
      im[w + h * width] =
          std::powf(im[w + h * width], EXPONENT) > 0.85f ? 1.0f : 0.0f;
    }
  }
}

void Utils::blur(Image<float>& im, float varianceLevel) {
  float variance = 1.0f + VARIANCE_BASE * varianceLevel;
  int radius = 6 * variance + 1;

  float* ker = computeKernel(variance, radius);
  convolution_separableKernel(ker, im, radius);
}

void Utils::blur(float* im, int width, int height, BYTE varianceLevel) {
  Image<float> img(width, height);
  memcpy(img.data(), im, width * height * sizeof(float));
  blur(img, varianceLevel);
  memcpy(im, img.data(), width * height * sizeof(float));
}

void Utils::blurAndTreshold(Image<float>& img) {
  blurAndTreshold(img.data(), img.width(), img.height());
}

void Utils::blurAndTreshold(float* image, int width, int height) {
  Image<float> tmp(width, height);
  memcpy(tmp.data(), image, width * height * sizeof(float));
  Utils::scaleAndPad(tmp);

  blur(tmp, 1.5f);

  for (int i = 0; i < width * height; i++) {
    image[i] = tmp.data()[i] > 0.65f ? 1.0f : 0.0f;
  }
}

float* Utils::computeKernel(double variance, int radius) {
  int len = 2 * radius + 1;
  float* kernel = new float[len];
  float sum = 0;
  for (int i = -radius; i < radius + 1; i++) {
    kernel[i + radius] = powf(M_E, -i * i / (2 * variance * variance));
    sum += kernel[i + radius];
  }
  for (int i = 0; i < len; i++) {
    kernel[i] /= sum;
  }
  return kernel;
}

void Utils::convolution_separableKernel(float* kernel, Image<float>& img,
                                        int radius) {
  int width = img.width();
  int height = img.height();
  float* tmp = new float[height * width];
  float* tmp2 = new float[height * width];
  // process rows
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      tmp[h * width + w] = 0;

      for (int r = -radius; r <= radius; r += 1) {
        int rwidx = r + w;
        rwidx = rwidx < 0 ? 0 : (rwidx >= width ? width - 1 : rwidx);
        float var = img(rwidx, h) * kernel[r + radius];
        tmp[h * width + w] += var;
      }
    }
  }
  // process columns
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      tmp2[h * width + w] = 0;

      for (int r = -radius; r <= radius; r += 1) {
        int rhidx = r + h;
        rhidx = rhidx < 0 ? 0 : (rhidx >= height ? height - 1 : rhidx);
        float var = tmp[rhidx * width + w] * kernel[r + radius];
        tmp2[h * width + w] += var;
      }
    }
  }

  // save results
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      img(w, h) = tmp2[h * width + w];
    }
  }

  delete[] tmp;
  delete[] tmp2;
  delete[] kernel;
}

void Utils::edgeDetect(float* img, int width, int height) {
  float* img_copy = new float[width * height];
  memcpy(img_copy, img, width * height * sizeof(float));

  const float sigma = 3.0f;
  int radius = 6.0f * sigma + 1;
  float* ker = computeKernel_2D(sigma, radius);
  convolution(ker, img_copy, radius, width, height);

  // save the results
  for (int i = 0; i < width * height; i++) {
    img[i] = img_copy[i] < 0.0f ? 1.0f : 0.0f;
  }
  delete[] ker;
  delete[] img_copy;
  return;

  // const float  // sigma = 4.0f,
  //    C_g = M_PI * sigma,
  //    C_log = 3.9f * sigma,
  //    // k_d is smaller than the equation var * PI / sqrt(C_g ^ 2 + C_log ^ 2)
  //    k_d = (sigma * M_PI) / std::sqrtf(C_g * C_g + C_log * C_log) - 0.0001f;
  //// k_s constant is denoted by two inequalities
  //// (C_log * k_d) / (sigma * PI) < 1 / k_s
  //// and
  //// 1 / k_s < sqrt(1 - ((C_g * k_d) / (sigma * M_PI) ^ 2)
  //// we take both sides and make average of them, then inverse the result
  //// to aquire k_s
  // const float k_s =
  //    1.0f / (((C_log * k_d / (sigma * M_PI)) +
  //             std::sqrtf(1.0f - std::powf(C_g * k_d / (sigma * M_PI), 2.0f)))
  //             /
  //            2.0f);

  // std::cout << (1.0f / sigma) << std::endl;
  // std::cout << (1.0f / k_d) << std::endl;
  // std::cout << 1.0f - (1.0f / k_d) << std::endl;

  // float G_var = std::sqrtf(1.0f - (1.0f / (k_s * k_s))) * sigma;
  // int G_rad = 6.0f * G_var + 1;
  //// get the Gaussian separable kernel
  // float* G_ker = computeKernel(G_var, G_rad);
  // convolution_separableKernel(G_ker, img_copy, G_rad, width, height);

  //// save the results
  // for (int i = 0; i < width * height; i++) {
  //  img[i] = img_copy[i] / 255.0f;
  //}
  // delete[] G_ker;
}

float Utils::LoG(float x, float y, float sigma) {
  return (1.0f / (M_PI * sigma * sigma)) *
         (((x * x + y * y) / (2.0f * sigma * sigma)) - 1.0f) *
         powf(M_E, -(x * x + y * y) / (2 * sigma * sigma));
}

float* Utils::computeKernel_2D(float sigma, int radius) {
  int len = 2 * radius + 1;
  float* kernel = new float[len * len];
  // float sum = 0;
  for (int y = -radius; y < radius + 1; y++) {
    for (int x = -radius; x < radius + 1; x++) {
      kernel[x + radius + (y + radius) * len] = LoG(x, y, sigma);
      // sum += kernel[x + radius];
    }
  }
  // for (int i = 0; i < len; i++) {
  //  kernel[i] /= sum;
  //  //std::cout << kernel[y] << ' ';
  //}
  // std::cout << std::endl;
  return kernel;
}

void Utils::convolution(float* kernel, float* img, int radius, int width,
                        int height) {
  float* arr = (float*)_malloca(height * width * sizeof(float));
  int len = 2 * radius + 1;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      arr[h * width + w] = 0.0f;
      for (int rh = -radius; rh <= radius; rh++) {
        for (int rw = -radius; rw <= radius; rw++) {
          int hidx = h + rh;
          int widx = w + rw;
          hidx = hidx < 0 ? 0 : (hidx >= height ? height - 1 : hidx);
          widx = widx < 0 ? 0 : (widx >= width ? width - 1 : widx);

          arr[h * width + w] += img[hidx * width + widx] *
                                kernel[(rh + radius) * len + rw + radius];
        }
      }
    }
  }

  // save results
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      img[h * width + w] = arr[h * width + w];
    }
  }
  _freea(arr);
}

void Utils::printHue() {
  Image<RGB> im(360, 7);
  // 0, 360 - > red | 120 -> green | 240 -> blue

  RGB rgb[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

  BYTE c1 = 0, c2 = 1;
  int t = 0;
  for (int w = 0; w < 360; w++, t++) {
    if (t % 120 == 0 && w != 0) {
      c1 = c2;
      c2 = (c2 + 1) % 3;
      t = 0;
    }
    float inter = (float)t / 120.0f;
    RGB c = rgb[c1] * (1.0f - inter) + inter * rgb[c2];

    for (int h = 0; h < 7; h++) {
      im(w, h) = c;
    }
  }
  imwrite(im, "data/hue.png");
}

RGB Utils::HSL2RGB(HSL& hsl) {
  float h = std::fmod(hsl.H, 360.0f),
        s = hsl.S <= 0.0f   ? 0.0f
            : hsl.S >= 1.0f ? 1.0f
                            : hsl.S,
        l = hsl.L <= 0.0f   ? 0.0f
            : hsl.L >= 1.0f ? 1.0f
                            : hsl.L;
  while (h >= 360.0f) h -= 360.0f;
  while (h < 0.0f) h += 360.0f;

  float c = (1.0f - std::fabs(2.0f * l - 1)) * s;
  float _h = h / 60.0f;
  float x = c * (1.0f - std::fabs((std::fmod(_h, 2) - 1.0f)));
  float m = l - c / 2.0f;

  float R = 0, G = 0, B = 0;
  if (0.0 <= _h && _h < 1.0f) {
    R = c;
    G = x;
    B = 0;
  } else if (1.0 <= _h && _h < 2.0f) {
    R = x;
    G = c;
    B = 0;
  } else if (2.0 <= _h && _h < 3.0f) {
    R = 0;
    G = c;
    B = x;
  } else if (3.0 <= _h && _h < 4.0f) {
    R = 0;
    G = x;
    B = c;
  } else if (4.0 <= _h && _h < 5.0f) {
    R = x;
    G = 0;
    B = c;
  } else if (5.0 <= _h && _h < 6.0f) {
    R = c;
    G = 0;
    B = x;
  }

  return {R + m, G + m, B + m};
}

HSL Utils::RGB2HSL(RGB& rgb) {
  float r = std::fmin(std::fmax(0.0f, rgb.r), 1.0f),
        g = std::fmin(std::fmax(0.0f, rgb.g), 1.0f),
        b = std::fmin(std::fmax(0.0f, rgb.b), 1.0f);
  float M = std::fmax(std::fmax(r, g), b), m = std::fmin(std::fmin(r, g), b),
        d = M - m;
  float L = (M + m) / 2.0f;
  if (M == m) {
    return {0, 0, L};
  }
  float S = L <= 0.5f ? (M - m) / (M + m) : (M - m) / (2.0f - M - m);
  float H;

  if (r == M) {
    H = (g - b) / (M - m);
  }
  if (g == M) {
    H = 2.0f + (b - r) / (M - m);
  }
  if (b == M) {
    H = 4.0f + (r - g) / (M - m);
  }
  H = H * 60.0f;
  H = std::fmod(H, 360.0f);
  while (H < 0) H += 360.0f;

  return {H, S, L};
}
