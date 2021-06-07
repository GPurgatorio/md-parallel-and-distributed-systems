/*
* SPM Project (2020-2021)
* Parallel and Distributed Systems: paradigms and models
* Master in Computer Science, Pisa
*
* Teachers: M. Danelutto, M. Torquati
*
* @brief This file was made during the course.
* It is a timer that is used to calculate the time needed by specific portions
* of code, by exploiting the RAII pattern.
*/

#include <iostream>
#include <chrono>


#define START(timename) auto timename = std::chrono::system_clock::now();
#define STOP(timename,elapsed)  auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - timename).count();


class utimer {
  std::chrono::system_clock::time_point start;
  std::chrono::system_clock::time_point stop;
  std::string message;
  using usecs = std::chrono::microseconds;
  using msecs = std::chrono::milliseconds;

private:
  long * us_elapsed;

public:

  utimer(const std::string m) : message(m),us_elapsed((long *)NULL) {
    start = std::chrono::system_clock::now();
  }

  utimer(const std::string m, long * us) : message(m),us_elapsed(us) {
    start = std::chrono::system_clock::now();
  }

  ~utimer() {
    stop =
      std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed =
      stop - start;
    auto musec =
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    std::cout << message << " computed in " << musec << " usec "
	      << std::endl;
    if(us_elapsed != NULL)
      (*us_elapsed) = musec;
  }
};
