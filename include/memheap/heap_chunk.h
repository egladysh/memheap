#ifndef H_124C417AC8CF4C1286389274D466EF2F 
#define H_124C417AC8CF4C1286389274D466EF2F

#include <cstddef>
#include <vector>

namespace memheap
{
	struct free_node;
    
    typedef unsigned long msize;


	struct heap_chunk
	{
		struct range
		{
			void* start_;
			void* end_;
		};

		explicit heap_chunk(msize n); //size in bytes
		~heap_chunk();

		void* allocate(msize n);
		void free(void* p);

        //
		msize get_free_space() const;
        
        //actuall memory would take to allocate less than get_min_alloc_size() bytes,
        //if the requested memory more than that, the overhead is 2*sizeof(msize)
		static msize get_min_alloc_size(); ////(~48 bytes on 64bit)

		range get_range() const
		{
			range r;
			r.start_ = b_;
			r.end_ = b_ + size_;
			return r;
		}
        
        msize get_allocated_space() const
        {
            return allocated_space_ * sizeof(msize);
        }
        
        msize get_total_size() const
        {
            return size_ * sizeof(msize);
        }

	private:
        msize allocated_space_;
		msize* b_; //make sure msize alignment
		msize size_; //buffer size in msize

		//free lists arranged in size by power of 2
		std::vector<free_node*> buckets_;  

		heap_chunk(const heap_chunk&) = delete;
		heap_chunk& operator=(const heap_chunk&) = delete;
	};
};

#endif

