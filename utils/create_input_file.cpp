/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @author Giulio Purgatorio 516292 \n( <mailto:g.purgatorio@studenti.unipi.it> )
* @brief This file was used to create the three main input files. Typical use:
*         ./create_input_file 10000 100 123 outputname.csv
*       To create a file with 10000 numbers from [0, 10000) with seed 123
*/

#include <iostream>
#include <fstream>

void WriteToFile(int len, int max, int seed, char* name) {

  // Set the seed
  srand(seed);

  // Create given filename
  std::ofstream file;
  file.open(name);

  // Generate @len points
  for( int i=0; i < len; i++ ) {
    std::pair <float, float> point;
    point = std::make_pair(
      (float)rand() / (float)(RAND_MAX/max),
      (float)rand() / (float)(RAND_MAX/max)
    );

    // and write them into the previous file
    file << point.first << "," << point.second << std::endl;
  }
  // Close the file
  file.close();
}


int main (int argc, char* argv[]) {

  if (argc != 5) {
    std::cout << "Usage: " + std::string (argv[0]) + " number_of_points maximum_value seed outputfile" << std::endl;
    return -1;
  }
  int len = atoi(argv[1]);
  int max = atoi(argv[2]);
  int seed = atoi(argv[3]);
  char* name = argv[4];
  WriteToFile(len, max, seed, name);
  return 0;
}
