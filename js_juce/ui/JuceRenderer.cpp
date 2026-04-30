#include "JuceRenderer.h"
#include <algorithm>
#include <map>
#include <optional>
#include <vector>

namespace js_juce
{
namespace
{
struct CssLength
{
    enum class Kind
    {
        Auto,
        Px,
        Percent
    };

    Kind kind = Kind::Auto;
    float value = 0.0f;
};

static CssLength parseCssLength(const juce::var& value)
{
    CssLength out;
    if (value.isVoid() || value.isUndefined())
        return out;

    if (value.isString())
    {
        auto text = value.toString().trim();
        if (text.isEmpty() || text == "auto")
            return out;

        if (text.endsWithChar('%'))
        {
            out.kind = CssLength::Kind::Percent;
            out.value = static_cast<float>(text.dropLastCharacters(1).getDoubleValue()) / 100.0f;
            return out;
        }

        auto lower = text.toLowerCase();
        if (lower.endsWith("px"))
            lower = lower.dropLastCharacters(2).trim();

        out.kind = CssLength::Kind::Px;
        out.value = static_cast<float>(lower.getDoubleValue());
        return out;
    }

    if (value.isDouble() || value.isInt() || value.isInt64() || value.isBool())
    {
        out.kind = CssLength::Kind::Px;
        out.value = static_cast<float>(static_cast<double>(value));
        return out;
    }

    return out;
}

static float resolveCssLengthPx(const CssLength& spec, float referencePx)
{
    if (spec.kind == CssLength::Kind::Auto)
        return -1.0f;

    if (spec.kind == CssLength::Kind::Percent)
        return referencePx * spec.value;

    return spec.value;
}

static juce::Colour pickBorderPaintColour(std::optional<juce::Colour> edge, const juce::Colour& fallback)
{
    return edge.value_or(fallback);
}

static float readHorizontalExtras(const juce::BorderSize<int>& padding, const juce::BorderSize<int>& border)
{
    return static_cast<float>(padding.getLeft() + padding.getRight() + border.getLeft() + border.getRight());
}

static float readVerticalExtras(const juce::BorderSize<int>& padding, const juce::BorderSize<int>& border)
{
    return static_cast<float>(padding.getTop() + padding.getBottom() + border.getTop() + border.getBottom());
}

static float cssWidthToOuterPx(const CssLength& widthSpec,
                               float referencePx,
                               const juce::BorderSize<int>& padding,
                               const juce::BorderSize<int>& border,
                               bool borderBox)
{
    const auto resolved = resolveCssLengthPx(widthSpec, referencePx);
    if (resolved < 0.0f)
        return -1.0f;

    const auto extras = readHorizontalExtras(padding, border);
    return borderBox ? resolved : resolved + extras;
}

static float cssHeightToOuterPx(const CssLength& heightSpec,
                                float referencePx,
                                const juce::BorderSize<int>& padding,
                                const juce::BorderSize<int>& border,
                                bool borderBox)
{
    const auto resolved = resolveCssLengthPx(heightSpec, referencePx);
    if (resolved < 0.0f)
        return -1.0f;

    const auto extras = readVerticalExtras(padding, border);
    return borderBox ? resolved : resolved + extras;
}

struct ChildLayoutStyle
{
    CssLength widthSpec;
    CssLength heightSpec;
    float minWidth = 0.0f;
    float maxWidth = -1.0f;
    float minHeight = 0.0f;
    float maxHeight = -1.0f;
    float flexGrow = 0.0f;
    float flexShrink = 1.0f;
    float flexBasis = -2.0f;
    int order = 0;
    juce::FlexItem::AlignSelf alignSelf = juce::FlexItem::AlignSelf::autoAlign;
    juce::FlexItem::Margin margin;
    juce::String display = "block";
    juce::BorderSize<int> padding;
    juce::BorderSize<int> border;
    bool boxSizingBorderBox = false;
    juce::String position = "static";
    CssLength topSpec;
    CssLength rightSpec;
    CssLength bottomSpec;
    CssLength leftSpec;
    int zIndex = 0;
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

static juce::File resolveFontFile(const juce::String& path)
{
    DBG("[js_juce][font] resolve request: " + path);

    juce::File direct(path);
    if (juce::File::isAbsolutePath(path) && direct.existsAsFile())
    {
        DBG("[js_juce][font] resolved absolute: " + direct.getFullPathName());
        return direct;
    }

    const auto cwdCandidate = juce::File::getCurrentWorkingDirectory().getChildFile(path);
    if (cwdCandidate.existsAsFile())
    {
        DBG("[js_juce][font] resolved cwd: " + cwdCandidate.getFullPathName());
        return cwdCandidate;
    }

    const auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    const auto exeCandidate = exeDir.getChildFile(path);
    if (exeCandidate.existsAsFile())
    {
        DBG("[js_juce][font] resolved exe dir: " + exeCandidate.getFullPathName());
        return exeCandidate;
    }

    const auto exeParentCandidate = exeDir.getParentDirectory().getChildFile(path);
    if (exeParentCandidate.existsAsFile())
    {
        DBG("[js_juce][font] resolved exe parent: " + exeParentCandidate.getFullPathName());
        return exeParentCandidate;
    }

    // Finder-launched app bundles often have a cwd unrelated to the project.
    // Walk up from the executable directory and try the relative font path at each level.
    auto cursor = exeDir;
    for (int i = 0; i < 16; ++i)
    {
        const auto candidate = cursor.getChildFile(path);
        if (candidate.existsAsFile())
        {
            DBG("[js_juce][font] resolved by upward search: " + candidate.getFullPathName());
            return candidate;
        }

        const auto parent = cursor.getParentDirectory();
        if (parent == cursor)
            break;
        cursor = parent;
    }

    DBG("[js_juce][font] resolve failed for: " + path);
    DBG("[js_juce][font] checked cwd candidate: " + cwdCandidate.getFullPathName());
    DBG("[js_juce][font] checked exe candidate: " + exeCandidate.getFullPathName());
    DBG("[js_juce][font] checked exe parent candidate: " + exeParentCandidate.getFullPathName());

    return {};
}

static juce::Font applyFontProps(const ElementNode& node, juce::Font font)
{
    bool appliedTypefaceFromFile = false;

    if (const auto fontFile = readOptionalTextProp(node, "fontFile"))
    {
        DBG("[js_juce][font] applying fontFile: " + *fontFile);
        static std::map<juce::String, juce::Typeface::Ptr> loadedTypefaces;
        auto it = loadedTypefaces.find(*fontFile);
        if (it == loadedTypefaces.end())
        {
            const auto file = resolveFontFile(*fontFile);
            if (file.existsAsFile())
            {
                DBG("[js_juce][font] loading file: " + file.getFullPathName());
                juce::MemoryBlock data;
                if (file.loadFileAsData(data))
                {
                    DBG("[js_juce][font] file loaded bytes: " + juce::String(static_cast<int>(data.getSize())));
                    if (auto typeface = juce::Typeface::createSystemTypefaceFor(data.getData(), data.getSize()))
                    {
                        DBG("[js_juce][font] typeface created: " + typeface->getName() + " / " + typeface->getStyle());
                        it = loadedTypefaces.emplace(*fontFile, std::move(typeface)).first;
                    }
                    else
                    {
                        DBG("[js_juce][font] ERROR: createSystemTypefaceFor failed for file: " + file.getFullPathName());
                    }
                }
                else
                {
                    DBG("[js_juce][font] ERROR: loadFileAsData failed for file: " + file.getFullPathName());
                }
            }
            else
            {
                DBG("[js_juce][font] ERROR: resolved file does not exist for path: " + *fontFile);
            }
        }
        else
        {
            DBG("[js_juce][font] cache hit for fontFile: " + *fontFile);
        }

        if (it != loadedTypefaces.end() && it->second != nullptr)
        {
            const auto previousHeight = font.getHeight();
            juce::FontOptions options(it->second);
            options = options.withHeight(previousHeight);
            font = juce::Font(options);
            appliedTypefaceFromFile = true;
            font.setHeight(previousHeight);
            DBG("[js_juce][font] applied typeface from file: " + it->second->getName());
        }
        else
        {
            DBG("[js_juce][font] WARNING: no loaded typeface available for: " + *fontFile);
        }
    }

    if (!appliedTypefaceFromFile)
    {
        juce::String chosenFamily;

        if (const auto* famVar = node.props.getVarPointer("fontFamilies"))
        {
            if (const auto* arr = famVar->getArray())
            {
                for (const auto& item : *arr)
                {
                    const auto piece = item.toString().trim();
                    if (piece.isNotEmpty())
                    {
                        chosenFamily = piece;
                        break;
                    }
                }
            }
        }

        if (chosenFamily.isEmpty())
        {
            if (const auto family = readOptionalTextProp(node, "fontFamily"))
            {
                juce::StringArray tokens;
                tokens.addTokens(*family, ",", "\"'");
                for (const auto& token : tokens)
                {
                    const auto trimmed = token.trim();
                    if (trimmed.isNotEmpty())
                    {
                        chosenFamily = trimmed;
                        break;
                    }
                }
            }
        }

        if (chosenFamily.isNotEmpty() && font.getTypefaceName() != chosenFamily)
        {
            font.setTypefaceName(chosenFamily);
            DBG("[js_juce][font] applied fontFamily: " + chosenFamily);
        }
    }

    if (const auto size = readOptionalNumberProp(node, "fontSize"))
    {
        font.setHeight(*size);
        DBG("[js_juce][font] applied fontSize: " + juce::String(*size));
    }

    if (const auto style = readOptionalTextProp(node, "fontStyle"))
    {
        const auto normalized = style->toLowerCase();
        if (normalized == "italic" || normalized == "oblique")
            font.setItalic(true);
        else if (normalized == "normal")
            font.setItalic(false);
    }

    if (const auto weightText = readOptionalTextProp(node, "fontWeight"))
    {
        const auto normalized = weightText->trim().toLowerCase();
        if (normalized.containsOnly("0123456789"))
        {
            const int numeric = normalized.getIntValue();
            if (numeric >= 100 && numeric <= 900)
                font.setBold(numeric >= 600);
        }
        else
        {
            const bool isBold = normalized == "bold"
                                || normalized == "bolder"
                                || normalized == "600"
                                || normalized == "700"
                                || normalized == "800"
                                || normalized == "900";
            font.setBold(isBold);
        }
        DBG("[js_juce][font] applied fontWeight: " + *weightText);
    }
    else if (const auto weightNumber = readOptionalNumberProp(node, "fontWeight"))
    {
        const int numeric = static_cast<int>(*weightNumber);
        if (numeric >= 100 && numeric <= 900)
            font.setBold(numeric >= 600);
    }

    if (const auto lh = readOptionalNumberProp(node, "lineHeight"))
    {
        const auto current = font.getHeight();
        if (*lh >= 4.0f)
            font.setHeight(*lh);
        else if (*lh > 0.0f)
            font.setHeight(current * *lh);
    }

    if (const auto ls = readOptionalNumberProp(node, "letterSpacing"))
        font = font.withExtraKerningFactor(*ls * 0.02f);

    return font;
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

static juce::BorderSize<int> readPaddingInsets(const ElementNode& node)
{
    const auto all = readOptionalNumberProp(node, "padding").value_or(0.0f);
    const auto top = readEdgeValue(node, "padding", "paddingTop", all);
    const auto right = readEdgeValue(node, "padding", "paddingRight", all);
    const auto bottom = readEdgeValue(node, "padding", "paddingBottom", all);
    const auto left = readEdgeValue(node, "padding", "paddingLeft", all);
    return { static_cast<int>(top), static_cast<int>(left), static_cast<int>(bottom), static_cast<int>(right) };
}

static juce::BorderSize<int> readBorderWidthInsets(const ElementNode& node)
{
    const auto all = readOptionalNumberProp(node, "borderWidth").value_or(0.0f);
    const auto top = readEdgeValue(node, "borderWidth", "borderTopWidth", all);
    const auto right = readEdgeValue(node, "borderWidth", "borderRightWidth", all);
    const auto bottom = readEdgeValue(node, "borderWidth", "borderBottomWidth", all);
    const auto left = readEdgeValue(node, "borderWidth", "borderLeftWidth", all);
    return { static_cast<int>(top), static_cast<int>(left), static_cast<int>(bottom), static_cast<int>(right) };
}

static bool readBoxSizingBorderBox(const ElementNode& node)
{
    const auto text = readOptionalTextProp(node, "boxSizing").value_or("content-box").toLowerCase();
    return text == "border-box";
}

static ChildLayoutStyle readChildLayoutStyle(const ElementNode& node)
{
    ChildLayoutStyle style;

    if (const auto* widthVar = node.props.getVarPointer("width"))
        style.widthSpec = parseCssLength(*widthVar);
    else if (const auto w = readOptionalNumberProp(node, "width"))
        style.widthSpec = *w < 0.0f ? CssLength{} : CssLength{ CssLength::Kind::Px, *w };

    if (const auto* heightVar = node.props.getVarPointer("height"))
        style.heightSpec = parseCssLength(*heightVar);
    else if (const auto h = readOptionalNumberProp(node, "height"))
        style.heightSpec = *h < 0.0f ? CssLength{} : CssLength{ CssLength::Kind::Px, *h };

    style.minWidth = readOptionalNumberProp(node, "minWidth").value_or(0.0f);
    style.maxWidth = readOptionalNumberProp(node, "maxWidth").value_or(-1.0f);
    style.minHeight = readOptionalNumberProp(node, "minHeight").value_or(0.0f);
    style.maxHeight = readOptionalNumberProp(node, "maxHeight").value_or(-1.0f);
    style.flexGrow = readOptionalNumberProp(node, "grow").value_or(0.0f);
    style.flexShrink = readOptionalNumberProp(node, "shrink").value_or(1.0f);
    style.flexBasis = readOptionalNumberProp(node, "basis").value_or(-2.0f);
    style.order = static_cast<int>(readOptionalNumberProp(node, "order").value_or(0.0f));
    style.alignSelf = parseAlignSelf(readOptionalTextProp(node, "alignSelf").value_or(juce::String()));
    style.display = readOptionalTextProp(node, "display").value_or("block").toLowerCase();
    style.padding = readPaddingInsets(node);
    style.border = readBorderWidthInsets(node);
    style.boxSizingBorderBox = readBoxSizingBorderBox(node);
    style.position = readOptionalTextProp(node, "position").value_or("static").toLowerCase();
    if (const auto* v = node.props.getVarPointer("top"))
        style.topSpec = parseCssLength(*v);
    if (const auto* v = node.props.getVarPointer("right"))
        style.rightSpec = parseCssLength(*v);
    if (const auto* v = node.props.getVarPointer("bottom"))
        style.bottomSpec = parseCssLength(*v);
    if (const auto* v = node.props.getVarPointer("left"))
        style.leftSpec = parseCssLength(*v);
    style.zIndex = static_cast<int>(readOptionalNumberProp(node, "zIndex").value_or(0.0f));

    style.margin.left = readEdgeValue(node, "margin", "marginLeft", 0.0f);
    style.margin.right = readEdgeValue(node, "margin", "marginRight", 0.0f);
    style.margin.top = readEdgeValue(node, "margin", "marginTop", 0.0f);
    style.margin.bottom = readEdgeValue(node, "margin", "marginBottom", 0.0f);

    // Intrinsic minimums for leaf controls when no explicit size is provided.
    // This prevents common JSX cases (e.g. <text>Title</text>) from collapsing.
    if (style.minWidth <= 0.0f && style.minHeight <= 0.0f)
    {
        if (node.type == "Text")
        {
            const auto text = readOptionalTextProp(node, "text").value_or(juce::String());
            auto font = applyFontProps(node, juce::Font(juce::FontOptions(14.0f)));
            const auto w = juce::jmax(8.0f, font.getStringWidthFloat(text) + 6.0f);
            const auto h = juce::jmax(14.0f, font.getHeight() + 4.0f);
            style.minWidth = juce::jmax(style.minWidth, w);
            style.minHeight = juce::jmax(style.minHeight, h);
        }
        else if (node.type == "Button")
        {
            const auto text = readOptionalTextProp(node, "text").value_or(juce::String());
            auto font = applyFontProps(node, juce::Font(juce::FontOptions(14.0f)));
            const auto w = juce::jmax(56.0f, font.getStringWidthFloat(text) + 24.0f);
            const auto h = juce::jmax(24.0f, font.getHeight() + 10.0f);
            style.minWidth = juce::jmax(style.minWidth, w);
            style.minHeight = juce::jmax(style.minHeight, h);
        }
        else if (node.type == "TextInput")
        {
            const auto text = readOptionalTextProp(node, "text").value_or(juce::String());
            auto font = applyFontProps(node, juce::Font(juce::FontOptions(14.0f)));
            const auto w = juce::jmax(80.0f, font.getStringWidthFloat(text) + 20.0f);
            const auto h = juce::jmax(24.0f, font.getHeight() + 10.0f);
            style.minWidth = juce::jmax(style.minWidth, w);
            style.minHeight = juce::jmax(style.minHeight, h);
        }
        else if (node.type == "Slider")
        {
            style.minWidth = juce::jmax(style.minWidth, 120.0f);
            style.minHeight = juce::jmax(style.minHeight, 24.0f);
        }
    }

    return style;
}

class ViewComponent final : public juce::Component
{
public:
    explicit ViewComponent(bool isRow) : row(isRow) {}

    static bool isOutOfFlow(const ChildLayoutStyle& s)
    {
        return s.position == "absolute" || s.position == "fixed";
    }

    juce::Rectangle<int> paddingBox() const
    {
        auto r = getLocalBounds();
        return borderInsets.subtractedFrom(r);
    }

    juce::Rectangle<int> contentBoxForChildren() const
    {
        return paddingInsets.subtractedFrom(paddingBox());
    }

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

        drawBorderEdges(g);
    }

    void paintOverChildren(juce::Graphics& g) override
    {
        if (!debugLayout)
            return;

        g.setColour(juce::Colours::limegreen.withAlpha(0.45f));
        for (auto* c : getChildren())
        {
            if (c == nullptr || !c->isVisible())
                continue;
            g.drawRect(c->getBounds(), 1);
        }
    }

    void resized() override
    {
        if (display == "none")
            return;

        const auto inner = contentBoxForChildren();
        const bool useFlex = display == "flex" || row;

        if (!useFlex)
        {
            layoutAsBlock(inner);
            layoutAbsoluteChildren(inner);
            applyRelativeOffsets(inner);
            applyChildZOrder();
            return;
        }

        juce::FlexBox fb;
        fb.flexDirection = parseDirection(direction, row);
        fb.flexWrap = parseWrap(wrap);
        fb.justifyContent = parseJustify(justify);
        fb.alignItems = parseAlignItems(alignItems);
        fb.alignContent = parseAlignContent(alignContent);

        const float innerW = static_cast<float>(juce::jmax(0, inner.getWidth()));
        const float innerH = static_cast<float>(juce::jmax(0, inner.getHeight()));

        for (int index = 0; index < children.size(); ++index)
        {
            auto* c = children[index];
            const auto& s = childStyles.getReference(index);
            if (s.display == "none")
            {
                c->setVisible(false);
                continue;
            }

            if (isOutOfFlow(s))
            {
                c->setVisible(true);
                continue;
            }

            c->setVisible(true);
            auto item = juce::FlexItem(*c);
            const auto outerW = cssWidthToOuterPx(s.widthSpec, innerW, s.padding, s.border, s.boxSizingBorderBox);
            const auto outerH = cssHeightToOuterPx(s.heightSpec, innerH, s.padding, s.border, s.boxSizingBorderBox);
            item.width = outerW >= 0.0f ? outerW : -1.0f;
            item.height = outerH >= 0.0f ? outerH : -1.0f;
            item.minWidth = s.minWidth;
            item.maxWidth = s.maxWidth;
            item.minHeight = s.minHeight;
            item.maxHeight = s.maxHeight;
            item.flexGrow = s.flexGrow;
            item.flexShrink = s.flexShrink;
            item.flexBasis = s.flexBasis;
            item.order = s.order;
            item.alignSelf = s.alignSelf;
            item.margin = s.margin;

            // JUCE FlexItem does not infer intrinsic content width for our custom
            // container components (View/Row/Column). If a container child has no
            // explicit width/basis and grow=0, it can collapse to zero width.
            // Keep these visible by allowing them to participate in row growth.
            if (item.width < 0.0f && item.flexBasis < 0.0f && item.flexGrow == 0.0f)
                if (dynamic_cast<ViewComponent*>(c) != nullptr)
                    item.flexGrow = 1.0f;

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

        fb.performLayout(inner.toFloat());
        layoutAbsoluteChildren(inner);
        applyRelativeOffsets(inner);
        applyChildZOrder();
    }

    void drawBorderEdges(juce::Graphics& g) const
    {
        const auto full = getLocalBounds();
        const auto top = borderInsets.getTop();
        const auto left = borderInsets.getLeft();
        const auto bottom = borderInsets.getBottom();
        const auto right = borderInsets.getRight();

        if (top <= 0 && left <= 0 && bottom <= 0 && right <= 0)
            return;

        const juce::Colour fallback = borderColour.value_or(juce::Colours::transparentBlack);
        const auto cTop = pickBorderPaintColour(borderTopColour, fallback);
        const auto cRight = pickBorderPaintColour(borderRightColour, fallback);
        const auto cBottom = pickBorderPaintColour(borderBottomColour, fallback);
        const auto cLeft = pickBorderPaintColour(borderLeftColour, fallback);

        if (top > 0)
        {
            g.setColour(cTop);
            g.fillRect(0, 0, full.getWidth(), top);
        }
        if (bottom > 0)
        {
            g.setColour(cBottom);
            g.fillRect(0, full.getBottom() - bottom, full.getWidth(), bottom);
        }
        if (left > 0)
        {
            g.setColour(cLeft);
            g.fillRect(0, top, left, full.getHeight() - top - bottom);
        }
        if (right > 0)
        {
            g.setColour(cRight);
            g.fillRect(full.getRight() - right, top, right, full.getHeight() - top - bottom);
        }
    }

    void layoutAsBlock(const juce::Rectangle<int>& content)
    {
        int y = content.getY();

        for (int index = 0; index < children.size(); ++index)
        {
            auto* c = children[index];
            const auto& s = childStyles.getReference(index);
            if (s.display == "none")
            {
                c->setVisible(false);
                continue;
            }

            if (isOutOfFlow(s))
            {
                c->setVisible(true);
                continue;
            }

            c->setVisible(true);

            const int marginTop = static_cast<int>(s.margin.top);
            const int marginRight = static_cast<int>(s.margin.right);
            const int marginBottom = static_cast<int>(s.margin.bottom);
            const int marginLeft = static_cast<int>(s.margin.left);

            y += marginTop;

            const float innerW = static_cast<float>(juce::jmax(0, content.getWidth() - marginLeft - marginRight));
            const float innerH = static_cast<float>(juce::jmax(0, content.getHeight()));

            float outerW = cssWidthToOuterPx(s.widthSpec, innerW, s.padding, s.border, s.boxSizingBorderBox);
            if (outerW < 0.0f)
                outerW = static_cast<float>(juce::jmax(0, content.getWidth() - marginLeft - marginRight));

            int childWidth = static_cast<int>(outerW);
            if (s.maxWidth >= 0.0f)
                childWidth = juce::jmin(childWidth, static_cast<int>(s.maxWidth));
            childWidth = juce::jmax(static_cast<int>(s.minWidth), childWidth);
            childWidth = juce::jmax(0, childWidth);

            float outerH = cssHeightToOuterPx(s.heightSpec, innerH, s.padding, s.border, s.boxSizingBorderBox);
            if (outerH < 0.0f)
                outerH = 30.0f;

            int childHeight = static_cast<int>(outerH);
            if (s.maxHeight >= 0.0f)
                childHeight = juce::jmin(childHeight, static_cast<int>(s.maxHeight));
            childHeight = juce::jmax(static_cast<int>(s.minHeight), childHeight);
            childHeight = juce::jmax(0, childHeight);

            c->setBounds(content.getX() + marginLeft, y, childWidth, childHeight);
            y += childHeight + marginBottom;
        }
    }

    void layoutAbsoluteChildren(const juce::Rectangle<int>& containingBlock)
    {
        const float cw = static_cast<float>(juce::jmax(0, containingBlock.getWidth()));
        const float ch = static_cast<float>(juce::jmax(0, containingBlock.getHeight()));

        for (int index = 0; index < children.size(); ++index)
        {
            auto* c = children[index];
            const auto& s = childStyles.getReference(index);
            if (s.display == "none" || !isOutOfFlow(s))
                continue;

            float outerW = cssWidthToOuterPx(s.widthSpec, cw, s.padding, s.border, s.boxSizingBorderBox);
            if (outerW < 0.0f)
                outerW = static_cast<float>(c->getWidth() > 0 ? c->getWidth() : 100);

            float outerH = cssHeightToOuterPx(s.heightSpec, ch, s.padding, s.border, s.boxSizingBorderBox);
            if (outerH < 0.0f)
                outerH = static_cast<float>(c->getHeight() > 0 ? c->getHeight() : 30);

            int w = static_cast<int>(outerW);
            int h = static_cast<int>(outerH);
            if (s.maxWidth >= 0.0f)
                w = juce::jmin(w, static_cast<int>(s.maxWidth));
            w = juce::jmax(static_cast<int>(s.minWidth), w);
            if (s.maxHeight >= 0.0f)
                h = juce::jmin(h, static_cast<int>(s.maxHeight));
            h = juce::jmax(static_cast<int>(s.minHeight), h);

            const float leftPx = resolveCssLengthPx(s.leftSpec, cw);
            const float rightPx = resolveCssLengthPx(s.rightSpec, cw);
            const float topPx = resolveCssLengthPx(s.topSpec, ch);
            const float bottomPx = resolveCssLengthPx(s.bottomSpec, ch);

            int x = containingBlock.getX();
            int y = containingBlock.getY();

            if (leftPx >= 0.0f)
                x += static_cast<int>(leftPx);
            else if (rightPx >= 0.0f)
                x = containingBlock.getRight() - w - static_cast<int>(rightPx);

            if (topPx >= 0.0f)
                y += static_cast<int>(topPx);
            else if (bottomPx >= 0.0f)
                y = containingBlock.getBottom() - h - static_cast<int>(bottomPx);

            c->setBounds(x, y, w, h);
        }
    }

    void applyRelativeOffsets(const juce::Rectangle<int>& containingBlock)
    {
        (void)containingBlock;

        for (int index = 0; index < children.size(); ++index)
        {
            auto* c = children[index];
            const auto& s = childStyles.getReference(index);
            if (s.display == "none" || s.position != "relative")
                continue;

            const auto b = c->getBounds();
            const float dx = resolveCssLengthPx(s.leftSpec, static_cast<float>(b.getWidth()));
            const float dy = resolveCssLengthPx(s.topSpec, static_cast<float>(b.getHeight()));
            int ox = 0;
            int oy = 0;
            if (dx >= 0.0f)
                ox = static_cast<int>(dx);
            if (dy >= 0.0f)
                oy = static_cast<int>(dy);
            c->setBounds(b.translated(ox, oy));
        }
    }

    void applyChildZOrder()
    {
        std::vector<std::pair<int, int>> order;
        order.reserve(static_cast<size_t>(children.size()));
        for (int i = 0; i < children.size(); ++i)
            order.emplace_back(childStyles.getReference(i).zIndex, i);

        std::stable_sort(order.begin(), order.end(), [](const auto& a, const auto& b)
                         {
                             return a.first < b.first;
                         });

        for (const auto& entry : order)
        {
            auto* c = children[entry.second];
            if (c != nullptr)
                c->toFront(false);
        }
    }

    juce::Array<juce::Component*> children;
    juce::Array<ChildLayoutStyle> childStyles;
    bool row = false;
    std::optional<juce::Colour> backgroundColour;
    std::optional<juce::Colour> gradientFrom;
    std::optional<juce::Colour> gradientTo;
    bool gradientVertical = true;
    std::optional<juce::Colour> borderColour;
    std::optional<juce::Colour> borderTopColour;
    std::optional<juce::Colour> borderRightColour;
    std::optional<juce::Colour> borderBottomColour;
    std::optional<juce::Colour> borderLeftColour;
    juce::BorderSize<int> borderInsets;
    juce::BorderSize<int> paddingInsets;
    bool debugLayout = false;
    float gap = 0.0f;
    float rowGap = 0.0f;
    float columnGap = 0.0f;
    juce::String direction;
    juce::String display = "block";
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

    void setTextFont(juce::Font fontToUse)
    {
        textFont = std::move(fontToUse);
        repaint();
    }

    void setTextJustification(juce::Justification j)
    {
        textJustification = j;
        repaint();
    }

    void setHoverStyle(std::optional<juce::Colour> background,
                       std::optional<juce::Colour> text,
                       std::optional<juce::Colour> border)
    {
        hoverBackgroundColour = background;
        hoverTextColour = text;
        hoverBorderColour = border;
        repaint();
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto baseColour = findColour(juce::TextButton::buttonColourId);
        if (shouldDrawButtonAsDown)
            baseColour = baseColour.brighter(0.12f);
        else if (shouldDrawButtonAsHighlighted && hoverBackgroundColour.has_value())
            baseColour = *hoverBackgroundColour;
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.06f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

        auto textColour = findColour(juce::TextButton::textColourOffId);
        if (shouldDrawButtonAsHighlighted && hoverTextColour.has_value())
            textColour = *hoverTextColour;
        g.setColour(textColour);
        g.setFont(textFont);
        g.drawFittedText(getButtonText(), getLocalBounds().reduced(6, 2), textJustification, 1);

        auto resolvedBorderColour = borderColour;
        if (shouldDrawButtonAsHighlighted && hoverBorderColour.has_value())
            resolvedBorderColour = hoverBorderColour;

        if (resolvedBorderColour.has_value() && borderWidth > 0.0f)
        {
            g.setColour(*resolvedBorderColour);
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(borderWidth * 0.5f),
                                   borderWidth,
                                   4.0f);
        }
    }

private:
    std::optional<juce::Colour> borderColour;
    std::optional<juce::Colour> hoverBackgroundColour;
    std::optional<juce::Colour> hoverTextColour;
    std::optional<juce::Colour> hoverBorderColour;
    float borderWidth = 0.0f;
    juce::Font textFont { juce::FontOptions(14.0f) };
    juce::Justification textJustification { juce::Justification::centred };
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

    const auto lower = text.toLowerCase();
    if (lower.startsWith("rgba(") || lower.startsWith("rgb("))
    {
        const bool hasAlpha = lower.startsWith("rgba(");
        const int openIdx = text.indexOfChar('(');
        const int closeIdx = text.lastIndexOfChar(')');
        if (openIdx > 0 && closeIdx > openIdx)
        {
            const auto inside = text.substring(openIdx + 1, closeIdx);
            juce::StringArray parts;
            parts.addTokens(inside, ",", "");
            parts.trim();
            parts.removeEmptyStrings();

            if ((hasAlpha && parts.size() == 4) || (!hasAlpha && parts.size() == 3))
            {
                const auto r = juce::jlimit(0, 255, parts[0].getIntValue());
                const auto g = juce::jlimit(0, 255, parts[1].getIntValue());
                const auto b = juce::jlimit(0, 255, parts[2].getIntValue());
                juce::uint8 alpha = 255;

                if (hasAlpha)
                {
                    const auto aRaw = static_cast<float>(parts[3].getDoubleValue());
                    const auto aNorm = juce::jlimit(0.0f, 1.0f, aRaw);
                    alpha = static_cast<juce::uint8>(juce::roundToInt(aNorm * 255.0f));
                }

                return juce::Colour(static_cast<juce::uint8>(r),
                                    static_cast<juce::uint8>(g),
                                    static_cast<juce::uint8>(b),
                                    alpha);
            }
        }
    }

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

static juce::Justification parseTextAlign(const juce::String& rawText)
{
    const auto s = rawText.trim().toLowerCase();
    if (s == "center" || s == "centre")
        return juce::Justification::centred;
    if (s == "right" || s == "end")
        return juce::Justification::centredRight;
    if (s == "justify")
        return juce::Justification::horizontallyJustified;
    return juce::Justification::centredLeft;
}

static std::unique_ptr<juce::Component> buildComponent(
    const ElementNode& node,
    const std::function<void(const juce::String&, const juce::var&)>& onControl)
{
    if (node.type == "Text")
    {
        auto label = std::make_unique<juce::Label>();
        label->setText(readTextProp(node, "text"), juce::dontSendNotification);
        label->setJustificationType(parseTextAlign(readTextProp(node, "textAlign")));
        label->setFont(applyFontProps(node, label->getFont()));
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
        button->setTextFont(applyFontProps(node, juce::Font(juce::FontOptions(14.0f))));
        button->setTextJustification(parseTextAlign(readTextProp(node, "textAlign")));
        button->setBorderStyle(readColourProp(node, "borderColor"),
                               static_cast<float>(readNumberProp(node, "borderWidth", 0.0)));
        button->setHoverStyle(readColourProp(node, "hoverBackground"),
                              readColourProp(node, "hoverColor"),
                              readColourProp(node, "hoverBorderColor"));
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
        editor->applyFontToAllText(applyFontProps(node, editor->getFont()));
        editor->setJustification(parseTextAlign(readTextProp(node, "textAlign")));
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
    view->borderTopColour = readColourProp(node, "borderTopColor");
    view->borderRightColour = readColourProp(node, "borderRightColor");
    view->borderBottomColour = readColourProp(node, "borderBottomColor");
    view->borderLeftColour = readColourProp(node, "borderLeftColor");
    view->borderInsets = readBorderWidthInsets(node);
    view->paddingInsets = readPaddingInsets(node);
    view->debugLayout = readOptionalNumberProp(node, "debugLayout").value_or(0.0f) != 0.0f;
    view->gap = static_cast<float>(readNumberProp(node, "gap", 0.0));
    view->rowGap = static_cast<float>(readNumberProp(node, "rowGap", 0.0));
    view->columnGap = static_cast<float>(readNumberProp(node, "columnGap", 0.0));
    view->direction = readTextProp(node, "direction");
    view->display = readOptionalTextProp(node, "display").value_or(node.type == "View" ? "block" : "flex").toLowerCase();
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
