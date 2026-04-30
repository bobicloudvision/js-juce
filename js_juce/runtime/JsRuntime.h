#pragma once

#include <functional>
#include <memory>

#include <juce_core/juce_core.h>

namespace js_juce
{
class JsRuntime
{
public:
    JsRuntime();
    ~JsRuntime();

    bool initialise();
    void shutdown();

    void setRenderCallback(std::function<void(const juce::var&)> callback);
    bool evaluateFile(const juce::File& scriptFile, juce::String& errorMessage);
    bool invokeControlCallback(const juce::String& callbackId, const juce::var& value, juce::String& errorMessage);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
}
