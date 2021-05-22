#include "score.hpp"
#include "graph.h"
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if(argc < 4) {
    std::cerr << "Usage: ./score <data graph file> <query graph file>"
                 "<output file>" << std::endl;
    return EXIT_FAILURE;
  }
  
  std::string data_filename(argv[1]);
  std::string query_filename(argv[2]);
  std::string output_filename(argv[3]);

  Graph data(data_filename);
  Graph query(query_filename, true);
  
  const auto res = score(data, query, output_filename);
  
  std::cout << "Correct Embeddings   : " << res.first << std::endl;
  std::cout << "Incorrect Embeddings : " << res.second << std::endl; 

  return EXIT_SUCCESS;
}
