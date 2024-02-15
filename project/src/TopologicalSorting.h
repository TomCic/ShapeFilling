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

#ifndef TOPOLOGICAL_SORT
#define TOPOLOGICAL_SORT

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <list>
#include <set>
#include <vector>

namespace TopologicalSorting {
/// <summary>
/// This structure takes pair of IDs of colored regions and saves them as an
/// oriented edge.
/// </summary>
struct Edge {
  BYTE from;
  BYTE to;
  BYTE type;
};

struct Node {
  std::list<Edge> edgesOut;  // edges outcoming from the node
  int incomingEdges;         // number of incoming edges
  int depth;                 // the depth assigned to the node
  bool edgesUsed;            // flag to detect usage

  Node() {
    incomingEdges = depth = 0;
    edgesUsed = false;
  }
};

// Graph G(V, Edge)
// L - empty set
// S - starting nodes

/// <summary>
/// Topological sorting method using the Kahn algorithm
/// </summary>
/// <param name="nodes">Graph nodes to be sorted</param>
/// <param name="startingNodes">Indices of the starting nodes</param>
/// <param name="sorted">Idices of the sorted nodes</param>
/// <returns>Success</returns>
inline bool topologicalSort_Kahn(std::vector<Node*>& nodes,
                                 std::set<BYTE>& startingNodes,
                                 std::list<BYTE>& sorted) {
  while (!startingNodes.empty()) {
    // select a node
    int current = *startingNodes.begin();
    startingNodes.erase(startingNodes.begin());
    sorted.push_back(current);
    if (!nodes[current]->edgesUsed) {
      // if it was not already used, go throught its outcoming edges
      std::list<Edge>::iterator it = nodes[current]->edgesOut.begin();
      for (size_t i = 0; i < nodes[current]->edgesOut.size(); i++) {
        // node that is pointed to by the current edge loses this connection
        nodes[(*it).to]->incomingEdges -= 1;
        // node has no incoming edges is a starting node
        if (nodes[(*it).to]->incomingEdges == 0) {
          startingNodes.insert((*it).to);
        }
        it++;
      }
      nodes[current]->edgesUsed = true;
    }
  }
  for (size_t i = 0; i < nodes.size(); i++) {
    // If there is a cycle, the nodes in the cycle will not be added to the
    // startingNodes list (set). This concludes in the fact that all of the
    // edges in the cycle would not be touched and will remain in the nodes.
    if (nodes[i] != nullptr && !nodes[i]->edgesUsed) {
      return false;
    }
  }
  return true;
}

}  // namespace TopologicalSorting

#endif  // !TOPOLOGICAL_SORT
