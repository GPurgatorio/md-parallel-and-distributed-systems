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

using namespace ff;


struct Emitter: ff_node_t<int> {
    Emitter(const size_t size):size(size) {}
    int* svc(int *) {
        for(size_t i=0; i<size; ++i) {
            ff_send_out(new int(i));
        }
        return EOS;
    }
    const size_t size;
};


struct Collector:ff_node_t<string> {
    Collector(ofstream *outputfile):outputfile(outputfile) {}
    string* svc(string* in) {
        (*outputfile) << *in;
        delete in;
        return GO_ON;
    }
    ofstream *outputfile;
};


struct Worker: ff_node_t<int, string> {
  Worker(vector<pair<float, float>> *data, int k):data(data), size(data->size()), k(k) {}
  string *svc(int *task) {
    int index = *task;
    vector<pair<int,float>> neighbors;
    int stop = (index < k) ? (k + 1) : k;
    // Insert K elems
    for(int j = 0; j < stop; j++) {
      if(index==j)
        continue;
      neighbors.push_back(move(make_pair(j,
        euclidean_distance(data->at(index), data->at(j))
      )));
    }

    MaxHeap maxheap(k, neighbors);

    // Calculate the distance with every point
    for(int j=stop; j < size; j++) {
      if(j==index) continue;    // (except itself)
      maxheap.insertElem(move(make_pair(j, euclidean_distance(data->at(index), data->at(j)))));
    }
    string *result = new string(to_string(index) + ": { " + printNeighbors(neighbors, k) + " }\n");
    delete task;
    return result;
  }

  vector<pair<float,float>> *data;
  const int size;
  const int k;
};


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

  Emitter emitter(data.size());

  ofstream outputfile("output_explicit.txt", ios::trunc);
  Collector collector(&outputfile);

  vector<ff_node*> workers;

  for(int i=0; i<nw; i++) {
    Worker *W = new Worker(&data, k_param);
    workers.push_back(W);
  }

  ff_farm farm(workers, emitter, collector);

  farm.cleanup_workers();
  farm.set_scheduling_ondemand(10);
  farm.blocking_mode(false);

  ffTime(START_TIME);
  if (farm.run_and_wait_end()<0) {
      error("running farm");
      return -1;
  }

  outputfile.close();

  ffTime(STOP_TIME);
  std::cout << "Time: " << ffTime(GET_TIME) << " ms\n";
  return 0;
}
