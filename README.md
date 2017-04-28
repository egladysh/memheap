# memheap
memheap is a simple, general purpose memory allocator. It has single-thread and multi-thread modes. 
The only difference is that the multi-thread one is guarded with a mutex. The trivial API could be found in [include/memheap/memheap.h](https://github.com/egladysh/memheap/blob/master/include/memheap/memheap.h).
A standard std::allocator interface is provided in [include/memheap/allocator.h](https://github.com/egladysh/memheap/blob/master/include/memheap/allocator.h), that could be used with STL containers, etc..
Depending on your application, it could be much faster than standard malloc/free. 
See the benchmark section. For some allocation patterns, memheap is about 100 times faster. 

## Build

### Requirements
C++11 or newer

### Steps
* Suppose you have cloned the source to [HOME]/work/memheap
* For out-of source, create a build folder in [HOME]/work, and go there.

    $mkdir build

	$cd build


* Run cmake (make sure the build Release for speed).

	$cmake -DCMAKE_BUILD_TYPE=Release ../memheap/

* Or if you want some unit tests.

	$cmake -DCMAKE_BUILD_TYPE=Release -Dmemheap_build_tests=on ../memheap/


* Build it.     

    $make

* You can now run benchmarks.
    
	$./benchmark

* To install the library, do:

	$make install
	
## Benchmarks

For benchmarking [src/test/benchmark.cpp](https://github.com/egladysh/memheap/blob/master/test/benchmark.cpp): 
* we allocate thousands of arrays of random sizes from 0 to some limit bytes.
* then randomly delete about half of them, 
* then allocate random sized arrays again, 
* clear everything at the end

For some sizes the ratio of the standard malloc speed to the memheap speed could be as big as about 100 times.

The following benchmarks are for the following configuration.
* Apple LLVM version 8.1.0
* OSX 10.12.4
* 2.5 GHz Intel Core i7, 16Gb

### benchmark output

#### SINGLE-THREAD VERSION
Random allocations in ranges [0, 32] ... [0, 64] bytes, and 150000 allocations per test<br />
[malloc speed]/[memheap speed]=2.11074

Random allocations in ranges [0, 128] ... [0, 512] bytes, and 150000 allocations per test<br />
[malloc speed]/[memheap speed]=2.06717

Random allocations in ranges [0, 1024] ... [0, 10240] bytes, and 15000 allocations per test<br />
[malloc speed]/[memheap speed]=4.32719

Random allocations in ranges [0, 20480] ... [0, 262144] bytes, and 1500 allocations per test<br />
[malloc speed]/[memheap speed]=30.1054

Random allocations in ranges [0, 524288] ... [0, 1048576] bytes, and 1500 allocations per test<br />
[malloc speed]/[memheap speed]=93.1988


#### MULTI-THREAD VERSION
Random allocations in ranges [0, 32] ... [0, 64] bytes, and 150000 allocations per test<br />
[malloc speed]/[memheap speed]=1.37567

Random allocations in ranges [0, 128] ... [0, 512] bytes, and 150000 allocations per test<br />
[malloc speed]/[memheap speed]=1.8293

Random allocations in ranges [0, 1024] ... [0, 10240] bytes, and 15000 allocations per test<br />
[malloc speed]/[memheap speed]=3.73458

Random allocations in ranges [0, 20480] ... [0, 262144] bytes, and 1500 allocations per test<br />
[malloc speed]/[memheap speed]=28.8689

Random allocations in ranges [0, 524288] ... [0, 1048576] bytes, and 1500 allocations per test<br />
[malloc speed]/[memheap speed]=61.6003

