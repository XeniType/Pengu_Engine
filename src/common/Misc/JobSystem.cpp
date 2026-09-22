#include "Pengu_Engine/Misc/JobSystem.hpp"

JobSystem::JobSystem() : stop_{ false }
{
	int threads = std::thread::hardware_concurrency();
	for (size_t i = 0; i < threads; ++i) {
		workers_.emplace_back(&JobSystem::Worker, this);
	}

}

//JobSystem::JobSystem(const JobSystem&& rvalue) {
//
//}
//JobSystem& JobSystem::operator=(const JobSystem&& rvalue) {
//
//}

JobSystem::~JobSystem()
{
	stop_ = true;
	condition_.notify_all();
	while (!workers_.empty()) {
		workers_.back().join();
		workers_.pop_back();
	}
}

void JobSystem::Worker()
{
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(queue_mutex_);
			condition_.wait(lock, [this] {return !tasks_.empty() || stop_; });
			if (stop_ && tasks_.empty()) return;
			task = std::move(tasks_.front());
			tasks_.pop();
		}
		task();
	}
}

void JobSystem::Enqueue(std::function<void()> task)
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		tasks_.push(task);
	}
	condition_.notify_one();
}
