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
    renderer.setControlCallback([this](const juce::String& callbackId, const juce::var& value)
    {
        juce::String err;
        if (! runtime.invokeControlCallback(callbackId, value, err))
            lastError = err;
    });

    if (! runtime.initialise())
    {
        lastError = "Failed to initialize QuickJS runtime. Ensure quickjs.h is available in project include paths.";
        return false;
    }
    runtime.setHostData(initialHostData);

    juce::String err;
    if (! reloadNow(err))
        return false;

    if (enableLiveReload)
        reloader.start(entryScript, [this]() { onLiveReloadTriggered(); });

    return true;
}

void JsBridge::setHostData(const juce::var& data)
{
    initialHostData = data;
    runtime.setHostData(data);
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

    if (! evaluateEntryScript(errorMessage))
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

bool JsBridge::evaluateEntryScript(juce::String& errorMessage)
{
    if (entryScript.hasFileExtension("jsx"))
    {
        juce::File transpiled;
        if (! transpileJsxToTemp(entryScript, transpiled, errorMessage))
            return false;

        compiledScript = transpiled;
        return runtime.evaluateFile(compiledScript, errorMessage);
    }

    compiledScript = juce::File();
    return runtime.evaluateFile(entryScript, errorMessage);
}

bool JsBridge::transpileJsxToTemp(const juce::File& jsxFile, juce::File& outJsFile, juce::String& errorMessage) const
{
    if (! jsxFile.existsAsFile())
    {
        errorMessage = "JSX source file does not exist: " + jsxFile.getFullPathName();
        return false;
    }

    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("js_juce");
    tempDir.createDirectory();
    outJsFile = tempDir.getChildFile("compiled_playground.js");

    const auto inputPath = jsxFile.getFullPathName();
    const auto outputPath = outJsFile.getFullPathName();
    const auto esbuildFromEnv = juce::SystemStats::getEnvironmentVariable("JS_JUCE_ESBUILD", "");
    juce::String esbuildBin;
    if (esbuildFromEnv.isNotEmpty())
    {
        esbuildBin = esbuildFromEnv;
    }
    else
    {
        const auto localEsbuild = jsxFile.getParentDirectory()
                                     .getChildFile("jsx-tooling")
                                     .getChildFile("node_modules")
                                     .getChildFile(".bin")
                                     .getChildFile("esbuild");

        if (localEsbuild.existsAsFile())
            esbuildBin = localEsbuild.getFullPathName();
        else
            esbuildBin = "esbuild";
    }

    const auto command = esbuildBin
        + " " + inputPath
        + " --loader:.jsx=jsx"
        + " --jsx=transform"
        + " --jsx-factory=JuceUI.createElement"
        + " --jsx-fragment=JuceUI.Fragment"
        + " --format=iife"
        + " --platform=browser"
        + " --outfile=" + outputPath;

    juce::ChildProcess process;
    if (! process.start(command))
    {
        errorMessage = "Failed to start JSX compiler. Install esbuild globally, run npm install in examples/playground/jsx-tooling, or set JS_JUCE_ESBUILD env var.";
        return false;
    }

    const auto finished = process.waitForProcessToFinish(10000);
    const auto output = process.readAllProcessOutput();
    if (! finished || process.getExitCode() != 0 || ! outJsFile.existsAsFile())
    {
        errorMessage = "JSX compile failed.\nCommand: " + command + "\nOutput: " + output;
        return false;
    }

    return true;
}
}
