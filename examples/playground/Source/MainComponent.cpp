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

    if (const auto* envPath = std::getenv("JS_JUCE_APP_JSX"))
    {
        const juce::File envFile(juce::String::fromUTF8(envPath));
        if (envFile.existsAsFile())
            return envFile;
    }

    const juce::Array<juce::File> candidates {
        cwd.getChildFile("app.jsx"),
        cwd.getChildFile("examples/playground/app.jsx"),
        cwd.getParentDirectory().getChildFile("app.jsx"),
        cwd.getParentDirectory().getChildFile("examples/playground/app.jsx"),
        appDir.getChildFile("app.jsx"),
        appDir.getParentDirectory().getChildFile("app.jsx"),
        appDir.getParentDirectory().getParentDirectory().getChildFile("examples/playground/app.jsx")
    };

    for (const auto& file : candidates)
        if (file.existsAsFile())
            return file;

    const auto fromCwdJsx = findByWalkingUp(cwd, "examples/playground/app.jsx", 16);
    if (fromCwdJsx.existsAsFile())
        return fromCwdJsx;

    const auto fromAppDirJsx = findByWalkingUp(appDir, "examples/playground/app.jsx", 16);
    if (fromAppDirJsx.existsAsFile())
        return fromAppDirJsx;

    const auto localFromCwdJsx = findByWalkingUp(cwd, "app.jsx", 16);
    if (localFromCwdJsx.existsAsFile())
        return localFromCwdJsx;

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
        statusMessage = "app.jsx not found. Set JS_JUCE_APP_JSX env var, or place examples/playground/app.jsx in reachable parent path.";
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
