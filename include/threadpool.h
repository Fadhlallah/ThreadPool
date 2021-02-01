#pragma once
#include<vector>
#include<thread>
#include <mutex>
#include <queue>
#include<functional>
#include<future>
#include<iostream>

#ifdef __WIN__
	#ifdef THREAD_POOL_EXPORTS
		#define THREAD_POOL __declspec(dllexport)
	#else
		#define THREAD_POOL __declspec(dllimport)
	#endif
#endif



namespace threadpool
{
	
	class THREAD_POOL ThreadPool
	{

	private:

		

	public:
		enum class status
		{
			started = 0,
			paused,
			stopped,

			maxNbre
		};
		void job_loop();

		template<class F, class ...T>
		auto enqueue(F&& i_task, T&& ...i_params)->std::future<typename std::result_of<F(T...)>::type>;

		
		ThreadPool(uint32_t i_jobsnbr = 5, bool i_start = false);
		~ThreadPool();
		void start();
		void stop(bool i_finish_job = true);
		void pause();
		uint32_t  getJobnbr() const { return _jobnbr;  }
		status getStatus() const { return  _thread_pool_status; }
		void changePoolSize(uint32_t i_jobsnbr);

	private:


		uint32_t _jobnbr;
		std::vector<std::thread> _workers;
		std::mutex _workers_mutex;
		std::condition_variable _workers_cond_variable ;
		std::condition_variable _finish_job_variable ;
		std::queue<std::function<void()>> _jobs;

		

		status _thread_pool_status = status::stopped;


	};


	
	
	ThreadPool::~ThreadPool()
	{
		for (std::thread& worker: _workers)
		{
			worker.join();
		}

		_thread_pool_status = status::stopped;
	}

	void  ThreadPool::job_loop()
	{
		std::cout << "thread id(" << std::this_thread::get_id() << ") starting\n";
		for (;;)
		{
			std::unique_lock<std::mutex> a_lock(_workers_mutex);
			
			if (_thread_pool_status != status::started)
			{
				std::cout << "thread id(" << std::this_thread::get_id() << ") quitting, byeeeeeeeeee\n";
				a_lock.unlock();
				break;;
			}
			auto a_condition = [this]() {return (!_jobs.empty()
				&& (_thread_pool_status == status::started)); };
			_workers_cond_variable.wait(a_lock, a_condition);
			std::function<void()> a_job = std::move( _jobs.front());
			_jobs.pop();
			a_lock.unlock();
			a_job();
			_finish_job_variable.notify_one();			
		}
	}
	
	ThreadPool::ThreadPool(uint32_t i_jobsnbr, bool i_start ) : _jobnbr(i_jobsnbr)
	{
		if (i_start == false)
			return;
		for (uint32_t index = 0; index < _jobnbr; index++)
		{
			_workers.push_back(std::thread(&ThreadPool::job_loop, this));
		}

		_thread_pool_status = status::started;
	}

	template<class F, class ...T>
	auto ThreadPool:: enqueue(F&& i_task, T&& ...i_params) 
		-> std::future<typename std::result_of<F(T...)>::type>
	{
		std::unique_lock<std::mutex> a_lock(_workers_mutex);
		if (_thread_pool_status == status::stopped)
			throw std::runtime_error("The threadpool is stopped");
		using ret_type = std::result_of<F(T...)>::type;
		std::shared_ptr<std::packaged_task<ret_type()>> task = 
			std::make_shared<std::packaged_task<ret_type()>>(std::bind(std::forward<F>(i_task), std::forward<T>(i_params)...));
		std::future<ret_type> future_obj =  task->get_future();
		
		_jobs.push([task]() { (*task)(); });
		_workers_cond_variable.notify_one();

		return future_obj;
	}

	void ThreadPool::start()
	{
		std::lock_guard<std::mutex> a_guard(_workers_mutex);
		if (_thread_pool_status == status::started)
			return;
		for (uint32_t index = 0; index <= _jobnbr; index++)
		{
			_workers.push_back(std::thread(&ThreadPool::job_loop, this));
		}

		_thread_pool_status = status::started;
	}

	void ThreadPool::pause()
	{
		std::lock_guard<std::mutex> a_guard(_workers_mutex);
		if (_thread_pool_status != status::paused)
			_thread_pool_status = status::paused;
	}

	void ThreadPool::stop(bool i_finish_job)
	{
		std::unique_lock<std::mutex> a_lock(_workers_mutex);
		std::cout<<"thread stopping 1\n";
		if(i_finish_job)
			_finish_job_variable.wait(a_lock, [this](){ return this->_jobs.empty();});

		_thread_pool_status = status::stopped;
		a_lock.unlock();
		
		for (std::thread& worker : _workers)
		{
			std::cout<<"thread "<< worker.get_id() << " stopping\n";
			worker.join();
			std::cout<<"thread "<< worker.get_id() << " stopped \n";
		}
		
		while (!_jobs.empty()) _jobs.pop();

		std::cout<<"Threadpool stopped\n";
	}

	void ThreadPool::changePoolSize(uint32_t i_jobsnbr)
	{
		stop();
		_jobnbr = i_jobsnbr;
		if (_thread_pool_status == status::started)
			start();
	}
}





