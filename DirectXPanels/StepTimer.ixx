module;
#include "pch.h"

export module DX:StepTimer;

import winrt.Windows.Foundation;

export namespace DX
{

class StepTimer
{
  public:
    // Get elapsed time since the previous Update call.
    std::uint64_t GetElapsedTicks() const
    {
        return m_elapsedTicks;
    }
    double GetElapsedSeconds() const
    {
        return TicksToSeconds(m_elapsedTicks);
    }

    // Get total time since the start of the program.
    std::uint64_t GetTotalTicks() const
    {
        return m_totalTicks;
    }
    double GetTotalSeconds() const
    {
        return TicksToSeconds(m_totalTicks);
    }

    // Get total number of updates since start of the program.
    std::uint32_t GetFrameCount() const
    {
        return m_frameCount;
    }

    // Get the current framerate.
    std::uint32_t GetFramesPerSecond() const
    {
        return m_framesPerSecond;
    }

    // Set whether to use fixed or variable timestep mode.
    void SetFixedTimeStep(bool isFixedTimestep)
    {
        m_isFixedTimeStep = isFixedTimestep;
    }

    // When in fixed timestep mode, set how often to call Update.
    void SetTargetElapsedTicks(std::uint64_t targetElapsed)
    {
        m_targetElapsedTicks = targetElapsed;
    }
    void SetTargetElapsedSeconds(double targetElapsed)
    {
        m_targetElapsedTicks = SecondsToTicks(targetElapsed);
    }

    // Integer format represents time using 10,000,000 ticks per second.
    static inline constexpr std::uint64_t TicksPerSecond = 10000000;

    static double TicksToSeconds(std::uint64_t ticks)
    {
        return (double)ticks / TicksPerSecond;
    }
    static std::uint64_t SecondsToTicks(double seconds)
    {
        return (std::uint64_t)(seconds * TicksPerSecond);
    }

    // After an intentional timing discontinuity (for instance a blocking IO operation)
    // call this to avoid fixed timestep logic attempting a string of catch-up Update calls.
    void ResetElapsedTime()
    {
        winrt::check_bool(QueryPerformanceCounter(&m_qpcLastTime));

        m_leftOverTicks = 0;
        m_framesPerSecond = 0;
        m_framesThisSecond = 0;
        m_qpcSecondCounter = 0;
    }

    // Update timer state, calling the specified Update function the appropriate number of times.
    template <typename TUpdate> void Tick(const TUpdate &update)
    {
        // Query the current time.
        LARGE_INTEGER currentTime;

        winrt::check_bool(QueryPerformanceCounter(&currentTime));

        std::uint64_t timeDelta = currentTime.QuadPart - m_qpcLastTime.QuadPart;

        m_qpcLastTime = currentTime;
        m_qpcSecondCounter += timeDelta;

        // Clamp excessively large time deltas (eg. after paused in the debugger).
        if (timeDelta > m_qpcMaxDelta)
        {
            timeDelta = m_qpcMaxDelta;
        }

        // Convert QPC units into our own canonical tick format. Cannot overflow due to the previous clamp.
        timeDelta *= TicksPerSecond;
        timeDelta /= m_qpcFrequency.QuadPart;

        std::uint32_t lastFrameCount = m_frameCount;

        if (m_isFixedTimeStep)
        {
            // Fixed timestep update logic.

            // If we are running very close to the target elapsed time (within 1/4 ms) we just clamp
            // the clock to exactly match our target value. This prevents tiny and irrelvant errors
            // from accumulating over time. Without this clamping, a game that requested a 60 fps
            // fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
            // accumulate enough tiny errors that it would drop a frame. Better to just round such
            // small deviations down to zero, and leave things running smoothly.

            if (std::abs((std::int64_t)(timeDelta - m_targetElapsedTicks)) < TicksPerSecond / 4000)
            {
                timeDelta = m_targetElapsedTicks;
            }

            m_leftOverTicks += timeDelta;

            while (m_leftOverTicks >= m_targetElapsedTicks)
            {
                m_elapsedTicks = m_targetElapsedTicks;
                m_totalTicks += m_targetElapsedTicks;
                m_leftOverTicks -= m_targetElapsedTicks;
                m_frameCount++;

                update();
            }
        }
        else
        {
            // Variable timestep update logic.
            m_elapsedTicks = timeDelta;
            m_totalTicks += timeDelta;
            m_leftOverTicks = 0;
            m_frameCount++;

            update();
        }

        // Track the current framerate.
        if (m_frameCount != lastFrameCount)
        {
            m_framesThisSecond++;
        }

        if (m_qpcSecondCounter >= (std::uint64_t)m_qpcFrequency.QuadPart)
        {
            m_framesPerSecond = m_framesThisSecond;
            m_framesThisSecond = 0;
            m_qpcSecondCounter %= m_qpcFrequency.QuadPart;
        }
    }

  private:
    // Source timing data uses QPC units.
    LARGE_INTEGER m_qpcFrequency;
    LARGE_INTEGER m_qpcLastTime;
    std::uint64_t m_qpcMaxDelta;

    // Derived timing data uses our own canonical tick format.
    std::uint64_t m_elapsedTicks = 0;
    std::uint64_t m_totalTicks = 0;
    std::uint64_t m_leftOverTicks = 0;

    // For tracking the framerate.
    std::uint32_t m_frameCount = 0;
    std::uint32_t m_framesPerSecond = 0;
    std::uint32_t m_framesThisSecond = 0;
    std::uint64_t m_qpcSecondCounter = 0;

    // For configuring fixed timestep mode.
    bool m_isFixedTimeStep = false;
    std::uint64_t m_targetElapsedTicks = TicksPerSecond / 60;
};

} // namespace DX