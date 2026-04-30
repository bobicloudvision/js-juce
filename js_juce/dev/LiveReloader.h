#pragma once

#include <juce_core/juce_core.h>

namespace js_juce
{
class LiveReloader : private juce::Timer
{
public:
    LiveReloader();
    ~LiveReloader() override;

    void start(const juce::File& fileToWatch, std::function<void()> callback, int pollMs = 250);
    void stop();

private:
    void timerCallback() override;

    juce::File watchedFile;
    juce::Time lastWriteTime;
    std::function<void()> onChange;
};
}
