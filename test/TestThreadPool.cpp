#include<threadpool.h>
#include<gtest/gtest.h>
#include<chrono>


class ThreadPoolTest : public ::testing::Test
{
protected: 
	virtual void SetUp()
	{
		thread_pool.start();
		
	
	}

	virtual void TearDown()
	{
		thread_pool.stop();
	}

	

	threadpool::ThreadPool thread_pool ;

public:

	void a_job(int iNumber)
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "I am task number: " << iNumber+1 << " executed by thread: " << std::this_thread::get_id() << "\n";
	}
	
};

TEST_F(ThreadPoolTest, simple_test)
{
	for (int i = 0; i < 30; i++)
	{
		thread_pool.enqueue(&ThreadPoolTest::a_job, this, i);
		
	}
}