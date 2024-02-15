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

#include "MatriceSolve.h"

#include <Eigen/Core>
#include <Eigen/Sparse>

void Matrices::insertValues(int width, int height, int id, int nw, int nh,
                            int& neighCnt, void* _coefs, void* _b, float* img,
                            int* ids) {
  std::vector<Eigen::Triplet<double>>* coefs =
      (std::vector<Eigen::Triplet<double>>*)_coefs;
  Eigen::VectorXd* b = (Eigen::VectorXd*)_b;
  int nid = nw + nh * width;
  // Neighbour coordinate is out of bounds or the neighbout is not valid for
  // computation
  if (nw < 0 || nh < 0 || nw == width || nh == height ||
      img[nw + nh * width] == -1.0f) {
    // Number of neighbours is decreased
    neighCnt -= 1;
    return;
  }
  // Neighbour is a boundary condition
  if (img[nid] != 0.5f) {
    // right hand side is modified to contain the boundary condition
    (*b)(id) -= img[nid];
    return;
  }
  // Neighbour is unknown and is marked in the A matrix
  (*coefs).push_back(Eigen::Triplet<double>(id, ids[nid], 1));
}

void Matrices::init(void* _coefs, void* _b, int width, int height, float* img,
                    int* ids, int n) {
  std::vector<Eigen::Triplet<double>>* coefs =
      (std::vector<Eigen::Triplet<double>>*)_coefs;
  Eigen::VectorXd* b = (Eigen::VectorXd*)_b;

  // prepare square matrix and vector size n
  // as well as ids for unknown pixels, because known pixels can be scattered
  // through the image and we want to avoid adding them to the matrix

  // img contains -1, 0, 0.5 and 1
  // 1 and 0 are known, 0.5 unknown
  // -1 is skipped and is excluded from computation

  (*b).resize(n);
  for (int i = 0; i < n; i++) (*b)(i) = 0;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      if (img[w + h * width] != 0.5f) {
        continue;
      }
      int id = ids[w + width * h];
      int diag = 4;

      insertValues(width, height, id, w - 1, h, diag, coefs, b, img, ids);
      insertValues(width, height, id, w + 1, h, diag, coefs, b, img, ids);
      insertValues(width, height, id, w, h - 1, diag, coefs, b, img, ids);
      insertValues(width, height, id, w, h + 1, diag, coefs, b, img, ids);

      (*coefs).push_back(Eigen::Triplet<double>(id, id, -diag));
    }
  }
}

void Matrices::solve(float* img, int width, int height, int* ids, int n) {
  // img contains -1, 0, 0.5 and 1
  // 1 and 0 are known, 0.5 unknown
  // -1 is skipped and is excluded from computation
  // ids are already detected and n is determined

  std::vector<Eigen::Triplet<double>> coefs;
  Eigen::VectorXd b;

  // initialize the input data
  init(&coefs, &b, width, height, img, ids, n);

  // prepare matrix
  Eigen::SparseMatrix<double> A(n, n);
  A.setFromTriplets(coefs.begin(), coefs.end());

  // solve the problem
  Eigen::SimplicialCholesky<Eigen::SparseMatrix<double>> chol(A);
  Eigen::VectorXd x = chol.solve(b);

  // Prepare return data
  for (int i = 0, k = 0; i < width * height; i++) {
    if (img[i] == 0.5f) {
      img[i] = x(k);
      k += 1;
    }
  }
}
