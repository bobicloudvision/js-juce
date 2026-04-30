#include "JsRuntime.h"

#if __has_include(<quickjs.h>)
 #include <quickjs.h>
 #define JS_JUCE_HAS_QUICKJS 1
#elif __has_include("quickjs.h")
 #include "quickjs.h"
 #define JS_JUCE_HAS_QUICKJS 1
#elif __has_include("../../ext/quickjs/quickjs.h")
 #include "../../ext/quickjs/quickjs.h"
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
const __jsJuceStyleContextStack = [Object.create(null)];

function __jsJuceCurrentStyleContext() {
  return __jsJuceStyleContextStack[__jsJuceStyleContextStack.length - 1];
}

function __jsJuceResolveContextStyle(typeName, ownProps) {
  const scope = __jsJuceCurrentStyleContext();
  const defaults = (scope && scope[typeName]) || {};
  return { ...defaults, ...(ownProps || {}) };
}

function __jsJuceFlattenChildren(input, out = []) {
  if (Array.isArray(input)) {
    for (const item of input) __jsJuceFlattenChildren(item, out);
    return out;
  }
  if (input !== undefined && input !== null) out.push(input);
  return out;
}

function __jsJuceNormalizeStyleProps(props) {
  const source = props || {};
  const style = source.style || {};
  const merged = { ...source };

  if (style.backgroundColor !== undefined) merged.background = style.backgroundColor;
  if (style.gradientFrom !== undefined) merged.gradientFrom = style.gradientFrom;
  if (style.gradientTo !== undefined) merged.gradientTo = style.gradientTo;
  if (style.gradientVertical !== undefined) merged.gradientVertical = style.gradientVertical ? 1 : 0;
  if (style.borderColor !== undefined) merged.borderColor = style.borderColor;
  if (style.borderWidth !== undefined) merged.borderWidth = style.borderWidth;
  if (style.color !== undefined) merged.color = style.color;
  if (style.background !== undefined) merged.background = style.background;

  delete merged.style;
  return merged;
}

let __jsJuceNextCallbackId = 1;
const __jsJuceControlCallbacks = Object.create(null);

function __jsJuceWrapFunctionsForTransport(input) {
  if (Array.isArray(input)) return input.map(__jsJuceWrapFunctionsForTransport);
  if (input && typeof input === "object") {
    const out = {};
    for (const k of Object.keys(input)) out[k] = __jsJuceWrapFunctionsForTransport(input[k]);
    return out;
  }
  if (typeof input === "function") {
    const id = "cb_" + (__jsJuceNextCallbackId++);
    __jsJuceControlCallbacks[id] = input;
    return { __fnId: id };
  }
  return input;
}

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

  const rawChildren = Array.isArray(node.children) ? node.children : [];
  const children = __jsJuceFlattenChildren(rawChildren).map(__jsJuceNormalizeNode);
  return __jsJuceCreateElement(
    node.type || "View",
    __jsJuceWrapFunctionsForTransport(__jsJuceNormalizeStyleProps(node.props || {})),
    ...children
  );
}

class View {
  constructor(props = {}, children = []) {
    const hasProps = props != null && typeof props === "object" && !Array.isArray(props) && typeof props.toElement !== "function";
    const effectiveProps = hasProps ? props : {};
    const effectiveChildren = hasProps ? children : props;
    this.props = __jsJuceResolveContextStyle(this.type(), __jsJuceNormalizeStyleProps(effectiveProps));
    this.children = __jsJuceFlattenChildren(effectiveChildren);
  }
  type() { return "View"; }
  style(nextProps = {}) {
    this.props = { ...this.props, ...__jsJuceNormalizeStyleProps(nextProps) };
    return this;
  }
  css(nextStyle = {}) { return this.style({ style: nextStyle }); }
  setStyle(nextStyle = {}) { return this.css(nextStyle); }
  background(value) { return this.style({ background: value }); }
  setBackground(value) { return this.background(value); }
  gradient(from, to, vertical = true) {
    return this.style({ gradientFrom: from, gradientTo: to, gradientVertical: vertical ? 1 : 0 });
  }
  setGradient(from, to, vertical = true) { return this.gradient(from, to, vertical); }
  setPluginBackground(value) { return this.setBackground(value); }
  setPluginGradient(from, to, vertical = true) { return this.setGradient(from, to, vertical); }
  border(color, width = 1) { return this.style({ borderColor: color, borderWidth: width }); }
  setBorder(color, width = 1) { return this.border(color, width); }
  color(value) { return this.style({ color: value }); }
  setColor(value) { return this.color(value); }
  setPosition(x, y) { return this.style({ x, y }); }
  add(...children) {
    this.children.push(...__jsJuceFlattenChildren(children));
    return this;
  }
  toElement() { return __jsJuceCreateElement(this.type(), this.props, ...this.children.map(__jsJuceNormalizeNode)); }
}

class Row extends View { type() { return "Row"; } }
class Column extends View { type() { return "Column"; } }

class Text extends View {
  constructor(textOrProps, props = {}) {
    if (typeof textOrProps === "string" || typeof textOrProps === "number" || typeof textOrProps === "boolean") {
      super({ ...props, text: String(textOrProps) }, []);
      return;
    }
    const p = textOrProps || {};
    const text = p.text ?? p.children ?? "";
    super({ ...p, text: String(text) }, []);
  }
  type() { return "Text"; }
}

class Button extends View {
  constructor(textOrProps, props = {}) {
    if (typeof textOrProps === "string" || typeof textOrProps === "number" || typeof textOrProps === "boolean") {
      super({ ...props, text: String(textOrProps) }, []);
      return;
    }
    const p = textOrProps || {};
    const text = p.text ?? p.children ?? "";
    super({ ...p, text: String(text) }, []);
  }
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

function h(type, props, ...children) {
  if (typeof type === "function") {
    return type({ ...(props || {}), children });
  }
  return __jsJuceCreateElement(
    type,
    __jsJuceWrapFunctionsForTransport(__jsJuceNormalizeStyleProps(props || {})),
    ...__jsJuceFlattenChildren(children).map(__jsJuceNormalizeNode)
  );
}

const view = (props, ...children) => new View(props, children);
const row = (props, ...children) => new Row(props, children);
const column = (props, ...children) => new Column(props, children);
const text = (value, props) => new Text(value, props);
const button = (value, props) => new Button(value, props);
const slider = (props) => new Slider(props);
const textInput = (props) => new TextInput(props);

globalThis.JuceUI = {
  createElement: function(type, props, ...children) {
    return h(type, props, ...children);
  },
  h,
  render: function(tree) {
    __jsJuceRender(__jsJuceNormalizeNode(tree));
  },
  withStyleContext: function(contextObject, buildFn) {
    const parent = __jsJuceCurrentStyleContext();
    const merged = Object.create(parent || null);
    for (const k of Object.keys(contextObject || {}))
      merged[k] = { ...(parent && parent[k] ? parent[k] : {}), ...(contextObject[k] || {}) };

    __jsJuceStyleContextStack.push(merged);
    try {
      return buildFn();
    } finally {
      __jsJuceStyleContextStack.pop();
    }
  }
};

globalThis.__jsJuceInvokeControlCallback = function(callbackId, value) {
  const fn = __jsJuceControlCallbacks[callbackId];
  if (typeof fn === "function")
    fn(value);
};

globalThis.JSJuce = {
  View: (nameOrProps, x, y) => new View({ name: nameOrProps, x, y }),
  Row: (nameOrProps, x, y) => new Row({ name: nameOrProps, x, y }),
  Column: (nameOrProps, x, y) => new Column({ name: nameOrProps, x, y }),
  Text: (nameOrText, x, y) => new Text(nameOrText, { x, y }),
  Button: (nameOrText, x, y) => new Button(nameOrText, { x, y }),
  Slider: (nameOrProps, x, y) => new Slider({ name: nameOrProps, x, y }),
  TextInput: (nameOrProps, x, y) => new TextInput({ name: nameOrProps, x, y })
};

globalThis.View = View;
globalThis.Row = Row;
globalThis.Column = Column;
globalThis.Text = Text;
globalThis.Button = Button;
globalThis.Slider = Slider;
globalThis.TextInput = TextInput;
globalThis.view = view;
globalThis.row = row;
globalThis.column = column;
globalThis.text = text;
globalThis.button = button;
globalThis.slider = slider;
globalThis.textInput = textInput;
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

bool JsRuntime::invokeControlCallback(const juce::String& callbackId, const juce::var& value, juce::String& errorMessage)
{
#if JS_JUCE_HAS_QUICKJS
    if (impl->context == nullptr)
    {
        errorMessage = "QuickJS runtime is not initialised.";
        return false;
    }

    JSValue global = JS_GetGlobalObject(impl->context);
    JSValue invokeFn = JS_GetPropertyStr(impl->context, global, "__jsJuceInvokeControlCallback");
    if (! JS_IsFunction(impl->context, invokeFn))
    {
        JS_FreeValue(impl->context, invokeFn);
        JS_FreeValue(impl->context, global);
        errorMessage = "Control callback bridge is not installed.";
        return false;
    }

    const auto valueJson = juce::JSON::toString(value);
    const auto script = "JSON.parse(" + juce::JSON::toString(valueJson) + ")";
    JSValue parsedValue = JS_Eval(impl->context,
                                  script.toRawUTF8(),
                                  static_cast<size_t>(script.getNumBytesAsUTF8()),
                                  "<callback-value>",
                                  JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(parsedValue))
    {
        JS_FreeValue(impl->context, parsedValue);
        JS_FreeValue(impl->context, invokeFn);
        JS_FreeValue(impl->context, global);
        errorMessage = "Failed to serialize callback value.";
        return false;
    }

    JSValue args[2] = {
        JS_NewString(impl->context, callbackId.toRawUTF8()),
        parsedValue
    };
    JSValue result = JS_Call(impl->context, invokeFn, global, 2, args);
    JS_FreeValue(impl->context, args[0]);
    JS_FreeValue(impl->context, args[1]);

    if (JS_IsException(result))
    {
        JSValue exception = JS_GetException(impl->context);
        const char* raw = JS_ToCString(impl->context, exception);
        errorMessage = raw != nullptr ? juce::String(raw) : "QuickJS callback exception";
        if (raw != nullptr)
            JS_FreeCString(impl->context, raw);
        JS_FreeValue(impl->context, exception);
        JS_FreeValue(impl->context, result);
        JS_FreeValue(impl->context, invokeFn);
        JS_FreeValue(impl->context, global);
        return false;
    }

    JS_FreeValue(impl->context, result);
    JS_FreeValue(impl->context, invokeFn);
    JS_FreeValue(impl->context, global);
    return true;
#else
    juce::ignoreUnused(callbackId, value);
    errorMessage = "QuickJS is not available.";
    return false;
#endif
}
}
