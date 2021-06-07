/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @author Giulio Purgatorio 516292 \n( <mailto:g.purgatorio@studenti.unipi.it> )
* @brief Simple function to populate a vector from a file
*/

#include <fstream>
#include <iostream>
#include <string>
#include <optional>

#include "readfile.hpp"

using namespace std;

// Reads the given input file data, populating the "data" vector
vector<pair<float, float>> ReadFile(char *name) {
  ifstream input(name);
  if(!input) {
    cerr << "Couldn't open file " << name << "... Exiting." << endl;
    exit(-1);
  }
  float x, y;
  char comma;

  vector<pair<float, float>> data;
  // Input format:
  // x_coordinate,y_coordinate
  while ( input >> x >> comma >> y ) {
    data.push_back(std::make_pair(x, y));
  }

  input.close();
  if(data.size() < 1) {
    cerr << "File " << name << " seems empty... Exiting." << endl;
    exit(-2);
  }
  return data;
}
