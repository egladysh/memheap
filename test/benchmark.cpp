#include <memheap/memheap.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <assert.h>
#include <functional>

namespace chrono=std::chrono;
using namespace memheap;
using namespace std::placeholders;

struct meminfo
{
	void* p_;
	msize size_;
	float rnd_; //random value
	msize rnd_size_;

	meminfo(msize sz)
		:p_{}
	,size_{sz}
	,rnd_{(float)std::rand()/(float)RAND_MAX}
	{}
};

struct memory
{
	std::function<void* (msize)> alloc_;
	std::function<void (void*)> free_;
};

unsigned long benchmark(msize max_alloc_size, std::vector<meminfo>& mi, memory mem)
{
	auto start = chrono::high_resolution_clock::now();

	//allocate all items
	for (auto& v: mi) {
		v.p_ = mem.alloc_(v.size_);
	}

	//do some random free/alloc
	//
	for (auto& v: mi) {
		if (v.rnd_ < 0.5) { //random free
			mem.free_(v.p_);
			v.p_ = nullptr;
		}
	}

	for (auto& v: mi) {
		if (!v.p_) { //allocate new items
			v.p_ = mem.alloc_(v.rnd_size_);
		}
	}

	for (auto& v: mi) { //free all items
		mem.free_(v.p_);
	}

	unsigned long dtm = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count();

	return dtm;
}


void range_benchmark(msize from, msize to, msize alloc_num, bool thread_safe)
{
	std::cout << "Random allocations in ranges [0, " << from << "] ... [0, " << to << "] bytes, and " << alloc_num + alloc_num/2 << " allocations per test" << std::endl;

	float diff{};

	msize lcnt = 0;
	for (msize i = from; i <= to; i *= 2, ++lcnt) {

		msize MAX_ALLOC_SIZE = i;
		msize MAX_ITEMS = alloc_num;

		std::vector<meminfo> mi;

		// init items with random sizes [1...MAX_ALLOC_SIZE]

		float k = ((float)MAX_ALLOC_SIZE) / (float)RAND_MAX;
		for (msize i = 0; i != MAX_ITEMS; ++i) {
			msize n =  static_cast<msize>((float)std::rand() * k);
			if (!n) n += 1;
			meminfo inf(n);
			inf.rnd_size_ = static_cast<msize>((float)std::rand() * k);
			if (!inf.rnd_size_)
				inf.rnd_size_ = 1;
			mi.push_back(inf);
		}

		float ft1{};
		{
			memory mem;
			mem.alloc_ = ::malloc;
			mem.free_ = ::free;

			ft1 = benchmark(MAX_ALLOC_SIZE, mi, mem);
		}

		float ft2{};
		{
			heap hc(thread_safe, MAX_ALLOC_SIZE, mi.size());

			memory mem;
			mem.alloc_ = std::bind(&heap::allocate, &hc, _1);
			mem.free_ = std::bind(&heap::free, &hc, _1);

			ft2 = benchmark(MAX_ALLOC_SIZE, mi, mem);
		}

		diff += ft1/ft2;
	}

	std::cout << "[malloc speed]/[memheap speed]=" << diff/(float)lcnt << std::endl;
}

int main()
{
	std::cout << "Running memheap benchmarks..." << std::endl;
	std::srand(std::time(0));


	std::cout << "SINGLE-THREAD VERSION" << std::endl;
	range_benchmark(32, 64, 100000, false);
	std::cout << std::endl;
	range_benchmark(128, 512, 100000, false);
	std::cout << std::endl;
	range_benchmark(1024, 10*1024, 10000, false);
	std::cout << std::endl;
	range_benchmark(20*1024, 256*1024, 1000, false);
	std::cout << std::endl;
	range_benchmark(512*1024, 1024*1024, 1000, false);
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << "MULTI-THREAD VERSION" << std::endl;
	range_benchmark(32, 64, 100000, true);
	std::cout << std::endl;
	range_benchmark(128, 512, 100000, true);
	std::cout << std::endl;
	range_benchmark(1024, 10*1024, 10000, true);
	std::cout << std::endl;
	range_benchmark(20*1024, 256*1024, 1000, true);
	std::cout << std::endl;
	range_benchmark(512*1024, 1024*1024, 1000, true);
	std::cout << std::endl;

	return 0;
}

