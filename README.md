# ParallelUT

A toy example of parallel processing of the elements of an upper triangular matrix.
A UT matrix of integer arrays is created, with each integer element initialized
to 0.  The processing increments each element.  Finally, each integer element is checked to
verify that it's 1.  If not, the indexing scheme for the threads is probably faulty.

To compile:  g++ -g checkParallelUT.cpp -pthread -std=c++11

To run:  ./a.out a b c, where
         a - matrix rows/columns
         b - length of integer vector associated with each
             unorder pair of indices
         c - number of simulated threads.  If c <= 0,
            use hardware_concurrency and attempt to use
            unsimulated threads.
Since we're using a triangular portion of the matrix,
there will only be a-1 actual rows/columns.

Good luck!
