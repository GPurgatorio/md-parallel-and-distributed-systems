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
  myqueue<optional<int>> *queue, vector<myqueue<string>*> &to_print, int queue_index) {
  {

    myqueue<string>* myqueue = to_print[queue_index];

    // Pull from queue the i-th element to elaborate
    auto task = queue->pop();
    int index;
    int count = 0;

    // Task is an optional<int>: in this case, it's either an INT or NULL
    while(task) {

      // Access INT value
      index = *task;

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

      //s.append("Neighbors' for " + to_string(index) + ": {" + printNeighbors(neighbors, k_param) + "}\n");
      myqueue->push(to_string(index) + ": { " + printNeighbors(neighbors, k_param) + " }\n");

      task = queue->pop();
    }
    // Notify termination to print_worker
    myqueue->push("");
  }
}


void print_task(vector<myqueue<string>*> &to_print, int nw) {

  string s = "";
  string task_to_write = "";     // = to_print.pop();

  ofstream outputfile;
  outputfile.open ("output_standard_faster.txt");

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
      continue;
    }
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
  myqueue<optional<int>> *tasks = new myqueue<optional<int>>();       // Queue used by workers to pull tasks
  vector<myqueue<string>*> to_print(nw);

  // Populate the "data" vector
  thread reader(reader_thread, ref(data), name);
  reader.join();

  // Start filling the queue with tasks
  {
    utimer knn("KNN");

    // Start a number of workers that matches the requested "nw"
    for(int i=0; i < nw; ++i) {

      to_print[i] = new myqueue<string>();

      threads.push_back(thread([&data, k_param, &tasks, &to_print, i]() {
        job_task(data, k_param, tasks, to_print, i);
      }));

    }

    thread writer(print_task, ref(to_print), nw);

    for(int i=0; i < data.size(); i++) {
      tasks->push({i});    // Each task is simply the index to be elaborated
    }

    // All tasks have been pushed, notify the end using our self-defined EOS
    for(int i=0; i < nw; ++i) {
      tasks->push(EOS);
    }

    writer.join();
    // Wait for all threads to terminate
    for (auto &t: threads) {
      t.join();
    }
  }
  return 0;
}
