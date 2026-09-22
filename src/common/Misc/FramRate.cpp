#include "Pengu_Engine/Misc/FramRate.hpp"

FrameRate::FrameRate()
{

	fpsCount_ = 0;
	fps_ = 0.0;
	lastTime_ = std::chrono::steady_clock::now();
	fpsTimer_ = lastTime_;
	deltaTime_ = 0.0f;

}

FrameRate::~FrameRate()
{
}

FrameRate& FrameRate::Get()
{
	static FrameRate Instance;

	return Instance;
}

void FrameRate::FPSTick()
{
	fpsCount_++;
	auto now = std::chrono::steady_clock::now();
	auto elapsedSinceFPS =
		std::chrono::duration<double>(now - fpsTimer_).count();

	if (elapsedSinceFPS >= 1.0f) {
		fps_ = fpsCount_ / elapsedSinceFPS;
		fpsTimer_ = now;
		fpsCount_ = 0;
	}

	deltaTime_ =
		std::chrono::duration_cast<std::chrono::microseconds>(now - lastTime_).count() / 1000.0f;
	lastTime_ = now;
}

const double FrameRate::GetFPS()
{
	return fps_;
}

const double FrameRate::GetDeltaTime()
{
	return deltaTime_;
}

