#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../runtime/JsRuntime.h"
#include "../ui/JuceRenderer.h"
#include "../ui/ElementNode.h"
#include "../dev/LiveReloader.h"

namespace js_juce
{
class JsBridge
{
public:
    JsBridge();
    ~JsBridge();

    bool attach(juce::Component& rootComponent, const juce::File& entryScriptFile, bool enableLiveReload);
    void setHostData(const juce::var& data);
    void detach();

    bool reloadNow(juce::String& errorMessage);
    juce::String getLastError() const;

private:
    void onJsRenderRequest(const juce::var& treeVar);
    void onLiveReloadTriggered();

    juce::Component* root = nullptr;
    juce::File entryScript;
    JsRuntime runtime;
    JuceRenderer renderer;
    LiveReloader reloader;
    juce::String lastError;
    juce::var initialHostData;
};
}
