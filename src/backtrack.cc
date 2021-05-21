/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"
#include <vector>
#include <queue>
#include <stdlib.h> // for exit(0)

Backtrack::Backtrack() {}
Backtrack::~Backtrack() {}

void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
  
  // little bit of initialization
  size_t N = query.GetNumVertices();
  size_t M = query.GetNumEdges();
  std::cout << "t " << N << endl;

  /////
  // Step 1 : build DAG from query graph, with BFS
  /////
  
  // find root
  Vertex root = -1;
  double csize_deg_ratio = 1e100; // +infinity
  for(Vertex u = 0; u < N; u++) {
    size_t csize = cs.GetCandidateSize(u);
    size_t deg = query.GetDegree(u);
    double cur_ratio = static_cast<double>(csize) / deg;
    if(csize_deg_ratio > cur_ratio) {
      csize_deg_ratio = cur_ratio;
      root = u;
    }
  }

  // perform BFS
  std::vector<int> dist(N, -1);
  std::queue<Vertex> que;
  que.push(root);
  dist[root] = 0;
  while(!que.empty()) {
    Vertex cur = que.front();
    que.pop();
    size_t st = query.GetNeighborStartOffset(cur);
    size_t en = query.GetNeighborEndOffset(cur);
    for(size_t i = st; i < en; i++) {
      Vertex nxt = query.GetNeighbor(i);
      if(dist[nxt] < 0) {
        dist[nxt] = dist[cur] + 1;
        que.push(nxt);
      }
    }
  }

  // build DAG
  std::vector<std::vector<Vertex>> edg(N), redg(N);
  for(Vertex u = 0; u < N; u++) {
    size_t st = query.GetNeighborStartOffset(u);
    size_t en = query.GetNeighborEndOffset(u);
    for(size_t i = st; i < en; i++) {
      Vertex v = query.GetNeighbor(i);

      bool is_forward = false;
      if(dist[u] != dist[v])
        is_forward = (dist[u] < dist[v]);
      else if(query.GetLabel(u) != query.GetLabel(v))
        is_forward = (data.GetLabelFrequency(u) < data.GetLabelFrequency(v));
      else if(query.GetDegree(u) != query.GetDegree(v))
        is_forward = (query.GetDegree(u) > query.GetDegree(v));
      else
        is_forward = (u < v);

      if(is_forward)
        edg[u].push_back(v);
      else
        edg[v].push_back(u);
    }
  }

  /////
  // Step 2 : perform backtracking
  /////

  // prepare for backtracking
  std::vector<std::vector<Vertex>> cand(N);
  for(Vertex u = 0; u < N; u++) {
    cand[u].resize(cs.GetCandidateSize(u));
    for(size_t i = 0; i < cand[u].size(); i++) cand[u][i] = cs.GetCandidate(u, i);
  }
  std::vector<int> indeg(N);
  for(Vertex u = 0; u < N; u++) indeg[u] = redg[u].size();
  std::vector<int> extendable = {root};
  std::vector<Vertex> embed(N);
  size_t count = 0;

  const function<void()> btk = [&]() {
    if(extentable.empty()) { // found a match
      cout << "a ";
      for(const auto &v : embed) cout << v << " ";
      cout << endl;
      count++;
      if(count == 100000) exit(0);
      return;
    }
  };
  btk();
}
