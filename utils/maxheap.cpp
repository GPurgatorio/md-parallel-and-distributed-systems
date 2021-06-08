/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @author Giulio Purgatorio 516292 \n( <mailto:g.purgatorio@studenti.unipi.it> )
* @brief MaxHeap class used to efficiently store the K nearest neighbors.
* The main idea is to save K elements only, with the root as the most distant node.
* When a new node is computed, it is compared to the root and if it's higher
* it can be discarded, otherwise it takes the root's place and then heapify()
*/

#include <vector>

using namespace std;

class MaxHeap {

    int size;
    vector<pair<int, float>> *arr;

public:

    MaxHeap(int size, vector<pair<int, float>> &input);

    void heapify(int i);

    // Build the max heap, by calling heapify for all non-leaf nodes.
    void buildHeap();

    // Tries to insert the given element into the maxheap
    void insertElem(pair<int, float> e);
};


// Constructor
MaxHeap::MaxHeap(int size, vector<pair<int, float>> &input) {
    // Initializing arr and size
    this->size = size;
    this->arr = &input;

    // Building the Max Heap
    buildHeap();
}


// Max Heapify function
void MaxHeap::heapify(int i) {
    // If Leaf Node, Simply return
    if (i >= size / 2)
        return;

    // variable to store the smallest (i, left, right)
    float smallest;

    // Index of left node
    int left = 2 * i + 1;

    // Index of right node
    int right = 2 * i + 2;

    // Store minimum from left node and current node i
    smallest = (*arr)[left].second > (*arr)[i].second ? left : i;

    // If right child exist, compare and update
    if (right < size)
        smallest = (*arr)[right].second > (*arr)[smallest].second
                             ? right : smallest;

    // If Node i violates the max heap property, swap current node i with
    // smallest and recursively call heapify for node smallest.
    if (smallest != i) {
        swap((*arr)[i], (*arr)[smallest]);
        heapify(smallest);
    }
}


// Build Max Heap
void MaxHeap::buildHeap() {
    // Calling Heapify for all non leaf nodes
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapify(i);
    }
}

// Tries to insert an elem by comparing it to the root
void MaxHeap::insertElem(pair<int, float> elem) {
  if (elem.second < (*arr)[0].second) {
    (*arr)[0] = elem;
    heapify(0);
  }
}
