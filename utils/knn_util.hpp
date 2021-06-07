#ifndef KNNUTIL_H
#define KNNUTIL_H

#include <cmath>
#include <string>
#include <vector>

float euclidean_dist(float &x1, float &y1, float &x2, float &y2);

float euclidean_distance(std::pair<float,float> &p1, std::pair<float,float> &p2);

bool distance_ordering (std::pair<int, float> &a, std::pair<int,float> &b);

std::string printNeighbors(std::vector<std::pair<int,float>> &neighbors, int k_param);

#endif  //KNNUTIL_H
