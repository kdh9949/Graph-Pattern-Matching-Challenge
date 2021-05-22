#include "graph.h"

int main(int argc, char **argv) {
  std::string filename(argv[1]);
  Graph g(filename);
  std::cout << g.GetNumVertices() << ' ' << g.GetNumEdges() << ' ' << g.GetNumLabels() << std::endl;
  return 0;
}
