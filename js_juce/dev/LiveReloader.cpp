#include "LiveReloader.h"

namespace js_juce
{
LiveReloader::LiveReloader() = default;
LiveReloader::~LiveReloader() = default;

void LiveReloader::start(const juce::File& fileToWatch, std::function<void()> callback, int pollMs)
{
    watchedFile = fileToWatch;
    onChange = std::move(callback);
    lastWriteTime = watchedFile.getLastModificationTime();
    startTimer(juce::jmax(50, pollMs));
}

void LiveReloader::stop()
{
    stopTimer();
    watchedFile = juce::File();
    onChange = nullptr;
}

void LiveReloader::timerCallback()
{
    if (! watchedFile.existsAsFile())
        return;

    const auto currentWriteTime = watchedFile.getLastModificationTime();
    if (currentWriteTime > lastWriteTime)
    {
        lastWriteTime = currentWriteTime;
        if (onChange != nullptr)
            onChange();
    }
}
}
