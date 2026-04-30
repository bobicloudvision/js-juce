#include "JuceRenderer.h"
#include <optional>

namespace js_juce
{
namespace
{
struct ChildLayoutStyle
{
    float width = -1.0f;
    float minWidth = 0.0f;
    float maxWidth = -1.0f;
    float height = -1.0f;
    float minHeight = 0.0f;
    float maxHeight = -1.0f;
    float flexGrow = 1.0f;
    float flexShrink = 1.0f;
    float flexBasis = -2.0f;
    int order = 0;
    juce::FlexItem::AlignSelf alignSelf = juce::FlexItem::AlignSelf::autoAlign;
    juce::FlexItem::Margin margin;
};

static std::optional<juce::String> readOptionalTextProp(const ElementNode& node, const juce::String& name)
{
    const auto* value = node.props.getVarPointer(name);
    if (value == nullptr)
        return std::nullopt;

    const auto text = value->toString().trim();
    if (text.isEmpty())
        return std::nullopt;
    return text;
}

static std::optional<float> readOptionalNumberProp(const ElementNode& node, const juce::String& name)
{
    const auto* value = node.props.getVarPointer(name);
    if (value == nullptr)
        return std::nullopt;
    return static_cast<float>(static_cast<double>(*value));
}

static juce::FlexBox::Direction parseDirection(const juce::String& value, bool defaultRow)
{
    if (value == "row")
        return juce::FlexBox::Direction::row;
    if (value == "column")
        return juce::FlexBox::Direction::column;
    if (value == "row-reverse")
        return juce::FlexBox::Direction::rowReverse;
    if (value == "column-reverse")
        return juce::FlexBox::Direction::columnReverse;
    return defaultRow ? juce::FlexBox::Direction::row : juce::FlexBox::Direction::column;
}

static juce::FlexBox::Wrap parseWrap(const juce::String& value)
{
    if (value == "wrap")
        return juce::FlexBox::Wrap::wrap;
    if (value == "wrap-reverse")
        return juce::FlexBox::Wrap::wrapReverse;
    return juce::FlexBox::Wrap::noWrap;
}

static juce::FlexBox::JustifyContent parseJustify(const juce::String& value)
{
    if (value == "start" || value == "flex-start" || value == "left")
        return juce::FlexBox::JustifyContent::flexStart;
    if (value == "end" || value == "flex-end" || value == "right")
        return juce::FlexBox::JustifyContent::flexEnd;
    if (value == "center")
        return juce::FlexBox::JustifyContent::center;
    if (value == "space-between")
        return juce::FlexBox::JustifyContent::spaceBetween;
    if (value == "space-around")
        return juce::FlexBox::JustifyContent::spaceAround;
    return juce::FlexBox::JustifyContent::flexStart;
}

static juce::FlexBox::AlignItems parseAlignItems(const juce::String& value)
{
    if (value == "stretch")
        return juce::FlexBox::AlignItems::stretch;
    if (value == "start" || value == "flex-start")
        return juce::FlexBox::AlignItems::flexStart;
    if (value == "end" || value == "flex-end")
        return juce::FlexBox::AlignItems::flexEnd;
    if (value == "center")
        return juce::FlexBox::AlignItems::center;
    return juce::FlexBox::AlignItems::stretch;
}

static juce::FlexBox::AlignContent parseAlignContent(const juce::String& value)
{
    if (value == "start" || value == "flex-start")
        return juce::FlexBox::AlignContent::flexStart;
    if (value == "end" || value == "flex-end")
        return juce::FlexBox::AlignContent::flexEnd;
    if (value == "center")
        return juce::FlexBox::AlignContent::center;
    if (value == "space-between")
        return juce::FlexBox::AlignContent::spaceBetween;
    if (value == "space-around")
        return juce::FlexBox::AlignContent::spaceAround;
    if (value == "stretch")
        return juce::FlexBox::AlignContent::stretch;
    return juce::FlexBox::AlignContent::stretch;
}

static juce::FlexItem::AlignSelf parseAlignSelf(const juce::String& value)
{
    if (value == "start" || value == "flex-start")
        return juce::FlexItem::AlignSelf::flexStart;
    if (value == "end" || value == "flex-end")
        return juce::FlexItem::AlignSelf::flexEnd;
    if (value == "center")
        return juce::FlexItem::AlignSelf::center;
    if (value == "stretch")
        return juce::FlexItem::AlignSelf::stretch;
    return juce::FlexItem::AlignSelf::autoAlign;
}

static float readEdgeValue(const ElementNode& node, const juce::String& allName, const juce::String& edgeName, float fallbackValue)
{
    const auto allValue = readOptionalNumberProp(node, allName);
    const auto edgeValue = readOptionalNumberProp(node, edgeName);
    return edgeValue.value_or(allValue.value_or(fallbackValue));
}

static ChildLayoutStyle readChildLayoutStyle(const ElementNode& node)
{
    ChildLayoutStyle style;

    style.width = readOptionalNumberProp(node, "width").value_or(-1.0f);
    style.minWidth = readOptionalNumberProp(node, "minWidth").value_or(0.0f);
    style.maxWidth = readOptionalNumberProp(node, "maxWidth").value_or(-1.0f);
    style.height = readOptionalNumberProp(node, "height").value_or(-1.0f);
    style.minHeight = readOptionalNumberProp(node, "minHeight").value_or(0.0f);
    style.maxHeight = readOptionalNumberProp(node, "maxHeight").value_or(-1.0f);
    style.flexGrow = readOptionalNumberProp(node, "grow").value_or(1.0f);
    style.flexShrink = readOptionalNumberProp(node, "shrink").value_or(1.0f);
    style.flexBasis = readOptionalNumberProp(node, "basis").value_or(-2.0f);
    style.order = static_cast<int>(readOptionalNumberProp(node, "order").value_or(0.0f));
    style.alignSelf = parseAlignSelf(readOptionalTextProp(node, "alignSelf").value_or(juce::String()));

    style.margin.left = readEdgeValue(node, "margin", "marginLeft", 0.0f);
    style.margin.right = readEdgeValue(node, "margin", "marginRight", 0.0f);
    style.margin.top = readEdgeValue(node, "margin", "marginTop", 0.0f);
    style.margin.bottom = readEdgeValue(node, "margin", "marginBottom", 0.0f);

    return style;
}

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
        fb.flexDirection = parseDirection(direction, row);
        fb.flexWrap = parseWrap(wrap);
        fb.justifyContent = parseJustify(justify);
        fb.alignItems = parseAlignItems(alignItems);
        fb.alignContent = parseAlignContent(alignContent);

        for (int index = 0; index < children.size(); ++index)
        {
            auto* c = children[index];
            const auto& s = childStyles.getReference(index);
            auto item = juce::FlexItem(*c);
            item.width = s.width;
            item.minWidth = s.minWidth;
            item.maxWidth = s.maxWidth;
            item.height = s.height;
            item.minHeight = s.minHeight;
            item.maxHeight = s.maxHeight;
            item.flexGrow = s.flexGrow;
            item.flexShrink = s.flexShrink;
            item.flexBasis = s.flexBasis;
            item.order = s.order;
            item.alignSelf = s.alignSelf;
            item.margin = s.margin;

            if (gap > 0.0f || rowGap > 0.0f || columnGap > 0.0f)
            {
                const auto effectiveColumnGap = columnGap > 0.0f ? columnGap : gap;
                const auto effectiveRowGap = rowGap > 0.0f ? rowGap : gap;
                const auto isLastItem = index == children.size() - 1;
                if (!isLastItem)
                {
                    item.margin.right += effectiveColumnGap;
                    item.margin.bottom += effectiveRowGap;
                }
            }

            fb.items.add(item);
        }

        fb.performLayout(getLocalBounds().reduced(static_cast<int>(padding)).toFloat());
    }

    juce::Array<juce::Component*> children;
    juce::Array<ChildLayoutStyle> childStyles;
    bool row = false;
    std::optional<juce::Colour> backgroundColour;
    std::optional<juce::Colour> gradientFrom;
    std::optional<juce::Colour> gradientTo;
    bool gradientVertical = true;
    std::optional<juce::Colour> borderColour;
    float borderWidth = 0.0f;
    float padding = 0.0f;
    float gap = 0.0f;
    float rowGap = 0.0f;
    float columnGap = 0.0f;
    juce::String direction;
    juce::String wrap = "nowrap";
    juce::String justify = "flex-start";
    juce::String alignItems = "stretch";
    juce::String alignContent = "stretch";
};

class StyledTextButton final : public juce::TextButton
{
public:
    explicit StyledTextButton(const juce::String& buttonText) : juce::TextButton(buttonText) {}

    void setBorderStyle(std::optional<juce::Colour> colour, float width)
    {
        borderColour = colour;
        borderWidth = juce::jmax(0.0f, width);
        repaint();
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto baseColour = findColour(juce::TextButton::buttonColourId);
        if (shouldDrawButtonAsDown)
            baseColour = baseColour.brighter(0.12f);
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.06f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

        g.setColour(findColour(juce::TextButton::textColourOffId));
        g.setFont(juce::Font(14.0f));
        g.drawFittedText(getButtonText(), getLocalBounds().reduced(6, 2), juce::Justification::centred, 1);

        if (borderColour.has_value() && borderWidth > 0.0f)
        {
            g.setColour(*borderColour);
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(borderWidth * 0.5f),
                                   borderWidth,
                                   4.0f);
        }
    }

private:
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
        auto button = std::make_unique<StyledTextButton>(readTextProp(node, "text"));
        button->setButtonText(readTextProp(node, "text"));
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
        button->setBorderStyle(readColourProp(node, "borderColor"),
                               static_cast<float>(readNumberProp(node, "borderWidth", 0.0)));
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
    view->padding = static_cast<float>(readNumberProp(node, "padding", 0.0));
    view->gap = static_cast<float>(readNumberProp(node, "gap", 0.0));
    view->rowGap = static_cast<float>(readNumberProp(node, "rowGap", 0.0));
    view->columnGap = static_cast<float>(readNumberProp(node, "columnGap", 0.0));
    view->direction = readTextProp(node, "direction");
    view->wrap = readTextProp(node, "wrap");
    view->justify = readTextProp(node, "justify");
    view->alignItems = readTextProp(node, "alignItems");
    view->alignContent = readTextProp(node, "alignContent");
    for (const auto& child : node.children)
    {
        auto builtChild = buildComponent(child, onControl);
        view->children.add(builtChild.get());
        view->childStyles.add(readChildLayoutStyle(child));
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
