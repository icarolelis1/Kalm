#include "Graphics/VulkanFramework.h"
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace RENDER {

	class Thread {

	public:
		Thread();

		void addJob(std::function<void()> f);

		void wait();

		~Thread();

	private:

		bool should_destroy = false;
		std::thread threadWork;
		std::queue<std::function<void()>> jobQueue;
		std::mutex queueMutex;
		std::condition_variable condition;

		void queueThread();


	};

	class ThreadPool {

	public:

		std::vector<std::unique_ptr<Thread>> threads;

		void setThreadCount(uint32_t count);

		void wait();

	};

}