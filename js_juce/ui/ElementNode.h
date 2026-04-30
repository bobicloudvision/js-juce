#pragma once

#include <vector>

#include <juce_core/juce_core.h>

namespace js_juce
{
struct ElementNode
{
    juce::String type;
    juce::NamedValueSet props;
    std::vector<ElementNode> children;

    static ElementNode fromVar(const juce::var& value);
};

inline ElementNode ElementNode::fromVar(const juce::var& value)
{
    ElementNode node;
    const auto* obj = value.getDynamicObject();
    if (obj == nullptr)
        return node;

    node.type = obj->getProperty("type").toString();
    const auto propsVar = obj->getProperty("props");
    if (const auto* propsObject = propsVar.getDynamicObject())
    {
        for (const auto& p : propsObject->getProperties())
            node.props.set(p.name, p.value);
    }

    const auto childrenVar = obj->getProperty("children");
    if (const auto* arr = childrenVar.getArray())
    {
        for (const auto& child : *arr)
            node.children.push_back(ElementNode::fromVar(child));
    }

    if (node.type.isEmpty())
        node.type = "View";

    return node;
}
}
