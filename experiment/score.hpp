#include "graph.h"
#include <utility>
#include <string>
#include <fstream>
#include <sstream>

std::pair<size_t, size_t> score(const Graph &data, const Graph &query, const std::string &filename) {
  size_t N = query.GetNumVertices();
  std::ifstream fin(filename.c_str());
  std::pair<size_t, size_t> ret;
  while(fin.good()) {
    std::string buf;
    getline(fin, buf);
    std::istringstream sin(buf);
    char typ;
    sin >> typ;
    if(typ != 'a') continue;

    // get embedding information
    bool valid = true;
    std::vector<Vertex> embed(N);
    for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
      if(!sin.good()) {
        valid = false;
        break;
      }
      sin >> embed[u];
    }
    if(!valid) continue;
    
    for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
      if(query.GetLabel(u) != data.GetLabel(embed[u])) {
        valid = false;
        break;
      }
      size_t st = query.GetNeighborStartOffset(u);
      size_t en = query.GetNeighborEndOffset(u);
      for(size_t i = st; i < en; i++) {
        Vertex v = query.GetNeighbor(i);
        if(!data.IsNeighbor(embed[u], embed[v])) {
          valid = false;
          break;
        }
      }
      if(!valid)
        break;
    }
    if(valid) ret.first++;
    else ret.second++;
  }
  return ret;
}
