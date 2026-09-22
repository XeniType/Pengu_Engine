/**
 * @file JobSystem.hpp
 * @brief A multi-threaded task scheduler (Thread Pool) for the Pengu Engine.
 * * This system manages a collection of worker threads that execute tasks
 * asynchronously to maximize CPU utilization across all available cores.
 */

#ifndef JOBSYSTEM_HPP
#define JOBSYSTEM_HPP 1

#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <algorithm>

 /**
	* @class JobSystem
	* @brief Manages a pool of worker threads and a synchronized task queue.
	* * The JobSystem is responsible for distributing work across multiple CPU cores.
	* It is non-copyable and non-movable to ensure the stability of the thread pool
	* during the engine's lifecycle.
	*/
class JobSystem {
public:

	/**
	 * @brief Constructor that initializes worker threads.
	 * @note Typically creates a number of threads equal to the hardware concurrency minus one.
	 */
	JobSystem();

	/// @name Deleted Operations
	/// JobSystem instances cannot be copied or moved due to active thread ownership.
	/// @{
	JobSystem(const JobSystem& rvalue) = delete;
	JobSystem& operator = (const JobSystem& rvalue) = delete;

	JobSystem(JobSystem&& rvalue) = delete;
	JobSystem& operator=(JobSystem&& rvalue) = delete;
	/// @}

	/**
	 * @brief Destructor.
	 * Signals all worker threads to stop and joins them before destruction.
	 */
	~JobSystem();

	/**
	 * @brief The main loop for worker threads.
	 * Workers wait on a condition variable until a new task is available in the queue.
	 */
	void Worker();

	/**
	 * @brief Adds a fire-and-forget task to the queue.
	 * @param task A callable object (lambda, function pointer) to be executed.
	 */
	void Enqueue(std::function<void()> task);

	/**
	 * @brief Adds a task to the queue and returns a future to retrieve the result.
	 * @tparam F The type of the callable object.
	 * @param task The task to be executed.
	 * @return A std::future that will eventually contain the result of the task.
	 * * @code
	 * auto result = jobSystem.Enqueue([]() { return 42; });
	 * // ... do other work ...
	 * int value = result.get(); // Blocks until the job is finished
	 * @endcode
	 */
	template<typename F>
	auto Enqueue(F&& task) -> std::future<std::invoke_result_t<F>>;

private:
	std::condition_variable condition_;         ///< Signals workers when new tasks arrive or the system stops.
	std::queue<std::function<void()>> tasks_;   ///< The synchronized queue of pending work units.
	std::vector<std::thread> workers_;          ///< The collection of persistent worker threads.
	std::atomic<bool> stop_;										///< Flag indicating if the system is shutting down.
	std::mutex queue_mutex_;                    ///< Protects access to the task queue and the stop flag.
};

/**
 * @brief Template implementation for Enqueue with return values.
 * * Wraps the task in a std::packaged_task to allow result retrieval via std::future.
 */
template<typename F>
inline auto JobSystem::Enqueue(F&& task) -> std::future<std::invoke_result_t<F>>
{
	using ReturnType = std::invoke_result_t<F>;
	auto pt = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(task)));

	std::future<ReturnType> result = pt->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		tasks_.push([pt]() {
			(*pt)();
			});
	}
	condition_.notify_one();
	return result;
}

#endif // !JOBSYSTEM_HPP

