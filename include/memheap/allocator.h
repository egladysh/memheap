#ifndef H_99970653A4FD4E81A2C5E14AD9F8CB39
#define H_99970653A4FD4E81A2C5E14AD9F8CB39

#include <memheap/memheap.h>
#include <memory>
#include <limits>

namespace memheap
{
	extern heap* g_std_heap;

	void init_std_allocator(bool thread_safe, msize est_max_size, msize est_cnt);
	void free_std_allocator();

    template <typename T, int N=sizeof(T)>
	struct allocator
	{
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;

		allocator() {}
		allocator(const allocator&) {}

		pointer allocate(size_type n, const void * = 0)
		{
			int offset = alignof(T) - 1 + sizeof(void*);
			void* p1 = g_std_heap->allocate(n*sizeof(T) + offset);
			void** p2 = (void**)(((size_t)(p1) + offset) & ~(alignof(T) - 1));
			*(p2-1) = p1;
			return (pointer)p2;
		}

		void deallocate(void* p, size_type) {
			g_std_heap->free(((void**)p)[-1]);
		}

		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }

		void construct(pointer p, const T& val) { new ((T*) p) T(val); }
		void destroy(pointer p) { p->~T(); }

		allocator&  operator=(const allocator&)
		{
			return *this;
		}


		size_type max_size() const { return std::numeric_limits<msize>::max(); }

		bool operator!=(const allocator&)
		{
			return false;
		}

		template <class U>
			struct rebind { typedef allocator<U> other; };

		template <class U>
			allocator(const allocator<U>&) {}

		template <class U>
			allocator& operator=(const allocator<U>&) { return *this; }
	};
    
    template <typename T>
	struct allocator<T, 1>
	{
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;

		allocator() {}
		allocator(const allocator&) {}

		pointer allocate(size_type n, const void * = 0)
		{
			return static_cast<pointer>(g_std_heap->allocate(n));
		}
		void deallocate(void* p, size_type) {
			g_std_heap->free(p);
		}

		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }

		allocator&  operator=(const allocator&)
		{
			return *this;
		}

		size_type max_size() const { return std::numeric_limits<msize>::max(); }

		//byte size alloctor, do nothing
		void construct(pointer p, const T& val) { *p = val; }
		void destroy(pointer p) {}

		bool operator!=(const allocator&)
		{
			return false;
		}

		template <class U>
			struct rebind { typedef allocator<U> other; };
	};
};

#endif
