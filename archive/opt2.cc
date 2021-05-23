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

  std::vector<std::vector<Vertex>> cand(N), inv_cand(DN);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
    cand[u].resize(cs.GetCandidateSize(u));
    for(size_t i = 0; i < cand[u].size(); i++) {
      cand[u][i] = cs.GetCandidate(u, i);
      inv_cand[cand[u][i]].push_back(u);
    }
    sort(cand[u].begin(), cand[u].end());
  }

  std::vector<size_t> indeg(N);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
    indeg[u] = redg[u].size();
  
  std::vector<Vertex> extendable = {root};
  std::vector<Vertex> embed(N, -1);
  size_t count = 0;

  // actual backtracking function
  const std::function<void()> btk = [&]() {
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
    size_t min_sz = cand[extendable[0]].size();
    for(size_t i = 1; i < extendable.size(); i++) {
      size_t cur_sz = cand[extendable[i]].size();
      if(min_sz > cur_sz) {
        min_sz = cur_sz;
        cur_idx = i;
      }
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
      // set new embedding
      embed[cur] = v;

      // update candidate lists
      bool valid = true; // check if one of candidate set become empty
      std::vector<std::pair<Vertex, std::vector<Vertex>>> restore_list;
      for(const auto &w : edg[cur]) {
        std::vector<Vertex> remove_list;
        if(!cand[w].empty()) {
          for(size_t i = cand[w].size() - 1; i + 1 > 0; i--) {
            if(!data_adj[v][cand[w][i]]) {
              remove_list.push_back(cand[w][i]);
              cand[w].erase(cand[w].begin() + i);
            }
          }
        }
        restore_list.emplace_back(w, remove_list);
        if(cand[w].empty()) {
          valid = false;
          break;
        }
      }
      std::vector<Vertex> inv_restore_list;
      for(Vertex u : inv_cand[v]) {
        if(embed[u] >= 0) continue;
        const auto it = lower_bound(cand[u].begin(), cand[u].end(), v);
        if(it != cand[u].end() && *it == v) {
          cand[u].erase(it);
          inv_restore_list.push_back(u);
        }
      }

      // recursive call
      if(valid) btk();

      // restore changes
      embed[cur] = -1;
      for(const auto &p : restore_list) {
        cand[p.first].insert(cand[p.first].end(), p.second.begin(), p.second.end());
        sort(cand[p.first].begin(), cand[p.first].end());
      }
      for(Vertex u : inv_restore_list)
        cand[u].insert(lower_bound(cand[u].begin(), cand[u].end(), v), v);
    }

    // restore extendable lists
    extendable.resize(old_sz);
    extendable.insert(extendable.begin() + cur_idx, cur);
    for(const auto &v : edg[cur])
      indeg[v]++;
  };
  btk();
}
