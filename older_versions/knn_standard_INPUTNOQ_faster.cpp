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

// The function used by the reader thread to populate data
void reader_thread(vector<pair<float, float>> &data, char* name) {
  {
    utimer reader("Reading");
    data = ReadFile(name);
  }
}


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

      MaxHeap maxheap(k_param, neighbors);

      // Calculate the distance with every point
      for(int j=stop; j < data.size(); j++) {
        if(j==index) continue;    // (except itself)
        maxheap.insertElem(move(make_pair(j, euclidean_distance(data[index], data[j]))));
      }

      // Sort the list in ascending order on the second element (the euclidean distance)
      //partial_sort(neighbors.begin(), neighbors.begin()+k_param, neighbors.end(), distance_ordering);
      sort_heap(neighbors.begin(), neighbors.end(), distance_ordering);

      myqueue->push(to_string(index) + ": { " + printNeighbors(neighbors, k_param) + " }\n");
    }
    // Notify termination to print_worker
    myqueue->push("");
  }
}


void print_task(vector<myqueue<string>*> &to_print, int nw) {

  string s = "";
  string task_to_write = "";

  ofstream outputfile;
  outputfile.open ("output_standard_INPUTNOQ_faster.txt");

  int round_robin = 0;    // Index of the queue to pop
  int prec_robin = 0;

  while(nw > 0) {
    task_to_write = to_print[round_robin]->pop();

    prec_robin = round_robin;
    round_robin = (round_robin + 1) % nw;

    if(task_to_write == "") {
      to_print.erase(to_print.begin() + prec_robin);
      nw--;
      if(round_robin == nw) {
        round_robin = 0;
      }
    }
    else
      s.append(task_to_write);
  }
  outputfile << s << endl;
  outputfile.close();
}


// Main
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

  // Populate the "data" vector
  thread reader(reader_thread, ref(data), name);
  reader.join();

  int size = data.size();

  // Start filling the queue with tasks
  {
    utimer knn("KNN");

    // Start a number of workers that matches the requested "nw"
    for(int i=0; i < nw; ++i) {
      to_print[i] = new myqueue<string>();
      threads.push_back(move(thread([&data, k_param, &to_print, nw, i, size]() {
        job_task(data, k_param, to_print, nw, i, size);
      })));
      /*
      cpu_set_t cpuset;
      CPU_ZERO(&cpuset);
      // Assuming on PHI
      CPU_SET(i%256, &cpuset);
      int rc = pthread_setaffinity_np(threads[i].native_handle(), sizeof(cpu_set_t), &cpuset);
      if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
      }
      */
    }

    //thread writer(print_task, ref(to_print), nw);
    threads.push_back(move(thread([&to_print, nw]() {
      print_task(to_print, nw);
    })));

    // Wait for all threads to terminate
    for (auto &t: threads) {
      t.join();
    }
  }
  return 0;
}
