/**
 * @file FramRate.hpp
 * @brief Utility for measuring engine performance and calculating frame-delta time.
 */

#ifndef FRAMERATE_HPP
#define FRAMERATE_HPP 1

#include <chrono>

 /**
	* @class FrameRate
	* @brief Singleton class that tracks frames per second (FPS) and time between frames.
	* 
	* This class provides the necessary timing data to ensure game logic
	* runs at a consistent speed regardless of the hardware's frame rate.
	*/
class FrameRate {
public:
	/** @brief Destructor. */
	~FrameRate();

	/**
	 * @brief Accesses the global FrameRate instance.
	 * @return Reference to the singleton instance.
	 */
	static FrameRate& Get();

	/**
	 * @brief Updates the internal timers at the start of a new frame.
	 * 
	 * This method calculates the Delta Time and updates the FPS counter once every second.
	 */
	void FPSTick();

	/**
	 * @brief Retrieves the average frames per second.
	 * @return The number of frames rendered in the last second.
	 */
	const double GetFPS();

	/**
	 * @brief Retrieves the time elapsed since the last frame.
	 * @return Double representing seconds passed (e.g., 0.016 for 60 FPS).
	 * @note Multiply movement vectors by this value for frame-rate independence.
	 */
	const double GetDeltaTime();

private:
	/** @brief Private constructor to enforce singleton pattern. */
	FrameRate();

	std::chrono::time_point<std::chrono::steady_clock> lastTime_; ///< Point in time of the previous frame.
	std::chrono::time_point<std::chrono::steady_clock> fpsTimer_; ///< Timer used to calculate the 1-second FPS interval.
	int fpsCount_;    ///< Counter for frames processed in the current second.
	double fps_;      ///< The cached FPS result.
	double deltaTime_; ///< The calculated time difference between the last two frames.
};

#endif // !FRAMERATE_HPP
