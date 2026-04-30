/*
  ==============================================================================

    This file is part of the js_juce module.

  ==============================================================================
*/

#pragma once

/*
    BEGIN_JUCE_MODULE_DECLARATION

    ID:                 js_juce
    vendor:             js_juce
    version:            0.0.1
    name:               js_juce
    description:        QuickJS-powered native JUCE GUI from JS/JSX.
    website:            https://github.com
    license:            MIT
    dependencies:       juce_core juce_events juce_gui_basics

    END_JUCE_MODULE_DECLARATION
*/

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "runtime/JsRuntime.h"
#include "ui/ElementNode.h"
#include "ui/JuceRenderer.h"
#include "bridge/JsBridge.h"
#include "dev/LiveReloader.h"

namespace js_juce
{
inline constexpr const char* moduleName = "js_juce";
}
