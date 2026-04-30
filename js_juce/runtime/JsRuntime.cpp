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

    const auto bootstrapFile = juce::File(juce::String(__FILE__)).getSiblingFile("JsRuntime.bootstrap.js");
    const auto bootstrap = bootstrapFile.loadFileAsString();
    if (bootstrap.isEmpty())
    {
        JS_FreeValue(impl->context, global);
        return false;
    }

    auto bootstrapResult = JS_Eval(impl->context,
                                   bootstrap.toRawUTF8(),
                                   static_cast<size_t>(bootstrap.getNumBytesAsUTF8()),
                                   bootstrapFile.getFileName().toRawUTF8(),
                                   JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(bootstrapResult))
    {
        JS_FreeValue(impl->context, bootstrapResult);
        JS_FreeValue(impl->context, global);
        return false;
    }
    JS_FreeValue(impl->context, bootstrapResult);
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
