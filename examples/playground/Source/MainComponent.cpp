#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);

    auto jsFile = juce::File::getCurrentWorkingDirectory().getChildFile("app.js");
    if (! jsFile.existsAsFile())
        jsFile = juce::File::getCurrentWorkingDirectory().getParentDirectory().getChildFile("app.js");

    if (jsFile.existsAsFile())
        juce::ignoreUnused(bridge.attach(*this, jsFile, true));
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    for (auto* child : getChildren())
        child->setBounds(getLocalBounds());
}
