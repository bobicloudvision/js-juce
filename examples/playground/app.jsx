const APP_FONT = JSJuce.Font({
  fontFile: "examples/playground/assets/fonts/Outfit[wght].ttf",
  fontFamily: "Outfit",
  fontSize: 14,
  fontWeight: "400"
});

JuceUI.setGlobalStyle({
  boxSizing: "border-box",
  color: "#f3f6fb"
});

const CARD = {
  background: "#1a2333",
  borderColor: "#2a364d",
  borderWidth: 1,
  padding: 10
};

function App() {
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 10, padding: 12, background: "#101521", minHeight: 540 }}>
      <div style={{ display: "flex", flexDirection: "column", gap: 4, padding: 8, background: "#1a2333", borderColor: "#2a364d", borderWidth: 1 }}>
        <text style={{ fontSize: 24, fontWeight: 700, color: "#dbe7ff" }}>JuceUI JSX Playground</text>
        <text style={{ fontSize: 13, color: "#9fb0cf" }}>Advanced JSX examples: layout, controls, and composition.</text>
      </div>

      <row style={{ display: "flex", gap: 10, flexWrap: "wrap" }}>
        <column style={{ ...CARD, minWidth: 320, flex: "1 1 320" }}>
          <text style={{ fontSize: 16, fontWeight: 600, color: "#d6e5ff" }}>Transport</text>
          <row style={{ display: "flex", gap: 8, marginTop: 8, flexWrap: "wrap" }}>
            <button text="Play" style={{ background: "#4d6386", color: "#f3f6fb", width: 110, height: 32 }} />
            <button text="Stop" style={{ background: "#586d8f", color: "#f3f6fb", width: 110, height: 32 }} />
            <button text="Record" style={{ background: "#4a607f", color: "#f3f6fb", width: 110, height: 32 }} />
            <button text="Loop" style={{ background: "#5a779f", color: "#f3f6fb", width: 110, height: 32 }} />
          </row>
        </column>

        <column style={{ ...CARD, minWidth: 320, flex: "1 1 320" }}>
          <text style={{ fontSize: 16, fontWeight: 600, color: "#d6e5ff" }}>Mixer Channel</text>
          <text style={{ marginTop: 8, color: "#aeb8c8" }}>Track name</text>
          <input
            text="Lead Synth"
            style={{ marginTop: 4, background: "#243043", color: "#f3f6fb", height: 30, paddingLeft: 8, paddingRight: 8 }}
          />
          <text style={{ marginTop: 10, color: "#aeb8c8" }}>Gain</text>
          <slider min={0} max={100} value={68} style={{ marginTop: 4, height: 28 }} />
          <row style={{ display: "flex", gap: 8, marginTop: 10 }}>
            <button text="Solo" style={{ background: "#415777", color: "#f3f6fb", width: 90, height: 30 }} />
            <button text="Mute" style={{ background: "#364860", color: "#f3f6fb", width: 90, height: 30 }} />
            <button text="Arm" style={{ background: "#6b4f6b", color: "#f3f6fb", width: 90, height: 30 }} />
          </row>
        </column>
      </row>

      <row style={{ display: "flex", gap: 10, flexWrap: "wrap" }}>
        <column style={{ ...CARD, minWidth: 320, flex: "1 1 320" }}>
          <text style={{ fontSize: 16, fontWeight: 600, color: "#d6e5ff" }}>Inspector</text>
          <row style={{ display: "flex", marginTop: 8, gap: 8 }}>
            <column style={{ display: "flex", gap: 6, minWidth: 140 }}>
              <text style={{ color: "#8fa1c1" }}>Mode</text>
              <text style={{ color: "#8fa1c1" }}>Sample rate</text>
              <text style={{ color: "#8fa1c1" }}>Latency</text>
            </column>
            <column style={{ display: "flex", gap: 6 }}>
              <text>Realtime</text>
              <text>48000 Hz</text>
              <text>128 samples</text>
            </column>
          </row>
        </column>

        <column style={{ ...CARD, minWidth: 320, flex: "1 1 320" }}>
          <text style={{ fontSize: 16, fontWeight: 600, color: "#d6e5ff" }}>Automation</text>
          <text style={{ marginTop: 8, color: "#aeb8c8" }}>Filter cutoff</text>
          <slider min={20} max={20000} value={7400} style={{ marginTop: 4, height: 28 }} />
          <text style={{ marginTop: 8, color: "#aeb8c8" }}>Resonance</text>
          <slider min={0} max={100} value={42} style={{ marginTop: 4, height: 28 }} />
          <button text="Write Automation" style={{ marginTop: 12, background: "#3d628a", color: "#f3f6fb", height: 32 }} />
        </column>
      </row>

      <div style={{ ...CARD, background: "#172033" }}>
        <text style={{ color: "#9fb0cf" }}>
          Tip: this screen intentionally uses intrinsic JSX tags (`row`, `column`, `text`, `input`, `slider`, `button`) with flex and mixed sizing.
        </text>
      </div>
    </div>
  );
}

const app = <App />;
APP_FONT.applyTo(app);
JuceUI.render(app);
