/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @author Giulio Purgatorio 516292 \n( <mailto:g.purgatorio@studenti.unipi.it> )
* @brief Some utility functions used by the two versions of the KNN algorithm.
*/

#include "knn_util.hpp"

// Called by euclidean_distance(), actually calculates it
// NOTE: the sqrt(..) part is removed because of efficiency reasons!
float euclidean_dist(float &x1, float &y1, float &x2, float &y2) {
  return pow(x2 - x1, 2) + pow(y2 - y1, 2);
}

// Less verbose way of writing the euclidean_dist() function
float euclidean_distance(std::pair<float,float> &p1, std::pair<float,float> &p2) {
  return euclidean_dist(p1.first, p1.second, p2.first, p2.second);
}

// Used to order by the second element of a pair (in this project, it's the distance)
bool distance_ordering (std::pair<int, float> &a, std::pair<int,float> &b) {
  return a.second < b.second;
}

// Prints the first K neighbors in a vector, concatenating them with a ", "
std::string printNeighbors(std::vector<std::pair<int,float>> &neighbors, int k_param) {
  std::string res;
  for(int i=0; i < k_param - 1; i++) {
    res += std::to_string(neighbors[i].first) + ", ";
  }
  return (res += std::to_string(neighbors[k_param-1].first));
}
