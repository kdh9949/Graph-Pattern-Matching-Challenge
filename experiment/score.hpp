#include "graph.h"
#include <algorithm>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>

std::pair<size_t, size_t> score(const Graph &data, const Graph &query, const std::string &filename) {
  size_t N = query.GetNumVertices();
  size_t DN = data.GetNumVertices();

  std::vector<std::vector<bool>> data_adj(DN, std::vector<bool>(DN));
  for(Vertex v = 0; v < static_cast<Vertex>(DN); v++) {
    size_t st = data.GetNeighborStartOffset(v);
    size_t en = data.GetNeighborEndOffset(v);
    for(size_t i = st; i < en; i++)
      data_adj[v][data.GetNeighbor(i)] = true;
  }

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
    
    auto embed_s = embed;
    std::sort(embed_s.begin(), embed_s.end());
    if(std::unique(embed_s.begin(), embed_s.end()) != embed_s.end())
      valid = false;

    for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
      if(query.GetLabel(u) != data.GetLabel(embed[u])) {
        valid = false;
        break;
      }
      size_t st = query.GetNeighborStartOffset(u);
      size_t en = query.GetNeighborEndOffset(u);
      for(size_t i = st; i < en; i++) {
        Vertex v = query.GetNeighbor(i);
        if(!data_adj[embed[u]][embed[v]]) {
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
