#include <memheap/allocator.h>
#include <memory>
#include <gtest/gtest.h>
#include <vector>

using namespace memheap;

struct HeapTest : testing::Test
{
	std::unique_ptr<heap_chunk> hc_;
	std::unique_ptr<heap> h_;

	HeapTest()
	{
	}
};

TEST_F(HeapTest, TestLimits)
{
	hc_.reset(new heap_chunk(2));
	EXPECT_LE(hc_->get_min_alloc_size(), hc_->get_free_space());
    
	void *p = hc_->allocate(1);
    EXPECT_NE(nullptr, p);
	EXPECT_EQ(0, hc_->get_free_space());
	*(char*)p = 0;
    EXPECT_EQ(nullptr, hc_->allocate(1));
    
    hc_->free(p);
    EXPECT_LE(hc_->get_min_alloc_size(), hc_->get_free_space());
    EXPECT_NE(0, hc_->get_free_space());
    
    p = hc_->allocate(1);
    EXPECT_NE(nullptr, p);
    EXPECT_EQ(0, hc_->get_free_space());
    EXPECT_EQ(nullptr, hc_->allocate(1));
}

TEST_F(HeapTest, TestApi)
{
	const msize sz = 100*1024*1024;
	hc_.reset(new heap_chunk(sz));
    
    msize freesz = hc_->get_free_space();

	EXPECT_LE(sz, freesz);

	std::vector<void*> mem;

	for (msize i = 1; i < 10*1024; ++i)
	{
		void *p = hc_->allocate(i);
		memset(p, 0, i);
		EXPECT_NE(nullptr, p);
		mem.push_back(p);
	}

	for (msize i = 1; i < 10*1024; ++i)
	{
		if (i%2 == 0) {
			hc_->free(mem[i-1]);
		}
	}
    
    for (msize i = 1; i < 10*1024; ++i)
    {
        if (i%2 == 0) {
            void *p = hc_->allocate(2 * i);
			memset(p, 0, 2 * i);
            ASSERT_NE(nullptr, p);
            mem[i-1] = p;
        }
    }
    for (msize i = 1; i < 10*1024; ++i)
    {
        hc_->free(mem[i-1]);
    }
    
    EXPECT_EQ(freesz, hc_->get_free_space());
}

TEST_F(HeapTest, TestFullHeap)
{
	const msize sz = 1024;
	h_.reset(new heap(false, sz, 10*1024));
    

	std::vector<void*> mem;

	for (msize i = 1; i < 10*1024; ++i)
	{
		void *p = h_->allocate(i);
		memset(p, 0, i);
		EXPECT_NE(nullptr, p);
		mem.push_back(p);
	}

	for (msize i = 1; i < 10*1024; ++i)
	{
		if (i%2 == 0) {
			h_->free(mem[i-1]);
		}
	}
    
    for (msize i = 1; i < 10*1024; ++i)
    {
        if (i%2 == 0) {
            void *p = h_->allocate(2 * i);
			memset(p, 0, 2 * i);
            ASSERT_NE(nullptr, p);
            mem[i-1] = p;
        }
    }
    for (msize i = 1; i < 10*1024; ++i)
    {
        h_->free(mem[i-1]);
    }
}

TEST_F(HeapTest, TestStdAllocator)
{
    struct testval
    {
        int n;
        testval()
        :n(0)
        {}
    };
    
    typedef std::vector<testval, memheap::allocator<testval> > testvec;
    
	init_std_allocator(false, 10*1024 * sizeof(testval), 10);

	std::vector< testvec > mem;

	for (msize i = 1; i < 10*1024; ++i)
	{
		testvec tv;
		tv.resize(i);
		EXPECT_EQ(i, tv.size());
		mem.push_back(tv);
	}

	for (msize i = 1; i < 10*1024; ++i)
	{
		if (i%2 == 0) {
			mem[i-1].clear();
		}
	}
    
    for (msize i = 1; i < 10*1024; ++i)
    {
        if (i%2 == 0) {
			testvec tv;
			tv.resize(2*i);
            mem[i-1] = tv;
        }
    }
	mem.clear();

	free_std_allocator();
}

int main(int argc, char *argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
