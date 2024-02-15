/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

This file is modified version of the algorithm found at
https://cs.brown.edu/people/pfelzens/dt/ made by Tomas Cicvarek for
the purposes of running distance transform algorithm in current project.
*/

/* distance transform */
#ifndef __DT
#define __DT

#define HIGH_CONSTANT 1e9

#include <assert.h>

#include <cmath>
#include <cstdlib>
#include <vector>

/// <summary>
/// DT transform of 1D array
/// Originated from https://cs.brown.edu/people/pfelzens/dt/index.html
/// </summary>
/// <param name="f"> function to compute DT from</param>
/// <param name="n"> length</param>
/// <returns></returns>
static float* dt1D(float* f, int n) {
  float* d = new float[n];
  int* v = new int[n];
  float* z = new float[(size_t)n + 1];
  int k = 0;
  v[0] = 0;
  z[0] = -HIGH_CONSTANT;
  z[1] = +HIGH_CONSTANT;
  for (int q = 1; q <= n - 1; q++) {
    float s = ((f[q] + q * q) - (f[v[k]] + v[k] * v[k])) / (2 * q - 2 * v[k]);
    while (s <= z[k]) {
      k--;
      s = ((f[q] + q * q) - (f[v[k]] + v[k] * v[k])) / (2 * q - 2 * v[k]);
    }
    k++;
    v[k] = q;
    z[k] = s;
    z[k + 1] = +HIGH_CONSTANT;
  }

  k = 0;
  for (int q = 0; q <= n - 1; q++) {
    while (z[k + 1] < q) k++;
    d[q] = (q - v[k]) * (q - v[k]) + f[v[k]];
  }

  delete[] v;
  delete[] z;
  return d;
}

/// <summary>
/// DT of 2D function
/// Originated from https://cs.brown.edu/people/pfelzens/dt/index.html
/// </summary>
/// <param name="im"></param>
/// <param name="width"></param>
/// <param name="height"></param>
static void dt2D(float* im, int width, int height) {
  float* f = new float[std::max(width, height)];

  // transform along columns
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      // f[y] = imRef(im, x, y);
      f[y] = im[x + y * width];
    }
    float* d = dt1D(f, height);
    for (int y = 0; y < height; y++) {
      im[x + y * width] = d[y];
    }
    delete[] d;
  }

  // transform along rows
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      f[x] = im[x + y * width];
    }
    float* d = dt1D(f, width);
    for (int x = 0; x < width; x++) {
      im[x + y * width] = d[x];
    }
    delete[] d;
  }

  delete[] f;
}

/// <summary>
/// DT of an image. Initial function
/// Originated from https://cs.brown.edu/people/pfelzens/dt/index.html
/// </summary>
/// <param name="im"> image data</param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <returns></returns>
static float* dt(float* im, int width, int height) {
  float* out = new float[(size_t)width * (size_t)height];
  assert(out != nullptr);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // 0.5f have the pixelas that represent background
      if (im[x + y * width] == 0.5f)
        out[x + y * width] = HIGH_CONSTANT;
      else
        out[x + y * width] = 0;
    }
  }

  dt2D(out, width, height);
  for (int i = 0; i < width * height; i++) out[i] = std::sqrt(out[i]);

  return out;
}

#endif  // __DT
