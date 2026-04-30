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

    void renderTo(juce::Component& target, const ElementNode& root);
};
}
