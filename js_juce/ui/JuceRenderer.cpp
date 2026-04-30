#include "JuceRenderer.h"

namespace js_juce
{
namespace
{
class ViewComponent final : public juce::Component
{
public:
    explicit ViewComponent(bool isRow) : row(isRow) {}

    void resized() override
    {
        juce::FlexBox fb;
        fb.flexDirection = row ? juce::FlexBox::Direction::row : juce::FlexBox::Direction::column;
        fb.flexWrap = juce::FlexBox::Wrap::noWrap;

        for (auto* c : children)
            fb.items.add(juce::FlexItem(*c).withFlex(1.0f));

        fb.performLayout(getLocalBounds().toFloat());
    }

    juce::Array<juce::Component*> children;
    bool row = false;
};

static juce::String readTextProp(const ElementNode& node, const juce::String& name)
{
    const auto* value = node.props.getVarPointer(name);
    return value != nullptr ? value->toString() : juce::String();
}

static std::unique_ptr<juce::Component> buildComponent(const ElementNode& node)
{
    if (node.type == "Text")
    {
        auto label = std::make_unique<juce::Label>();
        label->setText(readTextProp(node, "text"), juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centredLeft);
        return label;
    }

    if (node.type == "Button")
    {
        auto button = std::make_unique<juce::TextButton>(readTextProp(node, "text"));
        return button;
    }

    if (node.type == "Slider")
    {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
        return slider;
    }

    if (node.type == "TextInput")
    {
        auto editor = std::make_unique<juce::TextEditor>();
        editor->setText(readTextProp(node, "text"));
        return editor;
    }

    auto view = std::make_unique<ViewComponent>(node.type == "Row");
    for (const auto& child : node.children)
    {
        auto builtChild = buildComponent(child);
        view->children.add(builtChild.get());
        view->addAndMakeVisible(*builtChild);
        builtChild.release();
    }

    return view;
}
}

void JuceRenderer::renderTo(juce::Component& target, const ElementNode& root)
{
    target.removeAllChildren();
    auto rootComponent = buildComponent(root);
    target.addAndMakeVisible(*rootComponent);
    rootComponent->setBounds(target.getLocalBounds());
    rootComponent.release();
}
}
