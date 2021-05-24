#include "graph.h"
#include "common.h"
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <functional>
#include <cstdlib>

int main(int argc, char **argv) {
  std::string d_name(argv[1]);
  std::string q_name(argv[2]);
  std::string c_name(argv[3]);
  std::string query_size(argv[4]);
  size_t N = stoi(query_size);
  std::string min_avg_deg_s(argv[5]);
  std::string max_avg_deg_s(argv[6]);
  size_t min_M = static_cast<size_t>(stof(min_avg_deg_s) / 2 * N);
  size_t max_M = static_cast<size_t>(stof(max_avg_deg_s) / 2 * N);

  std::vector<Label> ori_label;
  std::ifstream fin(d_name.c_str());
  while(fin.good()) {
    char typ;
    fin >> typ;
    if(typ == 'e')
      break;
    Vertex v;
    Label l;
    fin >> v >> l;
    if(typ == 'v') 
      ori_label.push_back(l);
  }
  fin.close();
  std::sort(ori_label.begin(), ori_label.end());
  ori_label.erase(std::unique(ori_label.begin(), ori_label.end()), ori_label.end());

  Graph data(d_name);
  size_t DN = data.GetNumVertices();
  
  static std::mt19937 mt(
    std::chrono::high_resolution_clock::now()
    .time_since_epoch().count()
  );
  const auto rnd = [&](int s, int e) {
    return std::uniform_int_distribution<int>(s, e)(mt);
  };

  std::vector<Vertex> embed, inv_embed(DN, -1);
  Vertex cur = rnd(0, DN - 1);
  embed.push_back(cur);
  inv_embed[cur] = 0;
  while(embed.size() < N) {
    size_t st = data.GetNeighborStartOffset(cur);
    size_t en = data.GetNeighborEndOffset(cur);
    size_t i = rnd(st, en - 1);
    cur = data.GetNeighbor(i);
    if(inv_embed[cur] < 0) {
      inv_embed[cur] = 0;
      embed.push_back(cur);
    }
  }
  std::shuffle(embed.begin(), embed.end(), mt);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
    inv_embed[embed[u]] = u;

  std::vector<std::pair<Vertex, Vertex>> edges;
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
    Vertex v = embed[u];
    size_t st = data.GetNeighborStartOffset(v);
    size_t en = data.GetNeighborEndOffset(v);
    for(size_t i = st; i < en; i++) {
      Vertex w = data.GetNeighbor(i);
      if(v < w && inv_embed[w] >= 0) {
        edges.emplace_back(u, inv_embed[w]);
        if(edges.back().first > edges.back().second)
          std::swap(edges.back().first, edges.back().second);
      }
    }
  }
  
  if(edges.size() < (min_M + max_M) / 2) {
    main(argc, argv);
    return EXIT_SUCCESS;
  }
  max_M = std::min(max_M, edges.size());
  size_t M = rnd(min_M, max_M);

  std::shuffle(edges.begin(), edges.end(), mt);
  std::vector<int> parent(N);
  std::iota(parent.begin(), parent.end(), 0);
  std::vector<std::pair<Vertex, Vertex>> tree_edges, rem_edges;
  const std::function<int(int)> find_root = [&](int x) {
    if(x == parent[x])
      return x;
    return parent[x] = find_root(parent[x]);
  };
  for(const auto &p : edges) {
    if(find_root(p.first) != find_root(p.second)) {
      parent[find_root(p.first)] = find_root(p.second);
      tree_edges.push_back(p);
    }
    else
      rem_edges.push_back(p);
  }
  edges = tree_edges;
  std::shuffle(rem_edges.begin(), rem_edges.end(), mt);
  for(size_t i = 0; edges.size() < M; i++)
    edges.push_back(rem_edges[i]);
  std::sort(edges.begin(), edges.end());

  std::ofstream fout(q_name.c_str());
  fout << "t 0 " << N << std::endl;
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
    fout << "v " << u << ' ' << ori_label[data.GetLabel(embed[u])] << std::endl;
  for(const auto e : edges)
    fout << "e " << e.first << ' ' << e.second << " 0" << std::endl;
  fout.close();
  std::system(
    ("../executable/filter_vertices "
     + d_name + " " + q_name + " > " + c_name).c_str()
  );

  std::cout << std::setprecision(2) << std::fixed;
  std::cout << N << ' ' << edges.size() << " ("
    << static_cast<double>(2 * edges.size()) / N << ")" << std::endl;

  return EXIT_SUCCESS;
}
