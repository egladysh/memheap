#include <memheap/allocator.h>
#include <assert.h>

namespace memheap
{
	heap* g_std_heap = {nullptr};

	void init_std_allocator(bool thread_safe, msize est_max_size, msize est_cnt)
	{
		assert(!g_std_heap);
		g_std_heap = new heap(thread_safe, est_max_size, est_cnt);
	}

	void free_std_allocator()
	{
		if (!g_std_heap)
			return;
		delete g_std_heap;
		g_std_heap = nullptr;
	}
};

