#include "MainComponent.h"
#include <cstdlib>

namespace
{
juce::File findByWalkingUp(juce::File startDir, const juce::String& relativePath, int maxLevels)
{
    auto dir = startDir;
    for (int i = 0; i < maxLevels; ++i)
    {
        const auto candidate = dir.getChildFile(relativePath);
        if (candidate.existsAsFile())
            return candidate;
        dir = dir.getParentDirectory();
    }
    return {};
}

juce::File findPlaygroundScript()
{
    const auto cwd = juce::File::getCurrentWorkingDirectory();
    const auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    const auto appDir = appFile.getParentDirectory();

    if (const auto* envPath = std::getenv("JS_JUCE_APP_JS"))
    {
        const juce::File envFile(juce::String::fromUTF8(envPath));
        if (envFile.existsAsFile())
            return envFile;
    }

    const juce::Array<juce::File> candidates {
        cwd.getChildFile("app.js"),
        cwd.getChildFile("examples/playground/app.js"),
        cwd.getParentDirectory().getChildFile("app.js"),
        cwd.getParentDirectory().getChildFile("examples/playground/app.js"),
        appDir.getChildFile("app.js"),
        appDir.getParentDirectory().getChildFile("app.js"),
        appDir.getParentDirectory().getParentDirectory().getChildFile("examples/playground/app.js")
    };

    for (const auto& file : candidates)
        if (file.existsAsFile())
            return file;

    const auto fromCwd = findByWalkingUp(cwd, "examples/playground/app.js", 16);
    if (fromCwd.existsAsFile())
        return fromCwd;

    const auto fromAppDir = findByWalkingUp(appDir, "examples/playground/app.js", 16);
    if (fromAppDir.existsAsFile())
        return fromAppDir;

    const auto localFromCwd = findByWalkingUp(cwd, "app.js", 16);
    if (localFromCwd.existsAsFile())
        return localFromCwd;

    return {};
}
}

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);

    const auto jsFile = findPlaygroundScript();
    if (! jsFile.existsAsFile())
    {
        statusMessage = "app.js not found. Set JS_JUCE_APP_JS env var or place examples/playground/app.js in reachable parent path.";
        return;
    }

    if (! bridge.attach(*this, jsFile, true))
    {
        statusMessage = bridge.getLastError();
        return;
    }

    statusMessage = "Loaded: " + jsFile.getFullPathName();
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    if (! statusMessage.isEmpty() && getChildren().isEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(15.0f));
        g.drawFittedText(statusMessage, getLocalBounds().reduced(16), juce::Justification::topLeft, 6);
    }
}

void MainComponent::resized()
{
    for (auto* child : getChildren())
        child->setBounds(getLocalBounds());
}
