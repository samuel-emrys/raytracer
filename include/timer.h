#include <date/date.h>
#include <chrono>
#include <iostream>

template <class Period>
class Timer {
  public:
    Timer() { start(); }
    Timer(Timer&) = default;
    Timer(Timer&&)  noexcept = default;
    auto operator=(const Timer&) -> Timer& = default;
    auto operator=(Timer&&)  noexcept -> Timer& = default;

    ~Timer() { finish(); }

    void start() { mStartTime = std::chrono::steady_clock::now(); }
    void finish() {
        mEndTime = std::chrono::steady_clock::now();
        auto vDuration = std::chrono::duration_cast<Period>(
            mEndTime - mStartTime);
        using namespace date;
        std::cerr << "Duration " << vDuration << std::endl;
    }

  private:
    std::chrono::steady_clock::time_point mStartTime;
    std::chrono::steady_clock::time_point mEndTime;
};
