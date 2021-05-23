/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>
#include <stdlib.h> // for exit(0)

Backtrack::Backtrack() {}
Backtrack::~Backtrack() {}

void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
  
  // little bit of initialization
  size_t N = query.GetNumVertices();
  size_t DN = data.GetNumVertices();
  std::cout << "t " << N << std::endl;

  /////
  // Step 1 : build DAG from query graph, with BFS
  /////
  
  // find root
  Vertex root = -1;
  double csize_deg_ratio = 1e100; // +infinity
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
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
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
    size_t st = query.GetNeighborStartOffset(u);
    size_t en = query.GetNeighborEndOffset(u);
    Label lu = query.GetLabel(u);
    for(size_t i = st; i < en; i++) {
      Vertex v = query.GetNeighbor(i);
      Label lv = query.GetLabel(v);

      bool is_forward = false;
      if(dist[u] != dist[v])
        is_forward = (dist[u] < dist[v]);
      else if(data.GetLabelFrequency(lu) != data.GetLabelFrequency(lv))
        is_forward = (data.GetLabelFrequency(lu) < data.GetLabelFrequency(lv));
      else if(query.GetDegree(u) != query.GetDegree(v))
        is_forward = (query.GetDegree(u) > query.GetDegree(v));
      else
        is_forward = (u < v);

      if(is_forward)
        edg[u].push_back(v);
      else
        redg[u].push_back(v);
    }
  }

  /////
  // Step 2 : perform backtracking
  /////

  // prepare for backtracking
  std::vector<std::vector<bool>> data_adj(DN, std::vector<bool>(DN));
  for(Vertex v = 0; v < static_cast<Vertex>(DN); v++) {
    size_t st = data.GetNeighborStartOffset(v);
    size_t en = data.GetNeighborEndOffset(v);
    for(size_t i = st; i < en; i++)
      data_adj[v][data.GetNeighbor(i)] = true;
  }
  
  std::vector<std::vector<Vertex>> i_cand(N);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
    i_cand[u].resize(cs.GetCandidateSize(u));
    for(size_t i = 0; i < i_cand[u].size(); i++)
      i_cand[u][i] = cs.GetCandidate(u, i);
    std::sort(i_cand[u].begin(), i_cand[u].end());
  }
  
  std::vector<std::vector<std::vector<std::vector<Vertex>>>> v_cand(N);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
    v_cand[u].resize(i_cand[u].size());
    for(size_t i = 0; i < v_cand[u].size(); i++) {
      Vertex t = i_cand[u][i];
      v_cand[u][i].resize(edg[u].size());
      for(size_t j = 0; j < v_cand[u][i].size(); j++) {
        Vertex w = edg[u][j];
        for(const auto v : i_cand[w])
          if(data_adj[t][v])
            v_cand[u][i][j].push_back(v);
        std::sort(v_cand[u][i][j].begin(), v_cand[u][i][j].end());
      }
    }
  }

  std::vector<size_t> indeg(N);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
    indeg[u] = redg[u].size();
  std::vector<std::vector<Vertex>> cand(N);
  std::vector<Vertex> extendable = {root}, embed(N, -1);
  std::vector<int> cand_inittime(N, -1);
  std::vector<bool> visited(DN);
  cand[root] = i_cand[root];
  size_t count = 0;

  // actual backtracking function
  const std::function<void(int)> btk = [&](int depth) {
    if(extendable.empty()) { // found a match
      std::cout << "a ";
      for(const auto &v : embed) std::cout << v << " ";
      std::cout << std::endl;
      count++;
      if(count == 100000) exit(0);
      return;
    }
   
    // select u (in query graph) for next match
    size_t cur_idx = 0;
    for(size_t i = 1; i < extendable.size(); i++) {
      if(cand[extendable[cur_idx]].size() > cand[extendable[i]].size()) 
        cur_idx = i;
    }
    Vertex cur = extendable[cur_idx];
    if(cand[cur].empty()) return; // there is no answers.

    // update extendable lists
    extendable.erase(extendable.begin() + cur_idx);
    size_t old_sz = extendable.size();
    for(const auto &v : edg[cur]) {
      indeg[v]--;
      if(indeg[v] == 0)
        extendable.push_back(v);
    }
    
    // iterate for each current candidates
    for(const auto &v : cand[cur]) {
      // check if already used
      if(visited[v]) continue;

      // set new embedding
      embed[cur] = v;
      visited[v] = true;

      // update candidate lists
      bool valid = true; // check if one of candidate set become empty
      size_t v_idx = size_t(
        lower_bound(i_cand[cur].begin(), i_cand[cur].end(), v)
        - i_cand[cur].begin()
      );
      auto &cur_v_cand = v_cand[cur][v_idx];
      std::vector<std::vector<Vertex>> old_cand(edg[cur].size());
      for(size_t j = 0; j < edg[cur].size(); j++) {
        Vertex w = edg[cur][j];
        if(cand_inittime[w] < 0) {
          cand_inittime[w] = depth;
          cand[w].swap(cur_v_cand[j]);
        }
        else {
          old_cand[j].swap(cand[w]);
          for(const Vertex t : old_cand[j]) {
            if(binary_search(cur_v_cand[j].begin(), cur_v_cand[j].end(), t))
              cand[w].push_back(t);
          }
        }
      }

      // recursive call
      if(valid) btk(depth + 1);

      // restore changes
      embed[cur] = -1;
      visited[v] = false;
      for(size_t j = 0; j < edg[cur].size(); j++) {
        Vertex w = edg[cur][j];
        if(cand_inittime[w] == depth) {
          cand_inittime[w] = -1;
          cand[w].swap(cur_v_cand[j]);
        }
        else
          cand[w].swap(old_cand[j]);
      }
    }

    // restore extendable lists
    extendable.resize(old_sz);
    extendable.insert(extendable.begin() + cur_idx, cur);
    for(const auto &v : edg[cur])
      indeg[v]++;
  };
  btk(0);
}
