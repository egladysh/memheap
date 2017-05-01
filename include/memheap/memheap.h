#ifndef H_4906712DD25245A5BB32FC5BAC4A9C25 
#define H_4906712DD25245A5BB32FC5BAC4A9C25

#include <vector>
#include <mutex>
#include <memheap/heap_chunk.h>
#include <memory>
#include <limits>

namespace memheap
{
	struct heap
	{

		explicit heap(bool thread_safe, msize est_max_size, msize est_cnt); //hint about estimated memory profile
		~heap();

		void* allocate(msize n);
		void free(void* p);
		msize get_free_space() const;

	private:
		msize chunk_size_;

		heap_chunk* cur_heap_;

		typedef std::vector<heap_chunk*> chunks;

		chunks hs_;
		std::mutex* mtx_;

		heap(const heap&) = delete;
		heap& operator=(const heap&) = delete;
        
        void* do_allocate(msize n);
	};
}

#endif

