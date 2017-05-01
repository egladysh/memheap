#include <memheap/memheap.h>
#include <stdexcept>
#include <algorithm>
#include <new>
#include <limits>
#include <assert.h>

using namespace memheap;

namespace 
{
	const msize CHUNK_NUMBER = 8; //starting number of heap chunks

	struct scoped_lock
	{
		scoped_lock(std::mutex*m)
			:m_(m)
		{
			if (m_)
				m_->lock();
		}
		~scoped_lock()
		{
			if (m_)
				m_->unlock();
		}

	private:
		std::mutex* m_;

		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;
	};
}

heap::heap(bool thread_safe, msize est_max_size, msize est_cnt)
	:cur_heap_(nullptr)
	 ,mtx_(nullptr)
{
	assert(est_max_size && est_cnt);

	if (est_max_size < heap_chunk::get_min_alloc_size()) {
		est_max_size = heap_chunk::get_min_alloc_size();
	}
	else {
		est_max_size += 2*sizeof(msize);
	}

	msize chunkcnt = est_cnt < CHUNK_NUMBER? est_cnt: CHUNK_NUMBER;

	chunk_size_ = (est_max_size * est_cnt) / chunkcnt;


	/*
	   chunk_size_ = est_max_size * (est_cnt < 1024? est_cnt : 1024);
	   msize chunkcnt = (est_max_size * est_cnt + chunk_size_) / chunk_size_;
	   if (!chunkcnt)
	   chunkcnt = 1;
	   */

	//estimated total memory
	if (chunk_size_ < est_max_size)
		chunk_size_ = est_max_size;

	for (msize i = 0; i != chunkcnt; ++i) {
		heap_chunk* ph = new heap_chunk(chunk_size_);
		if (i == 0) 
			cur_heap_ = ph;
		hs_.push_back(ph);
	}
	std::sort(hs_.begin(), hs_.end(), [](const chunks::value_type& v1, const chunks::value_type& v2) -> bool { return v1->get_range().end_ < v2->get_range().end_; } );

	if (thread_safe) {
		mtx_ = new std::mutex;
	}
}

heap::~heap()
{
	if (mtx_) {
		delete mtx_;
	}

	for (auto v: hs_) {
		delete v;
	}
}

void* heap::do_allocate(msize n)
{
	void *pr = cur_heap_->allocate(n);
	if (!pr) {
		for (auto v: hs_) { //look for any chunk that works
			if (v == cur_heap_)
				continue;
			pr = v->allocate(n);
			if (pr) {
				cur_heap_ = v;
				return pr;
			}
		}
		//create a new heap
		if (n < chunk_size_) {
			cur_heap_ = new heap_chunk(chunk_size_);
			hs_.push_back(cur_heap_);
		}
		else { //big size
			cur_heap_ = new heap_chunk(n * 2);
			hs_.push_back(cur_heap_);
		}
		std::sort(hs_.begin(), hs_.end(), [](const chunks::value_type& v1, const chunks::value_type& v2) -> bool { return v1->get_range().end_ < v2->get_range().end_; } );

		pr = cur_heap_->allocate(n);
		if (!pr) {
			throw std::bad_alloc();
		}
	}
	return pr;
}

void* heap::allocate(msize n)
{
	if (!n)
		return nullptr;

	scoped_lock lk{mtx_};

	return do_allocate(n);
}

void heap::free(void* p)
{
	if (!p)
		return;

	scoped_lock lk{mtx_};

	assert(cur_heap_);

	//find chunk
	heap_chunk::range r = cur_heap_->get_range();
	if (r.start_ <= p && r.end_ > p) {
		cur_heap_->free(p);
		return;
	}


	assert(!hs_.empty());


	//
	auto it = std::upper_bound(hs_.begin(), hs_.end(), p, [](void* pb, const chunks::value_type& v)->bool 
			{
				heap_chunk::range hr = v->get_range();
				if (hr.end_ > pb) 
					return true;
				return false;
			} );
	assert(it != hs_.end());

	assert((*it)->get_range().start_ <= p && (*it)->get_range().end_ > p);
	cur_heap_ = (*it);
	cur_heap_->free(p);
	

	/*
	for (auto v: hs_) {
		if (cur_heap_ == v)
			continue;
		r = v->get_range();
		if (r.start_ <= p && r.end_ > p) {
			v->free(p);
			cur_heap_ = v;
			return;
		}
	}

	assert(false);
	*/
}

msize heap::get_free_space() const
{
	scoped_lock lk{mtx_};

	msize r = 0;
	for (auto v: hs_) {
		r += v->get_free_space();
	}
	return r;
}

