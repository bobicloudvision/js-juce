const APP_FONT = JSJuce.Font({
  fontFile: "/Users/bozhidar/PhpstormProjects/js-juce/examples/playground/assets/fonts/Outfit[wght].ttf",
  fontFamily: "Outfit",
  fontSize: 14,
  fontWeight: "400"
});

JuceUI.setGlobalStyle({
  boxSizing: "border-box",
  color: "#f3f6fb"
});

const CARD = {
  background: "#161f2f",
  borderColor: "#2b3951",
  borderWidth: 1,
  padding: 10
};

function App() {
  return (
    <column style={{ display: "flex", gap: 10, padding: 12, background: "#0f1625", minHeight: 900 }}>
      <column style={{ ...CARD, background: "#1a2740", gap: 4 }}>
        <text text="JS-JUCE Debug Matrix" style={{ fontSize: 20, fontWeight: 700, color: "#ffffff" }} />
        <text text="Report only the broken case number(s): #1, #7, #12..." style={{ fontSize: 12, color: "#b7c8e8" }} />
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#1 Plain text element" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <text text="Case #1 text" style={{ color: "#ffffff" }} />
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#2 Text styles (size + weight + color)" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <text text="Case #2 title" style={{ fontSize: 18, fontWeight: 700, color: "#ffffff" }} />
        <text text="Case #2 subtitle" style={{ fontSize: 12, color: "#c6d6f2" }} />
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#3 Single button intrinsic size" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <button text="Case #3 button" style={{ background: "#44608a", color: "#ffffff" }} />
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#4 Row: text + buttons + alignItems:center" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <row style={{ display: "flex", alignItems: "center", gap: 8, background: "#1a2a42", padding: 8 }}>
          <text text="Case #4 left text" style={{ color: "#ffffff", fontWeight: 600 }} />
          <button text="A" style={{ width: 40, height: 28, background: "#4c688f", color: "#fff" }} />
          <button text="B" style={{ width: 40, height: 28, background: "#4c688f", color: "#fff" }} />
        </row>
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#5 Header-like row (same pattern you reported)" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <row style={{ display: "flex", alignItems: "center", justifyContent: "space-between", background: "#8a2b2b", padding: 10, borderWidth: 1, borderColor: "#ff8080" }}>
          <row style={{ display: "flex", alignItems: "center", gap: 10, minWidth: 220 }}>
            <text text="Case #5 Pulse Studio" style={{ fontSize: 17, fontWeight: 700, color: "#ffffff" }} />
            <text text="Project: Midnight Drive" style={{ fontSize: 12, color: "#ffffff" }} />
          </row>
          <row style={{ display: "flex", gap: 8 }}>
            <button text="Save" style={{ background: "#3f5d88", color: "#f3f6fb", width: 86, height: 30 }} />
            <button text="Export" style={{ background: "#4a6d97", color: "#f3f6fb", width: 86, height: 30 }} />
            <button text="Share" style={{ background: "#5a5c87", color: "#f3f6fb", width: 86, height: 30 }} />
          </row>
        </row>
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#6 Input + text combo" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <row style={{ display: "flex", alignItems: "center", gap: 8 }}>
          <text text="Track" style={{ color: "#dbe7ff" }} />
          <input text="Case #6 input value" style={{ background: "#213049", color: "#fff", height: 30, width: 200 }} />
        </row>
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#7 Slider + labels" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <row style={{ display: "flex", alignItems: "center", gap: 8 }}>
          <text text="Gain" style={{ color: "#dbe7ff" }} />
          <slider min={0} max={100} value={45} style={{ width: 180, height: 26 }} />
          <text text="45%" style={{ color: "#ffffff" }} />
        </row>
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#8 Wrap behavior with many buttons" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <row style={{ display: "flex", gap: 6, wrap: "wrap", background: "#1a2a42", padding: 8 }}>
          <button text="8.1" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
          <button text="8.2" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
          <button text="8.3" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
          <button text="8.4" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
          <button text="8.5" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
          <button text="8.6" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
          <button text="8.7" style={{ width: 54, height: 28, background: "#4a678f", color: "#fff" }} />
        </row>
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#9 Absolute positioned text" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <div style={{ display: "flex", position: "relative", minHeight: 80, background: "#1f2d45", padding: 8 }}>
          <text text="Case #9 base text" style={{ color: "#dbe7ff" }} />
          <text text="Case #9 ABS" style={{ position: "absolute", top: 8, right: 8, zIndex: 5, color: "#ffd98a" }} />
        </div>
      </column>

      <column style={{ ...CARD, gap: 6 }}>
        <text text="#10 Column with grow/sizing split" style={{ fontSize: 13, color: "#9fb5d9" }} />
        <row style={{ display: "flex", gap: 8, minHeight: 90 }}>
          <column style={{ background: "#334968", grow: 2, shrink: 1, basis: 180, padding: 8 }}>
            <text text="Case #10 left grow=2" style={{ color: "#fff" }} />
          </column>
          <column style={{ background: "#41597b", grow: 1, shrink: 1, basis: 120, padding: 8 }}>
            <text text="Case #10 right grow=1" style={{ color: "#fff" }} />
          </column>
        </row>
      </column>

      <column style={{ ...CARD, background: "#1a2740" }}>
        <text text="If something is missing, send: BUG #<number> + short note." style={{ color: "#ffffff", fontWeight: 600 }} />
      </column>
    </column>
  );
}

const app = <App />;
APP_FONT.applyTo(app);
JuceUI.render(app);
