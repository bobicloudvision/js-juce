#include "JsRuntime.h"

#if __has_include(<quickjs.h>)
 #include <quickjs.h>
 #define JS_JUCE_HAS_QUICKJS 1
#else
 #define JS_JUCE_HAS_QUICKJS 0
#endif

namespace js_juce
{
class JsRuntime::Impl
{
public:
    std::function<void(const juce::var&)> onRender;

#if JS_JUCE_HAS_QUICKJS
    JSRuntime* runtime = nullptr;
    JSContext* context = nullptr;

    static JSValue jsCreateElement(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
    {
        JSValue element = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, element, "type", argc > 0 ? JS_DupValue(ctx, argv[0]) : JS_NewString(ctx, "View"));
        JS_SetPropertyStr(ctx, element, "props", argc > 1 ? JS_DupValue(ctx, argv[1]) : JS_NewObject(ctx));

        JSValue children = JS_NewArray(ctx);
        for (int i = 2; i < argc; ++i)
            JS_SetPropertyUint32(ctx, children, static_cast<uint32_t>(i - 2), JS_DupValue(ctx, argv[i]));
        JS_SetPropertyStr(ctx, element, "children", children);
        return element;
    }

    static juce::var parseJsonToVar(const juce::String& text)
    {
        juce::var out;
        juce::JSON::parse(text, out);
        return out;
    }

    static JSValue jsRender(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv)
    {
        juce::ignoreUnused(thisVal);
        if (argc < 1)
            return JS_UNDEFINED;

        auto* impl = static_cast<Impl*>(JS_GetContextOpaque(ctx));
        if (impl == nullptr || impl->onRender == nullptr)
            return JS_UNDEFINED;

        JSValue global = JS_GetGlobalObject(ctx);
        JSValue jsonObj = JS_GetPropertyStr(ctx, global, "JSON");
        JSValue stringify = JS_GetPropertyStr(ctx, jsonObj, "stringify");
        JSValue jsonString = JS_Call(ctx, stringify, jsonObj, 1, argv);

        size_t len = 0;
        const char* raw = JS_ToCStringLen(ctx, &len, jsonString);
        if (raw != nullptr)
        {
            const juce::String text = juce::String::fromUTF8(raw, static_cast<int>(len));
            impl->onRender(parseJsonToVar(text));
            JS_FreeCString(ctx, raw);
        }

        JS_FreeValue(ctx, jsonString);
        JS_FreeValue(ctx, stringify);
        JS_FreeValue(ctx, jsonObj);
        JS_FreeValue(ctx, global);
        return JS_UNDEFINED;
    }
#endif
};

JsRuntime::JsRuntime() : impl(std::make_unique<Impl>()) {}
JsRuntime::~JsRuntime() { shutdown(); }

bool JsRuntime::initialise()
{
#if JS_JUCE_HAS_QUICKJS
    impl->runtime = JS_NewRuntime();
    impl->context = JS_NewContext(impl->runtime);
    if (impl->runtime == nullptr || impl->context == nullptr)
        return false;

    JS_SetContextOpaque(impl->context, impl.get());
    JSValue global = JS_GetGlobalObject(impl->context);

    JS_SetPropertyStr(impl->context, global, "__jsJuceCreateElement",
                      JS_NewCFunction(impl->context, Impl::jsCreateElement, "__jsJuceCreateElement", 3));
    JS_SetPropertyStr(impl->context, global, "__jsJuceRender",
                      JS_NewCFunction(impl->context, Impl::jsRender, "__jsJuceRender", 1));

    const char* bootstrap = R"(
function __jsJuceNormalizeNode(node) {
  if (node == null) {
    return __jsJuceCreateElement("View", {});
  }

  if (node && typeof node.toElement === "function") {
    return __jsJuceNormalizeNode(node.toElement());
  }

  if (typeof node === "string" || typeof node === "number" || typeof node === "boolean") {
    return __jsJuceCreateElement("Text", { text: String(node) });
  }

  const children = Array.isArray(node.children) ? node.children.map(__jsJuceNormalizeNode) : [];
  return __jsJuceCreateElement(node.type || "View", node.props || {}, ...children);
}

class View {
  constructor(props = {}, children = []) {
    this.props = props || {};
    this.children = Array.isArray(children) ? children : [children];
  }
  type() { return "View"; }
  toElement() { return __jsJuceCreateElement(this.type(), this.props, ...this.children.map(__jsJuceNormalizeNode)); }
}

class Row extends View { type() { return "Row"; } }
class Column extends View { type() { return "Column"; } }

class Text extends View {
  constructor(text, props = {}) { super({ ...props, text }, []); }
  type() { return "Text"; }
}

class Button extends View {
  constructor(text, props = {}) { super({ ...props, text }, []); }
  type() { return "Button"; }
}

class Slider extends View {
  constructor(props = {}) { super(props, []); }
  type() { return "Slider"; }
}

class TextInput extends View {
  constructor(props = {}) { super(props, []); }
  type() { return "TextInput"; }
}

globalThis.JuceUI = {
  createElement: function(type, props, ...children) {
    return __jsJuceCreateElement(type, props || {}, ...children.map(__jsJuceNormalizeNode));
  },
  render: function(tree) {
    __jsJuceRender(__jsJuceNormalizeNode(tree));
  }
};

globalThis.View = View;
globalThis.Row = Row;
globalThis.Column = Column;
globalThis.Text = Text;
globalThis.Button = Button;
globalThis.Slider = Slider;
globalThis.TextInput = TextInput;
)";
    JS_Eval(impl->context, bootstrap, strlen(bootstrap), "<bootstrap>", JS_EVAL_TYPE_GLOBAL);
    JS_FreeValue(impl->context, global);
    return true;
#else
    return false;
#endif
}

void JsRuntime::shutdown()
{
#if JS_JUCE_HAS_QUICKJS
    if (impl->context != nullptr)
    {
        JS_FreeContext(impl->context);
        impl->context = nullptr;
    }

    if (impl->runtime != nullptr)
    {
        JS_FreeRuntime(impl->runtime);
        impl->runtime = nullptr;
    }
#endif
}

void JsRuntime::setRenderCallback(std::function<void(const juce::var&)> callback)
{
    impl->onRender = std::move(callback);
}

bool JsRuntime::evaluateFile(const juce::File& scriptFile, juce::String& errorMessage)
{
    if (! scriptFile.existsAsFile())
    {
        errorMessage = "Script file does not exist: " + scriptFile.getFullPathName();
        return false;
    }

#if JS_JUCE_HAS_QUICKJS
    if (impl->context == nullptr)
    {
        errorMessage = "QuickJS runtime is not initialised.";
        return false;
    }

    const auto code = scriptFile.loadFileAsString();
    const auto path = scriptFile.getFullPathName().toRawUTF8();
    auto result = JS_Eval(impl->context, code.toRawUTF8(), static_cast<size_t>(code.getNumBytesAsUTF8()), path, JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(result))
    {
        JSValue exception = JS_GetException(impl->context);
        const char* raw = JS_ToCString(impl->context, exception);
        errorMessage = raw != nullptr ? juce::String(raw) : "QuickJS exception";
        if (raw != nullptr)
            JS_FreeCString(impl->context, raw);
        JS_FreeValue(impl->context, exception);
        JS_FreeValue(impl->context, result);
        return false;
    }

    JS_FreeValue(impl->context, result);
    return true;
#else
    errorMessage = "QuickJS headers are missing. Add quickjs.h to your include paths.";
    return false;
#endif
}
}
