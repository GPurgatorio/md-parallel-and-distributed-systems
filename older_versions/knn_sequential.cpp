#include <vector>
#include <algorithm>
#include <fstream>
#include "utils/readfile.hpp"
#include "utils/knn_util.hpp"
#include "utils/utimer.cpp"
#include "utils/maxheap.cpp"

using namespace std;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Usage: " + std::string (argv[0]) + " inputfile k_hyperparameter" << endl;
    return -1;
  }

  char *name = argv[1];               // Input file name
  int k_param = atoi(argv[2]);        // K hyperparameter
  vector<pair<float, float>> data;    // The structure to populate by reading the input file

  {
    utimer test("Reading input file");
    data = ReadFile(name);            // Populate the structure
  }

  // Resulting string: all results will be appended here
  string res = "";

  {
    utimer knn("KNN");
    // For each point that was read
    for(int index=0; index < data.size(); index++) {

      // Create the structure where neighbors will be stored
      vector<pair<int,float>> neighbors;

      // The first K neighbors are inserted without checking
      // So we need to check if it's going to "insert" the node itself
      // And if so, we just iterate once more and skip that
      int stop = (index < k_param) ? (k_param + 1) : k_param;

      // Insert K elems
      for(int j = 0; j < stop; j++) {
        // Same index, skip it
        if(index==j)
          continue;

        // Insert the element in the vector
        neighbors.push_back(move(make_pair(j,
          euclidean_distance(data[index], data[j])
        )));
      }

      // Generate a MaxHeap from the previously created vector
      MaxHeap maxheap(k_param, neighbors);

      // Now try to insert all the other points, calculating their distances
      for(int j=stop; j < data.size(); j++) {
        if(j==index)        // (except itself)
          continue;
        maxheap.insertElem(move(make_pair(j,
          euclidean_distance(data[index], data[j]))));
      }

      // Sort the heap in ascending order (on the second element)
      sort_heap(neighbors.begin(), neighbors.end(), distance_ordering);

      res.append(to_string(index) + ": { " + printNeighbors(neighbors, k_param) + " }\n");
    }
  }

  {
    utimer writer("Writer");

    // Write to file
    ofstream outputfile;
    outputfile.open ("output_sequential.txt");
    outputfile << res;
    outputfile.close();
  }
  return 0;
}
