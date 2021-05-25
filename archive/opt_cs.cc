/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>
#include <limits>
#include <random>
#include <chrono>
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
  // Step 0 : calculate candidate set in different fashion : to get more information
  /////
  std::vector<std::vector<Vertex>> my_cand(N);
  {
    // init candidate set
    size_t L = data.GetNumLabels();
    std::vector<std::vector<Vertex>> label_list(L);
    for(Vertex v = 0; v < static_cast<Vertex>(DN); v++)
      label_list[data.GetLabel(v)].push_back(v);
    for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
      for(Vertex v : label_list[query.GetLabel(u)])
        if(data.GetDegree(v) >= query.GetDegree(u))
          my_cand[u].push_back(v);
    }

    // init dp table
    std::vector<std::vector<bool>> dp(N, std::vector<bool>(DN));
    for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
      for(Vertex v : my_cand[u])
        dp[u][v] = true;

    // init for random generation
    std::mt19937 mt(
      std::chrono::high_resolution_clock::now()
      .time_since_epoch().count()
    );
    const auto rnd = [&](int s, int e) {
      return std::uniform_int_distribution<int>(s, e)(mt);
    };

    for(int _ = 0; _ < 10; _++) { 
      // make random DAG
      std::vector<std::vector<Vertex>> edg(N);
      Vertex root = rnd(0, N - 1);
      std::vector<Vertex> extendable = {root}, top_order;
      std::vector<bool> pushed(N), popped(N);
      pushed[root] = true;
      while(!extendable.empty()) {
        size_t cur_idx = rnd(0, extendable.size() - 1);
        Vertex cur = extendable[cur_idx];
        popped[cur] = true;
        top_order.push_back(cur);
        extendable.erase(extendable.begin() + cur_idx);
        size_t st = query.GetNeighborStartOffset(cur);
        size_t en = query.GetNeighborEndOffset(cur);
        for(size_t i = st; i < en; i++) {
          Vertex v = query.GetNeighbor(i);
          if(popped[v])
            edg[v].push_back(cur);
          else if(!pushed[v]) {
            extendable.push_back(v);
            pushed[v] = true;
          }
        }
      }
      std::reverse(top_order.begin(), top_order.end());

      // calculte new candidate set
      std::vector<std::vector<Vertex>> new_cand(N);
      for(Vertex u : top_order) {
        for(Vertex v : my_cand[u]) {
          bool new_dp = true;
          for(Vertex c : edg[u]) {
            size_t st = data.GetNeighborStartOffset(v);
            size_t en = data.GetNeighborEndOffset(v);
            bool cur = false;
            for(size_t i = st; i < en; i++) {
              if(dp[c][data.GetNeighbor(i)]) {
                cur = true;
                break;
              }
            }
            if(!cur) {
              new_dp = false;
              break;
            }
          }
          dp[u][v] = new_dp;
          if(dp[u][v])
            new_cand[u].push_back(v);
        }
      }
      my_cand = new_cand;

      // further filtering
      const auto calc = [&](Graph &g, Vertex v, std::vector<size_t> deg_seq,
                            std::vector<std::pair<Label, size_t>> label_cnt_seq) {
        
      };
      for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
        new_cand[u].resize(0);
        std::vector<size_t> query_deg_seq;
        std::vector<std::pair<Label, size_t>> query_label_cnt_seq;
        calc(query, u, query_deg_seq, query_label_cnt_seq);
        size_t st = query.GetNeighborStartOffset(v);
        for(Vertex v : my_cand[u]) {
          std::vector<size_t> data_deg_seq;
          std::vector<std::pair<Label, size_t>> data_label_cnt_seq;
          calc(data, v, data_deg_seq, data_label_cnt_seq);
          bool valid = true;
          for(size_t i = 0; valid && i < query_deg_seq.size(); i++)
            if(query_deg_seq[i] > data_deg_seq[i])
              valid = false;
          for(size_t i = 0, j = 0; valid && i < query_label_cnt_seq.size(); i++) {
            while(j < data_label_cnt_seq.size()
                  && data_label_cnt_seq[j].first < query_label_cnt_seq[i].first)
              j++;
            if(j == data_label_cnt_seq.size()
               || query_label_cnt_seq[i].first != data_label_cnt_seq[i].first
               || query_label_cnt_seq[i].second > data_label_cnt_seq[i].second)
              valid = false;
          }
          if(valid) new_cand[u].push_back(v);
        }
      }
      my_cand = new_cand;
    }
  }

  /////
  // Step 1 : build DAG from query graph, with BFS
  /////
  
  // find root
  Vertex root = -1;
  double min_ratio = 1e100; // +infinity
  std::vector<double> cs_deg_ratio(N);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++) {
    size_t csize = cs.GetCandidateSize(u);
    size_t deg = query.GetDegree(u);
    cs_deg_ratio[u] = static_cast<double>(csize) / deg;
    if(min_ratio > cs_deg_ratio[u]) {
      min_ratio = cs_deg_ratio[u];
      root = u;
    }
  }

  // build DAG
  std::vector<std::vector<Vertex>> edg(N), redg(N);
  {
    std::vector<Vertex> extendable = {root};
    std::vector<bool> pushed(N), popped(N);
    pushed[root] = true;
    while(!extendable.empty()) {
      size_t cur_idx = 0;
      double min_ratio = cs_deg_ratio[extendable[0]];
      for(size_t i = 1; i < extendable.size(); i++) {
        if(min_ratio > cs_deg_ratio[extendable[i]]) {
          min_ratio = cs_deg_ratio[extendable[i]];
          cur_idx = i;
        }
      }
      Vertex cur = extendable[cur_idx];
      popped[cur] = true;
      extendable.erase(extendable.begin() + cur_idx);

      size_t st = query.GetNeighborStartOffset(cur);
      size_t en = query.GetNeighborEndOffset(cur);
      for(size_t i = st; i < en; i++) {
        Vertex v = query.GetNeighbor(i);
        if(popped[v]) {
          edg[v].push_back(cur);
          redg[cur].push_back(v);
        }
        else if(!pushed[v]) {
          extendable.push_back(v);
          pushed[v] = true;
        }
      }
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
    //cand[u].resize(cs.GetCandidateSize(u));
    std::cerr << cs.GetCandidateSize(u) << " vs " << my_cand[u].size() << std::endl;
    cand[u] = my_cand[u];
    for(size_t i = 0; i < cand[u].size(); i++) {
      //cand[u][i] = cs.GetCandidate(u, i);
      inv_cand[cand[u][i]].push_back(u);
    }
    sort(cand[u].begin(), cand[u].end());
  }
  auto ini_cand = cand;

  std::vector<std::vector<size_t>> weight(N);
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
    weight[u].resize(cand[u].size());
  const std::function<size_t(Vertex, size_t)> get_weight = [&](Vertex u, size_t i) {
    if(weight[u][i] > 0)
      return weight[u][i];
    bool has_proper_child = false;
    for(Vertex w : edg[u]) {
      if(redg[w].size() == 1) {
        has_proper_child = true;
        break;
      }
    }
    if(!has_proper_child)
      return weight[u][i] = 1;
    weight[u][i] = std::numeric_limits<size_t>::max();
    for(Vertex w : edg[u]) {
      if(redg[w].size() > 1) continue;
      size_t cur_weight = 0;
      for(size_t j = 0; j < cand[w].size(); j++)
        if(data_adj[cand[u][i]][cand[w][j]])
          cur_weight += get_weight(w, j);
      weight[u][i] = std::min(weight[u][i], cur_weight);
    }
    return weight[u][i];
  };
  for(Vertex u = 0; u < static_cast<Vertex>(N); u++)
    for(size_t i = 0; i < weight[u].size(); i++)
      weight[u][i] = get_weight(u, i);

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
    size_t min_weight = std::numeric_limits<size_t>::max();
    for(size_t i = 0; i < extendable.size(); i++) {
      Vertex u = extendable[i];
      size_t cur_weight = 0;
      for(Vertex v : cand[u]) {
        size_t idx = static_cast<size_t>(
          std::lower_bound(ini_cand[u].begin(), ini_cand[u].end(), v)
          - ini_cand[u].begin()
        );
        cur_weight += weight[u][idx];
      }
      if(min_weight > cur_weight) {
        min_weight = cur_weight;
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
