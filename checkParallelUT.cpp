//  A toy example of parallel processing of elements of a triangular matrix.
//  The comments implicitly refer to the association of integer vectors
//  with unordered pairs of integers.
//  To compile:  g++ checkParallelUT.cpp -pthread -std=c++11
//  MIT License Copyright (c) 2017 Jeffrey R. Begley

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <cmath>
#include <ctime>
#include <thread>
#include <stdlib.h>  // abort() (testing only?)

using namespace std;

class TriangularMatrixOfVectors {
 protected:
  int nrows, npairs, vlen;
  int **pairvecs;
  //
  int ut2dto1didx(int i1, int i2) {
  // Compute index into 1D matrix containing an upper triangular matrix.
  // If either index is not in the range 0..n-1, or if the indices are
  // equal, the return value is negative.  Otherwise, it's the 1D index.
   if (i1 == i2) return -128;  // This check might be redundant.
   int r, c;
   if(i1 < i2) {
    r = i1;
    c = i2;
   } else {
    r = i2;
    c = i1;
   }
   if(r >= c) {
    cerr << "Error computing triangular matrix 1D index." << endl;
    return -909;
   }
   if(r < 0) return -127;   // This check might be redundant.
   if(c >= npairs) return -255;  // This probably should never happen.
   // From a 2006 Ben Axelrod post on codeguru.com.
   return r*(nrows-1) - (r-1)*((r-1) + 1)/2 + c - r - 1;
  }  // ut2dto1didx()
 public:   // class TriangularMatrixOfVectors
  TriangularMatrixOfVectors(int nside, int vectlen) {
   nrows = nside;
   npairs = (nrows*(nrows-1))/2;
   vlen = vectlen;
   pairvecs = new int*[npairs];
   for(int i=0; i<npairs; ++i) {
    pairvecs[i] = new int[vlen];
    for(int j=0; j<vlen; ++j) pairvecs[i][j] = 0;
   }
  }  // constructor
  void processPair(int row, int col, int triangular1didx) {
   // This is a stub for testing.
   if(ut2dto1didx(row, col) != triangular1didx) {
    // We probably won't do this in the working program.
    cerr << "1D index error!!! *** " << triangular1didx << " should be "
         << ut2dto1didx(row, col) << "." << endl;
    abort();
   }
   for(int i=0; i<vlen; ++i) {
    // In this test, we expect each vector element to be 1 after all
    // processing is complete.
    ++(pairvecs[triangular1didx][i]);
   }
  }  // processPair
  int finalCheck(void) {
   // Just testing...
   cout << "Final check ..." << endl;
   for(int i=0; i<nrows; ++i) {
    for(int j=i+1; j<nrows; ++j) {
     int idx = ut2dto1didx(i, j);
     for(int k=0; k<vlen; ++k) {
      if(pairvecs[idx][k] != 1) {
       cerr <<  " Error at pair (" << i << ", " << j << ")   "
            << pairvecs[idx][k] << endl;
       return 1;
      }
     }
    }
   }
   cout << "Success!" << endl;
   return 0;
  }  // finalCheck
};  // class TriangularMatrixOfVectors

class ThreadedTriangularMatrix : public TriangularMatrixOfVectors {
 private:
  unsigned max_threads, nthreads;
  int thread_len;
 public:
  ThreadedTriangularMatrix(int nside, int vectlen, int nsimthreads) :
   TriangularMatrixOfVectors(nside, vectlen) {
   max_threads = std::thread::hardware_concurrency();
   cout << "Potential threads: " << max_threads << endl;
   if(nsimthreads <= 0) {
    nthreads = (max_threads>1)?(max_threads-1):1;
   } else {
    nthreads = nsimthreads;
   }
   cout << "Threads attempted in this run: " << nthreads << endl;
   thread_len = npairs / nthreads;
   // Make sure the threads finish the job.
   while((thread_len*nthreads) < npairs) ++thread_len;
   cout << "Maximum vectors/thread: " << thread_len << endl;
  }  //  ThreadedTriangularMatrix (3 param)

  void processRangeOfPairs(int start, int end) {
   // This process pairs in the triangular matrix.  The parameters are
   // 1D indices.
   // This is intended for parallel processing.
   int final = (npairs<end)?npairs:end;
   int rowlen = nrows-1;
   int firstrow = 0;
   int rowsum = 0;
   for(rowsum=rowlen; rowsum <= start; rowsum +=(--rowlen)) {
    ++firstrow;
   }
   cout << "1st row = " << firstrow << endl;
   cout << "  start, final " << start << " " << final << endl;
   int keepgoing = 1;
   for(int i=firstrow; (i<nrows)&&keepgoing; ++i) {
    for(int j=i+1; (j<nrows)&&keepgoing; ++j) {
     int pairidx = ut2dto1didx(i, j);
     if((pairidx >= start) && (keepgoing = (pairidx < final))) {
      processPair(i, j, pairidx);
     }
    }
   }
  }  // processRangeOfPairs

 void threadedProcessingOfAllPairs(void) {
  std::thread AllInThreads[nthreads];
  int fst = 0;
  int lst = thread_len;
  for(int i=0; i<nthreads; ++i) {
   AllInThreads[i] = 
    std::thread(&ThreadedTriangularMatrix::processRangeOfPairs, this, fst, lst);
   fst = lst;
   lst+=thread_len;
  }
  for (int i=0; i<nthreads; ++i) {
   AllInThreads[i].join();
  }
 }

 void loopToSimulateParallelProcessing(void) {
  int fst = 0;
  int lst = thread_len;
  for(int i=0; i<nthreads; ++i) {
   processRangeOfPairs(fst, lst);
   fst = lst;
   lst+=thread_len;
  }
 }

};  // class ThreadedTriangularMatrix


int main(int argc, char* argv[]) {
 const clock_t begin = clock();
 for(int i=0; i<argc; i++) {
  std::cout << argv[i] << std::endl;
 }
 if(argc < 3) {
  cout << "usage:  ./prog a b c, where" << endl;
  cout << "        a - matrix rows/columns" << endl;
  cout << "        b - length of integer vector associated with each" << endl;
  cout << "            unorder pair of indices" << endl;
  cout << "        c - number of simulated threads.  If c <= 0," << endl;
  cout << "            use hardware_concurrency and attempt to use" << endl;
  cout << "            unsimulated threads." << endl;
  cout << "Since we're using a triangular portion of the matrix," << endl;
  cout << "there will only be a-1 actual rows/columns." << endl;
  cout << "Good luck!" << endl;
  return 0;
 }
 int nitems = stoi(argv[1]);
 int vector_length = stoi(argv[2]);
 int nsimth = 0;
 if(argc > 3) {
  nsimth = stoi(argv[3]);
 }
 ThreadedTriangularMatrix m(nitems, vector_length, nsimth);
 if(nsimth > 0) {
  m.loopToSimulateParallelProcessing();
 } else {
  m.threadedProcessingOfAllPairs();
 }
 int chk = m.finalCheck();
 clock_t end = clock();
 cout << "Total elapsed time "
      << double(end - begin) / CLOCKS_PER_SEC << " seconds." << endl;
 return chk;
}
