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
  background: "#171f2d",
  borderColor: "#2a3448",
  borderWidth: 1,
  padding: 12
};

function App() {
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 10, padding: 12, background: "#0e1420", minHeight: 620 }}>
      <row style={{ ...CARD, display: "flex", alignItems: "center", justifyContent: "space-between", background: "#121a28" }}>
        <row style={{ display: "flex", alignItems: "center", gap: 10 }}>
          <text style={{ fontSize: 17, fontWeight: 700, color: "#dbe7ff" }}>Pulse Studio</text>
          <text style={{ fontSize: 12, color: "#8ea3c4" }}>Project: Midnight Drive</text>
        </row>
        <row style={{ display: "flex", gap: 8 }}>
          <button text="Save" style={{ background: "#3f5d88", color: "#f3f6fb", width: 86, height: 30 }} />
          <button text="Export" style={{ background: "#4a6d97", color: "#f3f6fb", width: 86, height: 30 }} />
          <button text="Share" style={{ background: "#5a5c87", color: "#f3f6fb", width: 86, height: 30 }} />
        </row>
      </row>

      <row style={{ display: "flex", gap: 10, flex: "1 1 auto" }}>
        <column style={{ ...CARD, minWidth: 200, width: 220, gap: 8, background: "#121a28" }}>
          <text style={{ fontSize: 12, color: "#8ea3c4" }}>NAVIGATION</text>
          <button text="Arrangement" style={{ background: "#405d87", color: "#f3f6fb", height: 32 }} />
          <button text="Mixer" style={{ background: "#2a3448", color: "#d6e2f8", height: 32 }} />
          <button text="Instruments" style={{ background: "#2a3448", color: "#d6e2f8", height: 32 }} />
          <button text="Effects" style={{ background: "#2a3448", color: "#d6e2f8", height: 32 }} />
          <button text="Automation" style={{ background: "#2a3448", color: "#d6e2f8", height: 32 }} />

          <text style={{ marginTop: 8, fontSize: 12, color: "#8ea3c4" }}>MASTER</text>
          <text style={{ color: "#d6e2f8" }}>Output: Built-in</text>
          <text style={{ color: "#d6e2f8" }}>CPU: 18%</text>
          <text style={{ color: "#d6e2f8" }}>Latency: 5.3 ms</text>
        </column>

        <column style={{ display: "flex", flex: "1 1 auto", gap: 10 }}>
          <row style={{ ...CARD, display: "flex", alignItems: "center", justifyContent: "space-between", background: "#121a28" }}>
            <row style={{ display: "flex", gap: 8 }}>
              <button text="Play" style={{ background: "#3d6a8a", color: "#f3f6fb", width: 90, height: 32 }} />
              <button text="Stop" style={{ background: "#4e5f7f", color: "#f3f6fb", width: 90, height: 32 }} />
              <button text="Record" style={{ background: "#7a4f6a", color: "#f3f6fb", width: 90, height: 32 }} />
              <button text="Loop" style={{ background: "#506e97", color: "#f3f6fb", width: 90, height: 32 }} />
            </row>
            <row style={{ display: "flex", gap: 12 }}>
              <text style={{ color: "#8ea3c4" }}>Tempo</text>
              <input text="124.0 BPM" style={{ width: 110, height: 30, background: "#223047", color: "#f3f6fb", paddingLeft: 8 }} />
              <text style={{ color: "#8ea3c4" }}>Time</text>
              <input text="01:12:480" style={{ width: 110, height: 30, background: "#223047", color: "#f3f6fb", paddingLeft: 8 }} />
            </row>
          </row>

          <row style={{ display: "flex", gap: 10, flexWrap: "wrap" }}>
            <column style={{ ...CARD, minWidth: 320, flex: "2 1 520" }}>
              <text style={{ fontSize: 15, fontWeight: 600, color: "#dbe7ff" }}>Arrangement Timeline</text>
              <row style={{ display: "flex", gap: 8, marginTop: 10 }}>
                <column style={{ background: "#2b3a53", borderWidth: 1, borderColor: "#3c4d69", padding: 8, flex: "1 1 0", minHeight: 120 }}>
                  <text style={{ color: "#d6e2f8" }}>Drums</text>
                  <text style={{ marginTop: 6, color: "#9eb2d1" }}>Pattern A - 8 bars</text>
                </column>
                <column style={{ background: "#314463", borderWidth: 1, borderColor: "#435a7d", padding: 8, flex: "1 1 0", minHeight: 120 }}>
                  <text style={{ color: "#d6e2f8" }}>Bass</text>
                  <text style={{ marginTop: 6, color: "#9eb2d1" }}>Take 03 - 8 bars</text>
                </column>
                <column style={{ background: "#3a4f72", borderWidth: 1, borderColor: "#506a93", padding: 8, flex: "1 1 0", minHeight: 120 }}>
                  <text style={{ color: "#d6e2f8" }}>Lead</text>
                  <text style={{ marginTop: 6, color: "#9eb2d1" }}>Chorus Layer</text>
                </column>
              </row>
            </column>

            <column style={{ ...CARD, minWidth: 290, flex: "1 1 290" }}>
              <text style={{ fontSize: 15, fontWeight: 600, color: "#dbe7ff" }}>Inspector</text>
              <text style={{ marginTop: 8, color: "#8ea3c4" }}>Selected Clip</text>
              <input text="Lead - Chorus Layer" style={{ marginTop: 4, background: "#223047", color: "#f3f6fb", height: 30, paddingLeft: 8 }} />
              <text style={{ marginTop: 10, color: "#8ea3c4" }}>Gain</text>
              <slider min={0} max={100} value={72} style={{ marginTop: 4, height: 26 }} />
              <text style={{ marginTop: 8, color: "#8ea3c4" }}>Pan</text>
              <slider min={-100} max={100} value={12} style={{ marginTop: 4, height: 26 }} />
              <row style={{ display: "flex", gap: 8, marginTop: 10 }}>
                <button text="Solo" style={{ background: "#3e5a83", color: "#f3f6fb", width: 86, height: 30 }} />
                <button text="Mute" style={{ background: "#344c6d", color: "#f3f6fb", width: 86, height: 30 }} />
                <button text="Freeze" style={{ background: "#4a557a", color: "#f3f6fb", width: 86, height: 30 }} />
              </row>
            </column>
          </row>
        </column>
      </row>

      <row style={{ ...CARD, display: "flex", alignItems: "center", justifyContent: "space-between", background: "#121a28" }}>
        <row style={{ display: "flex", gap: 16 }}>
          <text style={{ color: "#9eb2d1" }}>Sample Rate: 48000</text>
          <text style={{ color: "#9eb2d1" }}>Buffer: 256</text>
          <text style={{ color: "#9eb2d1" }}>Voices: 24</text>
        </row>
        <text style={{ color: "#86d39b" }}>All changes saved</text>
      </row>
    </div>
  );
}

const app = <App />;
APP_FONT.applyTo(app);
JuceUI.render(app);
