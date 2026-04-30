#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ElementNode.h"

namespace js_juce
{
class JuceRenderer
{
public:
    JuceRenderer() = default;
    ~JuceRenderer() = default;

    void setControlCallback(std::function<void(const juce::String&, const juce::var&)> callback);
    void renderTo(juce::Component& target, const ElementNode& root);

private:
    std::function<void(const juce::String&, const juce::var&)> onControl;
};
}
