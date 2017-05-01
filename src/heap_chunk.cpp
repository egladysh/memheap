#include <memheap/heap_chunk.h>
#include <iostream>
#include <assert.h>
#include <memory>
#include <cstring>
#include <algorithm>

using namespace memheap;

namespace memheap
{
	/*
	 * free block
	 * ----------
	 * | 0          |
	 * | free_node |
	 * |   ...      |
	 * | block_size |
	 *
	 *
	 * busy block
	 * ----------
	 * |  size      |
	 * |   ...      |
	 * |    0       |
	 *
	 * min block size = sizeof(free_node) + alignof(free_node) + 2 * sizeof(msize)
	 */

	struct free_node
	{
		msize* start_;
		msize size_; //in msize units
		free_node* prev_;
		free_node* next_;

		explicit free_node(msize* start, msize sz) 
			:start_(start)
			,size_(sz)
			,prev_(nullptr)
			,next_(nullptr)
		{}
	};
}

namespace
{
	using memheap::msize;

	const static msize NODE_ALIGN = (alignof(free_node) == alignof(msize))? 0: alignof(free_node); //already aligning as msize
	const static msize MIN_BLOCK_SIZE_BYTES_t = (sizeof(free_node) + NODE_ALIGN + 2*sizeof(msize));
	const static msize MIN_BLOCK_SIZE = ((MIN_BLOCK_SIZE_BYTES_t % sizeof(msize))? MIN_BLOCK_SIZE_BYTES_t + 1: MIN_BLOCK_SIZE_BYTES_t)/sizeof(msize);
	const static msize MIN_BLOCK_SIZE_BYTES = MIN_BLOCK_SIZE * sizeof(msize);


	template <typename T> inline
		T* place_aligned(void* p, std::size_t sz, std::size_t a = alignof(T))
		{
			if (!std::align(a, sizeof(T), p, sz))
				return nullptr;
			return reinterpret_cast<T*>(p);
		}

	inline free_node* get_free_node(msize* b, msize n)
	{
		std::size_t sz = (n - 2)*sizeof(msize);
		return place_aligned<free_node>(b + 1, sz);
	}

	free_node* make_free_node(msize* p, msize n)
	{
		assert(n);

		*p = 0;
		std::size_t sz = sizeof(msize) + MIN_BLOCK_SIZE_BYTES;
		free_node* r = place_aligned<free_node>(p + 1, sz); //(n - 2)*sizeof(msize));

		assert(r);
		//make sure that we have enough space for the end marker
		//MIN_BLOCK_SIZE must ensure this condition
		assert(reinterpret_cast<char*>(r) + sizeof(free_node) < reinterpret_cast<char*>(p + n));

		p[n-1] = n; //put the size at the block's end

		return new(r) free_node(p, n);
	}


	msize log2(msize n) {
		if (n <= MIN_BLOCK_SIZE)
			return 0;

		n -= MIN_BLOCK_SIZE;

		msize r = 0;
		while (n >>= 1)
			++r;
		return r;
	}

	void remove_node(free_node*& head, free_node* n)
	{
		assert(n);
		if (!n->prev_) {
			head = n->next_;
			if (head)
				head->prev_ = nullptr;
			return;
		}

		assert(n->prev_->next_ == n);

		n->prev_->next_ = n->next_;
		if (n->next_)
			n->next_->prev_ = n->prev_;
		return;

	}
	void add_node(free_node*& head, free_node* n)
	{
		assert(n);
		if (!head) {
			assert(!n->next_ && !n->prev_);
			head = n;
			return;
		}

		n->next_ = head;
		head->prev_ = n;
		head = n;
		n->prev_ = nullptr;
	}
}


heap_chunk::heap_chunk(msize n)
	:allocated_space_(0)
{
	assert(n);

	size_ = std::max(n, MIN_BLOCK_SIZE_BYTES) / sizeof(msize) + 3;
	b_ = new msize[size_];

	//this will allocate physical memory as much as possible 
	for (msize i = 0; i < size_; i += MIN_BLOCK_SIZE*4) {
		b_[i] = 0;
	}
	//memset(b_, 0xcd, sizeof(msize)*size_);

	msize lnum = log2(size_);

	buckets_.resize(lnum+1, nullptr);

	buckets_[lnum] = make_free_node(b_, size_);
}

heap_chunk::~heap_chunk()
{
	delete[] b_;
}

void* heap_chunk::allocate(msize nb)
{
	if (!nb)
		return nullptr;

	msize nw = std::max(nb + msize(2*sizeof(msize)) //place for block markers
			,MIN_BLOCK_SIZE_BYTES);

	nw = (nw % sizeof(msize))? nw/sizeof(msize) + 1: nw/sizeof(msize);

	assert(size_ >= allocated_space_);
	if (nw > size_ - allocated_space_)
		return nullptr;

	if (nw > size_)
		return nullptr;

	msize ln = log2(nw);
	assert(ln < buckets_.size());

	free_node** head = buckets_.data() + ln;
	free_node** end = buckets_.data() + buckets_.size();

	free_node* fn = {};

	for(; head != end; ++head) {
		fn = *head;
		for (; fn; fn = fn->next_) {
			if (fn->size_ >= nw)
				break;
		}
		if (fn)
			break;
	}

	if (!fn)
		return nullptr;

	msize* buf = fn->start_;
	//should we split the free block or just use the whole thing
	msize rmnd = fn->size_ - nw;

	if (rmnd < MIN_BLOCK_SIZE) { //use the whole thing
		nw = fn->size_;
		remove_node(*head, fn);
	}
	else {
		remove_node(*head, fn);
		add_node(buckets_[log2(rmnd)], make_free_node(buf + nw, rmnd));
	}

	//mark the busy block
	assert(nw >= MIN_BLOCK_SIZE);
	*buf = nw;
	buf[nw-1] = 0;

	allocated_space_ += nw;

	return buf + 1;
}

void heap_chunk::free(void* p)
{
	msize* b = reinterpret_cast<msize*>(p) - 1;

	msize n = *b;
	assert(n);
	allocated_space_ -= n;
	assert(!b[n-1]); //must be 0


	//is space before free
	//

	if (b_ != b) { //not at the chunk start
		msize sz =  *(b - 1);

		if (sz) { //have a free block of just befor this one
			b = b - sz;
			n += sz;

			free_node* r = get_free_node(b, sz);
			assert(r->size_ == sz);
			remove_node(buckets_[log2(sz)], r);
		}
	}

	//now check the next block
	if (b + n < b_ + size_) {
		if (!*(b + n)) { //free block after...
			free_node* r = get_free_node(b + n, MIN_BLOCK_SIZE);
			assert( (b+n)[r->size_-1] == r->size_ );
			n += r->size_;
			remove_node(buckets_[log2(r->size_)], r);
		}
	}

	add_node(buckets_[log2(n)], make_free_node(b, n));
}

msize heap_chunk::get_free_space() const
{
	msize r = 0;
	for (auto v: buckets_) {
		for (; v; v = v->next_) {
			r += (v->size_ * sizeof(msize));
		}
	}
	return r;
}

msize heap_chunk::get_min_alloc_size()
{
	return MIN_BLOCK_SIZE_BYTES;
}

