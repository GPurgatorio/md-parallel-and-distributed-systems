/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @author Giulio Purgatorio 516292 \n( <mailto:g.purgatorio@studenti.unipi.it> )
* @brief This file is the FastFlow version of the KNN algorithm.
* Typical use:    ./knn_fastflow input.csv 10 4
*           to start 4 workers computing 10-NN on the input file
*/

#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <optional>
#include <fstream>
#include "utils/readfile.hpp"
#include "utils/knn_util.hpp"
#include "utils/utimer.cpp"
#include "utils/queue.cpp"
#include "utils/maxheap.cpp"

#include <ff/parallel_for.hpp>

using namespace std;


int main(int argc, char* argv[]) {

  if (argc < 4) {
    cout << "Usage: " + std::string (argv[0]) + " inputfile k_hyperparameter nw" << endl;
    return -1;
  }

  char *name = argv[1];               // Input file
  int k_param = atoi(argv[2]);        // Hyperparameter K
  int nw = atoi(argv[3]);             // Number of workers (if <= 0, all threads)

  vector<pair<float, float>> data;    // All points will be pushed here

  {
    utimer test("Reading input file");
    data = ReadFile(name);            // Populate the structure
  }

  ff::ParallelForReduce<string> pfr(nw);
  string res = "";

  {
    utimer KNN("KNN");
    pfr.parallel_reduce(res, "", 0, data.size(), 1, 0,
      [&](const long index, string &mystring) {
      vector<pair<int,float>> neighbors;
      int stop = (index < k_param) ? (k_param + 1) : k_param;

      // Insert K elems
      for(int j = 0; j < stop; j++) {
        if(index==j)
          continue;
        neighbors.push_back(move(make_pair(j,
          euclidean_distance(data[index], data[j])
        )));
      }

      MaxHeap maxheap(k_param, neighbors);

      // Calculate the distance with every point
      for(int j=stop; j < data.size(); j++) {
        if(j==index) continue;    // (except itself)
        maxheap.insertElem(move(make_pair(j, euclidean_distance(data[index], data[j]))));
      }

      // Sort the list in ascending order on the second element (the euclidean distance)
      //partial_sort(neighbors.begin(), neighbors.begin()+k_param, neighbors.end(), distance_ordering);
      sort_heap(neighbors.begin(), neighbors.end(), distance_ordering);

      mystring.append(to_string(index) + ": { " + printNeighbors(neighbors, k_param) + " }\n");
      },
      [](string &finalstring, const string e) {
        finalstring.append(e);
    });

    ofstream outputfile;
    outputfile.open ("output_ff.txt");
    outputfile << res;
    outputfile.close();
  }

  return 0;
}
