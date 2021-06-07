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

// Min Heap Class
// arr holds reference to an integer
// array size indicate the number of
// elements in Min Heap
class MaxHeap {

    int size;
    vector<pair<int, float>> *arr;

public:
    // Constructor to initialize the size and arr
    MaxHeap(int size, vector<pair<int, float>> &input);

    // Min Heapify function, that assumes that
    // 2*i+1 and 2*i+2 are min heap and fix the
    // heap property for i.
    void heapify(int i);

    // Build the min heap, by calling heapify
    // for all non-leaf nodes.
    void buildHeap();

    void insertElem(pair<int, float> e);
};

// Constructor to initialize data
// members and creating mean heap
MaxHeap::MaxHeap(int size, vector<pair<int, float>> &input)
{
    // Initializing arr and size

    this->size = size;
    this->arr = &input;

    // Building the Min Heap
    buildHeap();
}

// Min Heapify function, that assumes
// 2*i+1 and 2*i+2 are min heap and
// fix min heap property for i

void MaxHeap::heapify(int i)
{
    // If Leaf Node, Simply return
    if (i >= size / 2)
        return;

    // variable to store the smallest element
    // index out of i, 2*i+1 and 2*i+2
    float smallest;

    // Index of left node
    int left = 2 * i + 1;

    // Index of right node
    int right = 2 * i + 2;

    // Select minimum from left node and
    // current node i, and store the minimum
    // index in smallest variable
    smallest = (*arr)[left].second > (*arr)[i].second ? left : i;

    // If right child exist, compare and
    // update the smallest variable
    if (right < size)
        smallest = (*arr)[right].second > (*arr)[smallest].second
                             ? right : smallest;

    // If Node i violates the min heap
    // property, swap  current node i with
    // smallest to fix the min-heap property
    // and recursively call heapify for node smallest.
    if (smallest != i) {
        swap((*arr)[i], (*arr)[smallest]);
        heapify(smallest);
    }
}

// Build Min Heap
void MaxHeap::buildHeap()
{
    // Calling Heapify for all non leaf nodes
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapify(i);
    }
}

void MaxHeap::insertElem(pair<int, float> elem) {
  // Vai al contenuto di ARR e poi prendi il K-esimo elem
  if (elem.second < (*arr)[0].second) {
    (*arr)[0] = elem;
    heapify(0);
  }
}
