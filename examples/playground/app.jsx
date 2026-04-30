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

function App() {
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 10, padding: 12, background: "#101521" }}>
      <div style={{ display: "flex", flexDirection: "column", gap: 4, padding: 8, background: "#1a2333" }}>
        <text style={{ fontSize: 24, fontWeight: 700, color: "#dbe7ff" }}>JuceUI JSX Playground</text>
      </div>

      <div style={{ display: "flex", flexDirection: "row", gap: 8, padding: 10, background: "#202632" }}>
        <button text="Play2" style={{ background: "#4d6386", color: "#f3f6fb", width: 120, height: 34 }} />
        <button text="Stop" style={{ background: "#586d8f", color: "#f3f6fb", width: 120, height: 34 }} />
        <button text="Record" style={{ background: "#4a607f", color: "#f3f6fb", width: 120, height: 34 }} />
      </div>
    </div>
  );
}

const app = <App />;
APP_FONT.applyTo(app);
JuceUI.render(app);
