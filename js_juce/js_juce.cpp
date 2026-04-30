#include "js_juce.h"

#include "runtime/JsRuntime.cpp"
#include "ui/JuceRenderer.cpp"
#include "bridge/JsBridge.cpp"
#include "dev/LiveReloader.cpp"

#if JUCE_UNIT_TESTS
 #include "bridge/JsBridgeTests.cpp"
#endif
