#include "JuceRenderer.h"
#include <optional>

namespace js_juce
{
namespace
{
class ViewComponent final : public juce::Component
{
public:
    explicit ViewComponent(bool isRow) : row(isRow) {}

    void paint(juce::Graphics& g) override
    {
        if (backgroundColour.has_value())
            g.fillAll(*backgroundColour);

        if (borderWidth > 0.0f && borderColour.has_value())
        {
            g.setColour(*borderColour);
            g.drawRect(getLocalBounds(), static_cast<int>(borderWidth));
        }
    }

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
    std::optional<juce::Colour> backgroundColour;
    std::optional<juce::Colour> borderColour;
    float borderWidth = 0.0f;
};

static juce::String readTextProp(const ElementNode& node, const juce::String& name)
{
    const auto* value = node.props.getVarPointer(name);
    return value != nullptr ? value->toString() : juce::String();
}

static double readNumberProp(const ElementNode& node, const juce::String& name, double fallbackValue = 0.0)
{
    const auto* value = node.props.getVarPointer(name);
    return value != nullptr ? static_cast<double>(*value) : fallbackValue;
}

static std::optional<juce::Colour> readColourProp(const ElementNode& node, const juce::String& name)
{
    const auto text = readTextProp(node, name).trim();
    if (text.isEmpty())
        return std::nullopt;

    if (text.startsWith("#"))
    {
        auto hex = text.substring(1);
        if (hex.length() == 6)
            hex = "ff" + hex;
        if (hex.length() == 8)
            return juce::Colour::fromString(hex);
    }

    return juce::Colours::findColourForName(text, juce::Colours::transparentBlack);
}

static std::unique_ptr<juce::Component> buildComponent(const ElementNode& node)
{
    if (node.type == "Text")
    {
        auto label = std::make_unique<juce::Label>();
        label->setText(readTextProp(node, "text"), juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centredLeft);
        if (const auto colour = readColourProp(node, "color"))
            label->setColour(juce::Label::textColourId, *colour);
        if (const auto colour = readColourProp(node, "background"))
            label->setColour(juce::Label::backgroundColourId, *colour);
        if (const auto colour = readColourProp(node, "borderColor"))
            label->setColour(juce::Label::outlineColourId, *colour);
        return label;
    }

    if (node.type == "Button")
    {
        auto button = std::make_unique<juce::TextButton>(readTextProp(node, "text"));
        if (const auto colour = readColourProp(node, "color"))
        {
            button->setColour(juce::TextButton::textColourOffId, *colour);
            button->setColour(juce::TextButton::textColourOnId, *colour);
        }
        if (const auto colour = readColourProp(node, "background"))
            button->setColour(juce::TextButton::buttonColourId, *colour);
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
        if (const auto colour = readColourProp(node, "color"))
            editor->setColour(juce::TextEditor::textColourId, *colour);
        if (const auto colour = readColourProp(node, "background"))
            editor->setColour(juce::TextEditor::backgroundColourId, *colour);
        if (const auto colour = readColourProp(node, "borderColor"))
            editor->setColour(juce::TextEditor::outlineColourId, *colour);
        return editor;
    }

    auto view = std::make_unique<ViewComponent>(node.type == "Row");
    view->backgroundColour = readColourProp(node, "background");
    view->borderColour = readColourProp(node, "borderColor");
    view->borderWidth = static_cast<float>(readNumberProp(node, "borderWidth", 0.0));
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
