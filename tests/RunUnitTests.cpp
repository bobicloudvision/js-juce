#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    juce::UnitTestRunner runner;
    runner.runAllTests();
    return 0;
}
