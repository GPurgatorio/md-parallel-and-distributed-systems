CXX	= g++ -std=c++17
INCLUDES	=
CXXFLAGS	= -g

LDFLAGS	= -pthread # -fopenmp
OPTFLAGS	= -O3 -finline-functions -fopt-info-vec-optimized # -fopenmp-simd #-fno-tree-vectorize

OBJECTS	= utils/readfile.o utils/utimer.o utils/knn_util.o

TARGETS	= knn_fastflow knn_stl

.PHONY: all clean cleanall cleanoutputs
.SUFFIXES: .cpp

all	: $(TARGETS)

knn_fastflow: $(OBJECTS) knn_fastflow.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I ./ff $(OPTFLAGS) -o $@ $(OBJECTS) knn_fastflow.o $(LDFLAGS)

knn_stl: $(OBJECTS) knn_stl.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $(OBJECTS) knn_stl.o $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ -c $< $(LDFLAGS)

clean	:
	rm -f $(TARGETS)

cleanall	:	clean
	rm -f *.txt *~
	rm -f *.o *~
	rm -f utils/*.o

cleanoutputs	 :
	rm -f *.txt *~
