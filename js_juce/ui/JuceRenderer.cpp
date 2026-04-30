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
        if (gradientFrom.has_value() && gradientTo.has_value())
        {
            juce::ColourGradient gradient(*gradientFrom,
                                          0.0f,
                                          0.0f,
                                          *gradientTo,
                                          gradientVertical ? 0.0f : static_cast<float>(getWidth()),
                                          gradientVertical ? static_cast<float>(getHeight()) : 0.0f,
                                          false);
            g.setGradientFill(gradient);
            g.fillAll();
        }
        else if (backgroundColour.has_value())
        {
            g.fillAll(*backgroundColour);
        }

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
    std::optional<juce::Colour> gradientFrom;
    std::optional<juce::Colour> gradientTo;
    bool gradientVertical = true;
    std::optional<juce::Colour> borderColour;
    float borderWidth = 0.0f;
};

static juce::var readVarProp(const ElementNode& node, const juce::String& name)
{
    const auto* value = node.props.getVarPointer(name);
    return value != nullptr ? *value : juce::var();
}

static juce::String readTextProp(const ElementNode& node, const juce::String& name)
{
    return readVarProp(node, name).toString();
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

static juce::String readCallbackId(const ElementNode& node)
{
    const auto callbackVar = readVarProp(node, "onControl");
    if (const auto* dyn = callbackVar.getDynamicObject())
        return dyn->getProperty("__fnId").toString();
    return {};
}

static std::unique_ptr<juce::Component> buildComponent(
    const ElementNode& node,
    const std::function<void(const juce::String&, const juce::var&)>& onControl)
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
        const auto callbackId = readCallbackId(node);
        if (callbackId.isNotEmpty() && onControl != nullptr)
        {
            button->onClick = [cb = callbackId, onControl]
            {
                onControl(cb, juce::var(true));
            };
        }
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
        const auto callbackId = readCallbackId(node);
        if (callbackId.isNotEmpty() && onControl != nullptr)
        {
            slider->onValueChange = [s = slider.get(), cb = callbackId, onControl]
            {
                onControl(cb, juce::var(s->getValue()));
            };
        }
        return slider;
    }

    if (node.type == "TextInput")
    {
        auto editor = std::make_unique<juce::TextEditor>();
        editor->setText(readTextProp(node, "text"));
        const auto callbackId = readCallbackId(node);
        if (callbackId.isNotEmpty() && onControl != nullptr)
        {
            editor->onTextChange = [e = editor.get(), cb = callbackId, onControl]
            {
                onControl(cb, juce::var(e->getText()));
            };
        }
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
    view->gradientFrom = readColourProp(node, "gradientFrom");
    view->gradientTo = readColourProp(node, "gradientTo");
    view->gradientVertical = readNumberProp(node, "gradientVertical", 1.0) != 0.0;
    view->borderColour = readColourProp(node, "borderColor");
    view->borderWidth = static_cast<float>(readNumberProp(node, "borderWidth", 0.0));
    for (const auto& child : node.children)
    {
        auto builtChild = buildComponent(child, onControl);
        view->children.add(builtChild.get());
        view->addAndMakeVisible(*builtChild);
        builtChild.release();
    }

    return view;
}
}

void JuceRenderer::setControlCallback(std::function<void(const juce::String&, const juce::var&)> callback)
{
    onControl = std::move(callback);
}

void JuceRenderer::renderTo(juce::Component& target, const ElementNode& root)
{
    target.removeAllChildren();
    auto rootComponent = buildComponent(root, onControl);
    target.addAndMakeVisible(*rootComponent);
    rootComponent->setBounds(target.getLocalBounds());
    rootComponent.release();
}
}
