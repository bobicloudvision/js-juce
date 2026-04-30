#include "JsBridge.h"
#include <cstdlib>
#include <vector>

namespace js_juce
{
namespace
{
class JsBridgeUnitTests final : public juce::UnitTest
{
public:
    JsBridgeUnitTests() : juce::UnitTest("JsBridge", "bridge") {}

    void runTest() override
    {
        beginTest("Attach fails with missing JSX source");
        {
            juce::Component root;
            root.setSize(800, 600);

            JsBridge bridge;
            const auto missingJsx = makeTempDir().getChildFile("missing-entry.jsx");
            const auto ok = bridge.attach(root, missingJsx, false);
            expect(! ok);
            expect(bridge.getLastError().contains("JSX source file does not exist"));
        }

        beginTest("Attach renders a JS entry script");
        {
            juce::Component root;
            root.setSize(800, 600);

            auto dir = makeTempDir();
            auto script = dir.getChildFile("entry.js");
            script.replaceWithText(
                "JuceUI.render(\n"
                "  JuceUI.createElement('View', { display: 'flex', direction: 'row', gap: 8 },\n"
                "    JuceUI.createElement('Text', { text: 'Bridge Test', color: '#ffffff' }),\n"
                "    JuceUI.createElement('Button', { text: 'Run', width: 80, height: 28 })\n"
                "  )\n"
                ");\n");

            JsBridge bridge;
            juce::String err;
            const auto attached = bridge.attach(root, script, false);
            expect(attached, "attach should succeed for valid JS script");
            expect(err.isEmpty());
            expectGreaterThan(root.getNumChildComponents(), 0);
        }

        beginTest("Attach transpiles JSX via JS_JUCE_ESBUILD bridge path");
        {
            juce::Component root;
            root.setSize(800, 600);

            auto dir = makeTempDir();
            auto jsx = dir.getChildFile("entry.jsx");
            jsx.replaceWithText(
                "function App() {\n"
                "  return <text style={{ color: '#ffffff' }}>JSX Bridge</text>;\n"
                "}\n"
                "JuceUI.render(<App />);\n");

            auto fakeEsbuild = dir.getChildFile("esbuild-mock.sh");
            fakeEsbuild.replaceWithText(
                "#!/bin/sh\n"
                "out=\"\"\n"
                "for arg in \"$@\"; do\n"
                "  case \"$arg\" in\n"
                "    --outfile=*) out=\"${arg#--outfile=}\" ;;\n"
                "  esac\n"
                "done\n"
                "if [ -z \"$out\" ]; then\n"
                "  echo \"missing outfile\" >&2\n"
                "  exit 1\n"
                "fi\n"
                "cat > \"$out\" <<'EOF'\n"
                "(function(){ JuceUI.render(JuceUI.createElement('Text', { text: 'compiled jsx ok' })); })();\n"
                "EOF\n"
                "exit 0\n");
            fakeEsbuild.setExecutePermission(true);

            auto oldEnv = juce::SystemStats::getEnvironmentVariable("JS_JUCE_ESBUILD", "");
            ::setenv("JS_JUCE_ESBUILD", fakeEsbuild.getFullPathName().toRawUTF8(), 1);

            JsBridge bridge;
            const auto attached = bridge.attach(root, jsx, false);
            expect(attached, "attach should succeed for JSX when bridge transpile is available");
            expectGreaterThan(root.getNumChildComponents(), 0);

            if (oldEnv.isEmpty())
                ::unsetenv("JS_JUCE_ESBUILD");
            else
                ::setenv("JS_JUCE_ESBUILD", oldEnv.toRawUTF8(), 1);
        }

        beginTest("JSX syntax and style matrix through bridge transpile");
        {
            const auto esbuildPath = resolveEsbuildForTests();
            if (! esbuildPath.existsAsFile())
            {
                logMessage("Skipping JSX matrix test: esbuild binary not found.");
            }
            else
            {
                const auto cases = buildCombinatorialJsxCases();
                for (const auto& testCase : cases)
                    expectJsxCaseRenders(esbuildPath, testCase.first, testCase.second);

                const auto pairwiseCases = buildPairwiseStyleInteractionCases();
                for (const auto& testCase : pairwiseCases)
                    expectJsxCaseRenders(esbuildPath, testCase.first, testCase.second);
            }
        }

        beginTest("Header row remains visible in column layout");
        {
            const auto esbuildPath = resolveEsbuildForTests();
            if (! esbuildPath.existsAsFile())
            {
                logMessage("Skipping visibility regression test: esbuild binary not found.");
            }
            else
            {
                juce::Component root;
                root.setSize(900, 700);

                auto dir = makeTempDir();
                auto jsxFile = dir.getChildFile("entry.jsx");
                jsxFile.replaceWithText(
                    "const CARD = { borderColor: '#2a3448', borderWidth: 1, padding: 12 };\n"
                    "function App() {\n"
                    "  return (\n"
                    "    <div style={{ display: 'flex', flexDirection: 'column', gap: 10, minHeight: 620, padding: 12 }}>\n"
                    "      <row style={{ ...CARD, display: 'flex', alignItems: 'center', justifyContent: 'space-between', background: '#121a28' }}>\n"
                    "        <row style={{ display: 'flex', alignItems: 'center', gap: 10 }}>\n"
                    "          <text style={{ fontSize: 17, fontWeight: 700 }}>Pulse Studio</text>\n"
                    "          <text style={{ fontSize: 12 }}>Project: Midnight Drive</text>\n"
                    "        </row>\n"
                    "        <row style={{ display: 'flex', gap: 8 }}>\n"
                    "          <button text='Save' style={{ width: 86, height: 30 }} />\n"
                    "          <button text='Export' style={{ width: 86, height: 30 }} />\n"
                    "          <button text='Share' style={{ width: 86, height: 30 }} />\n"
                    "        </row>\n"
                    "      </row>\n"
                    "      <row style={{ display: 'flex', gap: 10, grow: 1, shrink: 1, basis: -1 }}>\n"
                    "        <column style={{ ...CARD, minWidth: 200, width: 220 }}>\n"
                    "          <text>NAVIGATION</text>\n"
                    "        </column>\n"
                    "      </row>\n"
                    "      <row style={{ ...CARD, display: 'flex', justifyContent: 'space-between', background: '#121a28' }}>\n"
                    "        <text>Sample Rate: 48000</text>\n"
                    "        <text>All changes saved</text>\n"
                    "      </row>\n"
                    "    </div>\n"
                    "  );\n"
                    "}\n"
                    "JuceUI.render(<App />);\n");

                const auto oldEnv = juce::SystemStats::getEnvironmentVariable("JS_JUCE_ESBUILD", "");
                ::setenv("JS_JUCE_ESBUILD", esbuildPath.getFullPathName().toRawUTF8(), 1);

                JsBridge bridge;
                const auto attached = bridge.attach(root, jsxFile, false);
                expect(attached, "attach should succeed for header visibility test");

                auto* appRoot = root.getNumChildComponents() > 0 ? root.getChildComponent(0) : nullptr;
                expect(appRoot != nullptr, "expected app root component");
                if (appRoot != nullptr)
                {
                    expectGreaterThan(appRoot->getNumChildComponents(), 2);
                    auto* header = appRoot->getChildComponent(0);
                    auto* body = appRoot->getChildComponent(1);
                    auto* footer = appRoot->getChildComponent(2);

                    expect(header != nullptr && body != nullptr && footer != nullptr, "expected header/body/footer rows");
                    if (header != nullptr && body != nullptr)
                    {
                        expectGreaterThan(header->getHeight(), 0);
                        expectGreaterThan(header->getWidth(), 0);
                        expect(body->getHeight() > header->getHeight(),
                               "body row should be taller than header when only body has grow=1");

                        expectGreaterThan(header->getNumChildComponents(), 1);
                        if (header->getNumChildComponents() > 1)
                        {
                            auto* titleGroup = header->getChildComponent(0);
                            auto* actionGroup = header->getChildComponent(1);
                            expect(titleGroup != nullptr && actionGroup != nullptr,
                                   "expected title and action groups in header");
                            if (titleGroup != nullptr && actionGroup != nullptr)
                            {
                                expectGreaterThan(titleGroup->getWidth(), 0);
                                expectGreaterThan(actionGroup->getWidth(), 0);
                                expectGreaterThan(titleGroup->getHeight(), 0);
                                expectGreaterThan(actionGroup->getHeight(), 0);
                            }
                        }
                    }
                }

                if (oldEnv.isEmpty())
                    ::unsetenv("JS_JUCE_ESBUILD");
                else
                    ::setenv("JS_JUCE_ESBUILD", oldEnv.toRawUTF8(), 1);
            }
        }
    }

private:
    static juce::File makeTempDir()
    {
        const auto base = juce::File::getSpecialLocation(juce::File::tempDirectory)
                              .getChildFile("js_juce_bridge_tests");
        base.createDirectory();
        const auto dir = base.getNonexistentChildFile("case_", "", false);
        dir.createDirectory();
        return dir;
    }

    static juce::File resolveEsbuildForTests()
    {
        const auto fromEnv = juce::SystemStats::getEnvironmentVariable("JS_JUCE_ESBUILD", "");
        if (fromEnv.isNotEmpty())
        {
            const juce::File candidate(fromEnv);
            if (candidate.existsAsFile())
                return candidate;
        }

        auto root = juce::File::getCurrentWorkingDirectory();
        for (int i = 0; i < 12; ++i)
        {
            const auto candidate = root
                                       .getChildFile("examples")
                                       .getChildFile("playground")
                                       .getChildFile("jsx-tooling")
                                       .getChildFile("node_modules")
                                       .getChildFile(".bin")
                                       .getChildFile("esbuild");
            if (candidate.existsAsFile())
                return candidate;

            const auto parent = root.getParentDirectory();
            if (parent == root)
                break;
            root = parent;
        }

        return {};
    }

    static std::vector<std::pair<juce::String, juce::String>> buildCombinatorialJsxCases()
    {
        const std::vector<juce::String> syntaxVariants {
            "direct",
            "fragment"
        };
        const std::vector<juce::String> rootTags {
            "div",
            "row",
            "column"
        };
        const std::vector<juce::String> rootStyles {
            "display: 'flex', gap: 8, padding: 10",
            "display: 'flex', justifyContent: 'space-between', alignItems: 'center', minHeight: 80",
            "display: 'flex', wrap: 'wrap', rowGap: 6, columnGap: 10, minWidth: 240",
            "display: 'flex', position: 'relative', borderWidth: 1, borderColor: '#2a3a50', background: '#172236'"
        };
        const std::vector<juce::String> childVariants {
            "<text style={{ color: '#dbe7ff', fontSize: 14, fontWeight: '600' }}>A</text>",
            "<button style={{ width: 90, height: 28, background: '#3d628a', color: '#ffffff' }}>Run</button>",
            "<input text='Lead' style={{ width: 120, height: 28, background: '#223047', color: '#ffffff' }} />",
            "<slider min={0} max={100} value={40} style={{ width: 120, height: 24 }} />",
            "<text style={{ position: 'absolute', top: 6, left: 8, zIndex: 5 }}>Pinned</text>"
        };

        std::vector<std::pair<juce::String, juce::String>> cases;
        for (size_t syntaxIndex = 0; syntaxIndex < syntaxVariants.size(); ++syntaxIndex)
        {
            for (size_t rootIndex = 0; rootIndex < rootTags.size(); ++rootIndex)
            {
                for (size_t styleIndex = 0; styleIndex < rootStyles.size(); ++styleIndex)
                {
                    const auto childA = childVariants[(rootIndex + styleIndex) % childVariants.size()];
                    const auto childB = childVariants[(rootIndex + styleIndex + 2) % childVariants.size()];
                    const auto childC = childVariants[(rootIndex + styleIndex + 4) % childVariants.size()];

                    const auto name =
                        "syntax=" + syntaxVariants[syntaxIndex]
                        + " root=" + rootTags[rootIndex]
                        + " style#" + juce::String(static_cast<int>(styleIndex));

                    const auto jsx =
                        makeJsxCaseSource(syntaxVariants[syntaxIndex],
                                          rootTags[rootIndex],
                                          rootStyles[styleIndex],
                                          childA + "\n      " + childB + "\n      " + childC);
                    cases.push_back({ name, jsx });
                }
            }
        }

        return cases;
    }

    static std::vector<std::pair<juce::String, juce::String>> buildPairwiseStyleInteractionCases()
    {
        struct StyleDimension
        {
            juce::String name;
            juce::String valueA;
            juce::String valueB;
        };

        const std::vector<StyleDimension> dimensions {
            { "justify", "justifyContent: 'center'", "justifyContent: 'space-between'" },
            { "align", "alignItems: 'flex-start'", "alignItems: 'center'" },
            { "wrap", "wrap: 'nowrap'", "wrap: 'wrap'" },
            { "grow", "grow: 1, shrink: 1, basis: 140", "grow: 2, shrink: 1, basis: 220" },
            { "position", "position: 'relative'", "position: 'absolute', top: 4, left: 6" },
            { "zIndex", "zIndex: 1", "zIndex: 7" },
            { "border", "borderWidth: 1, borderColor: '#2d3d53'", "borderWidth: 2, borderColor: '#4b6c96'" },
            { "boxSizing", "boxSizing: 'content-box'", "boxSizing: 'border-box'" },
            { "text", "fontSize: 13, fontWeight: '400', letterSpacing: 0.1", "fontSize: 17, fontWeight: '700', letterSpacing: 0.6" }
        };

        std::vector<std::pair<juce::String, juce::String>> cases;
        for (size_t i = 0; i < dimensions.size(); ++i)
        {
            for (size_t j = i + 1; j < dimensions.size(); ++j)
            {
                for (int mask = 0; mask < 4; ++mask)
                {
                    const auto firstValue = ((mask & 1) == 0) ? dimensions[i].valueA : dimensions[i].valueB;
                    const auto secondValue = ((mask & 2) == 0) ? dimensions[j].valueA : dimensions[j].valueB;

                    const auto rootStyle =
                        "display: 'flex', gap: 6, padding: 8, minHeight: 88, "
                        + firstValue + ", "
                        + secondValue;

                    const auto children =
                        "<text style={{ color: '#dbe7ff' }}>pairwise</text>\n"
                        "      <button style={{ width: 84, height: 26, background: '#3f628a', color: '#ffffff' }}>ok</button>\n"
                        "      <input text='x' style={{ width: 90, height: 24, background: '#1f2d42', color: '#ffffff' }} />";

                    const auto name =
                        "pairwise " + dimensions[i].name
                        + "-" + dimensions[j].name
                        + " variant=" + juce::String(mask);

                    const auto jsx = makeJsxCaseSource("direct", "row", rootStyle, children);
                    cases.push_back({ name, jsx });
                }
            }
        }

        return cases;
    }

    static juce::String makeJsxCaseSource(const juce::String& syntax,
                                          const juce::String& rootTag,
                                          const juce::String& rootStyle,
                                          const juce::String& children)
    {
        const auto rootBlock =
            "    <" + rootTag + " style={{ " + rootStyle + " }}>\n"
            "      " + children + "\n"
            "    </" + rootTag + ">";

        const auto returned =
            syntax == "fragment"
                ? ("    <>\n" + rootBlock + "\n    </>")
                : rootBlock;

        return
            "function App() {\n"
            "  return (\n"
            + returned + "\n"
            "  );\n"
            "}\n"
            "JuceUI.render(<App />);\n";
    }

    void expectJsxCaseRenders(const juce::File& esbuildPath, const juce::String& name, const juce::String& jsxSource)
    {
        juce::Component root;
        root.setSize(900, 700);

        auto dir = makeTempDir();
        auto jsxFile = dir.getChildFile("entry.jsx");
        jsxFile.replaceWithText(jsxSource);

        const auto oldEnv = juce::SystemStats::getEnvironmentVariable("JS_JUCE_ESBUILD", "");
        ::setenv("JS_JUCE_ESBUILD", esbuildPath.getFullPathName().toRawUTF8(), 1);

        JsBridge bridge;
        const auto attached = bridge.attach(root, jsxFile, false);
        expect(attached, "JSX case failed: " + name + " | error: " + bridge.getLastError());
        expectGreaterThan(root.getNumChildComponents(), 0);

        if (oldEnv.isEmpty())
            ::unsetenv("JS_JUCE_ESBUILD");
        else
            ::setenv("JS_JUCE_ESBUILD", oldEnv.toRawUTF8(), 1);
    }
};

static JsBridgeUnitTests jsBridgeUnitTests;
} // namespace
} // namespace js_juce
