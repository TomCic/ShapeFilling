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

#ifndef MATRICE_SOLVE
#define MATRICE_SOLVE

#include <map>
#include <vector>

/// <summary>
/// Class used for solving linear system for image laplace.
/// </summary>
static class Matrices {
 public:
  /// <summary>
  /// Function used to compute linear system A*x=b
  /// where the square matrix A of size n contains neighbourhood data,
  /// the right hand size b containd baoudary conditions and
  /// x is the vector of unknown pixels.
  /// </summary>
  /// <param name="img">Input / Output image</param>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="ids">IDs of all unknown pixels</param>
  /// <param name="n">Number of unknown pixels</param>
  static void solve(float* img, int width, int height, int* ids, int n);

 private:
  /// <summary>
  /// Function inserting data to the matrix A and modifying data in the right
  /// hand vector depending on the space and boundary conditions in the image.
  /// </summary>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="id">Current pixel ID</param>
  /// <param name="nw">Neighbour width coordinate</param>
  /// <param name="nh">Neighbour height coordinate</param>
  /// <param name="neighCnt">Number of neighbours</param>
  /// <param name="_coefs">Coefficients describing the A matrix</param>
  /// <param name="_b">Right hand side</param>
  /// <param name="img">Image</param>
  /// <param name="ids">IDs of each unknown pixel</param>
  static void insertValues(int width, int height, int id, int nw, int nh,
                           int& neighCnt, void* _coefs, void* _b, float* img,
                           int* ids);

  /// <summary>
  /// Function initializing the structures needed for the linear system.
  /// </summary>
  /// <param name="_coefs">Coefficients describing the A matrix</param>
  /// <param name="_b">Right hand side</param>
  /// <param name="width">Image width</param>
  /// <param name="height">Image height</param>
  /// <param name="img">Image</param>
  /// <param name="ids">IDs of each unknown pixel</param>
  /// <param name="n">Number of unknown pixels</param>
  static void init(void* _coefs, void* b, int width, int height, float* img,
                   int* ids, int n);
};

#endif  // !MATRICE_SOLVE
