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
  myqueue<optional<int>> &queue, myqueue<string> &to_print) {
  {
    // Pull from queue the i-th element to elaborate
    auto task = queue.pop();
    int index;

    // Task is an optional<int>: in this case, it's either an INT or NULL
    while(task) {

      // Access INT value
      index = *task;

      // Declare the list that will store neighbors for element INDEX
      vector<pair<int, float>> neighbors;

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

      to_print.push(to_string(index) + ": { " + printNeighbors(neighbors, k_param) + " }\n");

      task = queue.pop();
    }
    // Notify termination to print_worker
    to_print.push("");
  }
}


void print_task(myqueue<string> &to_print, int nw) {

  string res = "";
  ofstream outputfile;
  outputfile.open ("output_maxheap.txt");

  int counter = 0;
  while(counter != nw) {
    string task_to_write = to_print.pop();
    if(task_to_write == "")
      counter++;
    else
      res.append(task_to_write);
  }
  outputfile << res << endl;
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
  myqueue<optional<int>> tasks;       // Queue used by workers to pull tasks
  myqueue<string> to_print;

  // Populate the "data" vector
  thread reader(reader_thread, ref(data), name);
  reader.join();

  // Start filling the queue with tasks
  {
    utimer knn("KNN");

    // Start a number of workers that matches the requested "nw"
    for(int i=0; i < nw; ++i) {
      threads.push_back(thread([&data, k_param, &tasks, &to_print]() {
        job_task(data, k_param, tasks, to_print);
      }));
    }

    threads.push_back(thread([&to_print, nw]() {
      print_task(to_print, nw);
    }));

    for(int i=0; i < data.size(); i++) {
      tasks.push({i});    // Each task is simply the index to be elaborated
    }

    // All tasks have been pushed, notify the end using our self-defined EOS
    for(int i=0; i < nw; ++i) {
      tasks.push(EOS);
    }

    // Wait for all threads to terminate
    for (auto &t: threads) {
      t.join();
    }
  }
  return 0;
}
