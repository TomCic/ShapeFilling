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

#ifndef DEPTH__
#define DEPTH__

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
#include "TopologicalSorting.h"

/// <summary>
/// Class used for representation and computation of relative depth levels
/// between segments.
/// </summary>
class Depth {
 public:
  std::list<Coordinates> graphicData;
  std::vector<TopologicalSorting::Node*> nodes;
  std::set<BYTE> startingNodes;
  std::list<BYTE> order;

  /// <summary>
  /// Calls init
  /// </summary>
  /// <param name="count">Number of scribbles of two types</param>
  Depth(const int* count) { init(count); }

  /// <summary>
  /// Destructor
  /// </summary>
  ~Depth() {
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i] != nullptr) delete (nodes[i]);
    }
  }

  /// <summary>
  /// Initializes the depth computation setup.
  /// </summary>
  /// <param name="count">Number of scribbles of two types</param>
  void init(const int* count) {
    // set nodes container to the right size and fill it with empty space
    nodes.resize(256);
    std::fill(nodes.begin(), nodes.end(), nullptr);

    // prepare starting nodes with count of all possible areas for soft and hard
    // scribbles
    for (int i = 0; i < count[0]; i++) {
      startingNodes.insert(i);
      nodes[i] = new TopologicalSorting::Node();
    }
    for (int i = 128; i < count[1] + 128; i++) {
      startingNodes.insert(i);
      nodes[i] = new TopologicalSorting::Node();
    }
  }

  /// <summary>
  /// Update depth structures with a new index
  /// </summary>
  /// <param name="idx">Node index</param>
  void update(int idx) {
    // index is a new starting node
    startingNodes.insert(idx);
    // the index occupies a new space in the container
    if (nodes[idx] == nullptr) nodes[idx] = new TopologicalSorting::Node();
  }

  /// <summary>
  /// Clear space used by nodes.
  /// </summary>
  void cleanup() {
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i] != nullptr) delete (nodes[i]);
    }
  }

  /// <summary>
  /// Clear space and initialize it again.
  /// </summary>
  /// <param name="count"></param>
  void reset(const int* count) {
    cleanup();
    order.clear();
    startingNodes.clear();
    graphicData.clear();
    nodes.clear();
    init(count);
  }

  /// <summary>
  /// Reset the graph structures for repeatable usage.
  /// </summary>
  void computationReset() {
    // All nonempty space occupied by nodes is to be marked as unused
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->edgesUsed = false;
        nodes[i]->incomingEdges = 0;
        startingNodes.insert(i);
      }
    }
    // outcoming edges from a node set the other nodes as non-starting ones
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i] != nullptr) {
        for (TopologicalSorting::Edge e : nodes[i]->edgesOut) {
          // increase the number of incoming edges of the second node
          nodes[e.to]->incomingEdges += 1;
          // if necessary, remove it from the starting nodes
          if (nodes[e.to]->incomingEdges == 1) {
            auto iter =
                std::find(startingNodes.begin(), startingNodes.end(), e.to);
            if (iter != startingNodes.end()) {
              startingNodes.erase(iter);
            }
          }
        }
      }
    }
  }

  /// <summary>
  /// Function used for update of the depth order structures.
  /// </summary>
  /// <param name="c_map"> Color map</param>
  /// <param name="from"> Index of the region in the back</param>
  /// <param name="to"> Index of the region in the front</param>
  /// <param name="coords"> From and to coordinates</param>
  /// <param name="flags"> Flags deciding whether connected segments are
  /// mergeable or not</param>
  bool addEdge(ColorMap& c_map, BYTE from, BYTE to, vec2<int>* coords,
               BYTE flag) {
    // background is in the back and its scribble is accounted for as 0
    // this is solely for unwanted useage
    if (from == to || from == 0 || to == 0) return false;
    if (nodes[from] == nullptr) nodes[from] = new TopologicalSorting::Node();
    if (nodes[to] == nullptr) nodes[to] = new TopologicalSorting::Node();
    // avoid computation for already added cases, but relocate them
    for (TopologicalSorting::Edge& e : nodes[from]->edgesOut) {
      if (e.to == to) {
        e.type = flag;
        for (std::list<Coordinates>::iterator it = graphicData.begin();
             it != graphicData.end(); it++) {
          if (c_map.getMaskAt((*it).x1 & 1023, (*it).y1) == from &&
              c_map.getMaskAt((*it).x2, (*it).y2) == to) {
            (*it) = {coords[0].x | (1024 * flag), coords[0].y, coords[1].x,
                     coords[1].y};
            break;
          }
        }
        return true;
      }
    }

    // update data structures
    // add edge to the first node
    nodes[from]->edgesOut.push_back({from, to, flag});
    // increase number of incoming edges in the second
    nodes[to]->incomingEdges++;
    // remove the second node from the starting nodes if possible
    auto iter = std::find(startingNodes.begin(), startingNodes.end(), to);
    bool removed = false;
    if (iter != startingNodes.end()) {
      removed = true;
      startingNodes.erase(iter);
    }

    std::list<BYTE> tmp;
    // find topological order
    if (!TopologicalSorting::topologicalSort_Kahn(nodes, startingNodes, tmp)) {
      nodes[from]->edgesOut.pop_back();
      nodes[to]->incomingEdges--;
      if (removed) {
        startingNodes.insert(to);
      }
      computationReset();
      return false;
    }
    computationReset();
    // update order and graphic data
    // flag in the first coordinate determines color
    graphicData.push_back(
        {coords[0].x | (1024 * flag), coords[0].y, coords[1].x, coords[1].y});
    order = tmp;
    return true;
  }

  /// <summary>
  /// Computes depths of all nodes
  /// </summary>
  void computeDepths() {
    for (int i : order) {
      for (TopologicalSorting::Edge e : nodes[i]->edgesOut) {
        nodes[e.to]->depth = std::max(nodes[i]->depth + 1, nodes[e.to]->depth);
      }
    }
  }

  /// <summary>
  /// Creates grayscale image representing depths of all areas in the picture
  /// </summary>
  /// <param name="c_map"> Color map</param>
  void printDepth(const ColorMap& c_map) {
    Depth::computeDepths();
    if (order.empty()) return;
    int maxD = nodes[order.back()]->depth;
    if (maxD == 0) return;
    int step = 200 / maxD;

    Image<float> img = Image<float>(c_map.getWidth(), c_map.getHeight());
    for (int h = 0; h < c_map.getHeight(); h++) {
      for (int w = 0; w < c_map.getWidth(); w++) {
        if (c_map.getMaskAt(w, h) == 0) {
          img(w, h) = 0;
          continue;
        }
        img(w, h) =
            (float)(50 + step * nodes[c_map.getMaskAt(w, h)]->depth) / 255.0f;
      }
    }

    imwrite<float>(img, "pictures/depth.png");
  }
};

#endif  // !DEPTH__
