#include <js_juce/js_juce.h>

namespace
{
juce::File findEntryScript()
{
    auto dir = juce::File::getCurrentWorkingDirectory();
    for (int i = 0; i < 6; ++i)
    {
        const auto candidate = dir.getChildFile("examples/basic-app/app.js");
        if (candidate.existsAsFile())
            return candidate;
        dir = dir.getParentDirectory();
    }

    return juce::File();
}

class RootComponent final : public juce::Component
{
public:
    RootComponent()
    {
        const auto script = findEntryScript();
        if (script.existsAsFile())
        {
            juce::ignoreUnused(bridge.attach(*this, script, true));
        }
    }

    void resized() override
    {
        for (auto* child : getChildren())
            child->setBounds(getLocalBounds());
    }

private:
    js_juce::JsBridge bridge;
};

class MainWindow final : public juce::DocumentWindow
{
public:
    MainWindow() : juce::DocumentWindow("js_juce_host",
                                        juce::Colours::darkgrey,
                                        juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setContentOwned(new RootComponent(), true);
        centreWithSize(900, 600);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class App final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "js_juce_host"; }
    const juce::String getApplicationVersion() override { return "0.0.1"; }
    void initialise(const juce::String&) override { window = std::make_unique<MainWindow>(); }
    void shutdown() override { window.reset(); }

private:
    std::unique_ptr<MainWindow> window;
};
}

START_JUCE_APPLICATION(App)
