#include "JsBridge.h"

namespace js_juce
{
JsBridge::JsBridge() = default;
JsBridge::~JsBridge() { detach(); }

bool JsBridge::attach(juce::Component& rootComponent, const juce::File& entryScriptFile, bool enableLiveReload)
{
    detach();

    root = &rootComponent;
    entryScript = entryScriptFile;
    runtime.setRenderCallback([this](const juce::var& treeVar) { onJsRenderRequest(treeVar); });

    if (! runtime.initialise())
    {
        lastError = "Failed to initialize QuickJS runtime. Ensure quickjs.h is available in project include paths.";
        return false;
    }

    juce::String err;
    if (! reloadNow(err))
        return false;

    if (enableLiveReload)
        reloader.start(entryScript, [this]() { onLiveReloadTriggered(); });

    return true;
}

void JsBridge::detach()
{
    reloader.stop();
    runtime.shutdown();
    root = nullptr;
    entryScript = juce::File();
    lastError.clear();
}

bool JsBridge::reloadNow(juce::String& errorMessage)
{
    if (root == nullptr)
    {
        errorMessage = "Root component not attached.";
        lastError = errorMessage;
        return false;
    }

    if (! runtime.evaluateFile(entryScript, errorMessage))
    {
        lastError = errorMessage;
        return false;
    }

    lastError.clear();
    return true;
}

juce::String JsBridge::getLastError() const
{
    return lastError;
}

void JsBridge::onJsRenderRequest(const juce::var& treeVar)
{
    if (root == nullptr)
        return;

    const auto node = ElementNode::fromVar(treeVar);
    renderer.renderTo(*root, node);
    root->resized();
    root->repaint();
}

void JsBridge::onLiveReloadTriggered()
{
    juce::String err;
    reloadNow(err);
}
}
