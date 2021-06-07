/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @author Giulio Purgatorio 516292 \n( <mailto:g.purgatorio@studenti.unipi.it> )
* @brief This file is the STL version of the KNN algorithm.
* Typical use:    ./knn_stl input.csv 10 4
*           to start 4 workers (+1 writer) computing 10-NN on the input file
*/

#include <vector>
#include <algorithm>
#include <thread>
#include <optional>
#include <fstream>
#include "utils/readfile.hpp"
#include "utils/knn_util.hpp"
#include "utils/utimer.cpp"
#include "utils/queue.cpp"
#include "utils/maxheap.cpp"

using namespace std;


// The function used by the worker thread to elaborate tasks
void job_task(vector<pair<float, float>> &data, int k_param,
  vector<myqueue<string>*> &to_print, int nw, int thid, int size) {
  {
    myqueue<string>* myqueue = to_print[thid];

    for(int index = thid; index < size; index += nw) {

      // Declare the list that will store neighbors for element INDEX
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

      // Create a max heap from the previous vector
      MaxHeap maxheap(k_param, neighbors);

      // Calculate the distance with every point and try to insert it
      for(int j=stop; j < data.size(); j++) {
        if(j==index) continue;    // (except itself)
        maxheap.insertElem(move(make_pair(j, euclidean_distance(data[index], data[j]))));
      }

      // Sort the heap in ascending order (on the second element)
      sort_heap(neighbors.begin(), neighbors.end(), distance_ordering);

      myqueue->push(to_string(index) + ": { " + printNeighbors(neighbors, k_param) + " }\n");
    }
    // Notify termination to print_worker
    myqueue->push("");
  }
}


// The function used by the Writer thread
void print_task(vector<myqueue<string>*> &to_print, int nw) {

  string s = "";                // The accumulator string
  string task_to_write = "";    // The popped string

  ofstream outputfile;
  outputfile.open ("output_stl.txt");

  int round_robin = 0;    // Index of the queue to pop
  int prec_robin = 0;     // Index of the previos round

  // As long as there are some workers still running..
  while(nw > 0) {
    // Pop a result from the current queue
    task_to_write = to_print[round_robin]->pop();

    prec_robin = round_robin;
    round_robin = (round_robin + 1) % nw;

    // We assume "" == EOS in this case
    if(task_to_write == "") {
      // We received an EOS from the previous queue, erase it
      to_print.erase(to_print.begin() + prec_robin);
      // The number of workers decrements
      nw--;
      if(round_robin == nw) {
        round_robin = 0;
      }
    }
    else
      s.append(task_to_write);
  }
  // At the end, write to file
  outputfile << s << endl;
  outputfile.close();
}


int main(int argc, char* argv[]) {

  if (argc < 4) {
    cout << "Usage: " + std::string (argv[0]) + " inputfile k_hyperparameter nw" << endl;
    return -1;
  }

  char *name = argv[1];               // Input file
  int k_param = atoi(argv[2]);        // Hyperparameter K
  int nw = atoi(argv[3]);             // Number of workers

  vector<thread> threads;             // All workers will be pushed here
  vector<pair<float, float>> data;    // All points will be pushed here
  vector<myqueue<string>*> to_print(nw);

  {
    utimer test("Reading input file");
    data = ReadFile(name);            // Populate the structure
  }

  // Calculate the size once
  int size = data.size();

  {
    utimer knn("KNN");

    // Start a number of workers that matches the given "nw" parameter
    for(int i=0; i < nw; ++i) {

      // Each worker will have its own output queue
      to_print[i] = new myqueue<string>();

      threads.push_back(move(thread([&data, k_param, &to_print, nw, i, size]() {
        job_task(data, k_param, to_print, nw, i, size);
      })));

      // https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
      // Create a cpu_set_t object representing a set of CPUs. Clear it and mark
      // only CPU i as set.
      cpu_set_t cpuset;
      CPU_ZERO(&cpuset);
      CPU_SET(i, &cpuset);
      // Give the native handler a mask that tells on which core the thread should run
      int rc = pthread_setaffinity_np(threads[i].native_handle(), sizeof(cpu_set_t), &cpuset);
      if (rc != 0) {  // Of course, we'll be limited by Hardware and this can fail
        std::cerr << "Warning: thread " << i << " isn't set to a specific CPU: setaffinity returned " << rc << "\n";
      }
    }

    // We also create the writer so that he can start clearing queues
    threads.push_back(move(thread([&to_print, nw]() {
      print_task(to_print, nw);
    })));

    // Wait for all threads (workers + writer) to terminate
    for (auto &t: threads) {
      t.join();
    }
  }
  return 0;
}
