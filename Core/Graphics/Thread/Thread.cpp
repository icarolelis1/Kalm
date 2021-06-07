#include "Thread.h"

RENDER::Thread::Thread()
{

	threadWork = std::thread(&Thread::queueThread, this);


}

void RENDER::Thread::addJob(std::function<void()> f)
{
	std::lock_guard<std::mutex> lock(queueMutex);
	jobQueue.push(std::move(f));
	condition.notify_one();

}

void RENDER::Thread::wait()
{
	std::unique_lock<std::mutex> lock(queueMutex);
	condition.wait(lock, [this]() { return jobQueue.empty(); });
}

RENDER::Thread::~Thread()
{

	if (threadWork.joinable())
	{
		

		wait();
		queueMutex.lock();
		should_destroy = true;
		condition.notify_one();
		queueMutex.unlock();
		threadWork.join();
	}
}

void RENDER::Thread::queueThread()
{
	while (true) {

		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			condition.wait(lock, [this] { return !jobQueue.empty() || should_destroy; });
			if (should_destroy)
			{
				break;
			}
			job = jobQueue.front();
		}
		job();

		{
			std::lock_guard<std::mutex> lock(queueMutex);
			jobQueue.pop();
			condition.notify_one();

		}

	}


}

void RENDER::ThreadPool::setThreadCount(uint32_t count)
{

	threads.clear();

	for (unsigned int  i = 0; i < count; i++) {
		threads.push_back(std::make_unique<Thread>());
	}

}

void RENDER::ThreadPool::wait()
{

	for (auto& t : threads) {
		t->wait();
	}
}
