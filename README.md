# concurrency-snippets
c++ code snippet for learning and recording about concurrency

## spin lock
Implementing a baisc spin lock by using std::atomic_flag and TAS instruction.
Implementing a TTAS(test test-and-set) spin lock that reducing cache coherent traffic, and providing better performance.