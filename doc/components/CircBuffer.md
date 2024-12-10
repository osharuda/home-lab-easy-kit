# <p align="center">Circular Buffer</p>
<p align="center"><img src="../images/hlek.svg"></p>

Circular buffer is one of the central components of the project. The key features of the circular buffer are these:
1. Basic circular buffer function
2. State prefix for reader.
3. Operation in block mode.
4. Circular buffer level warning check.
5. Overflow flag.
6. Lock-free (doesn't require critical section) for two asynchronous contexts (reader and writer).

<p align="center"><img src="../../doxygen/images/under_construction.png"></p>