const tree = JuceUI.withStyleContext(
  {
    Column: { background: "#1f1f24", borderColor: "#404040", borderWidth: 1 },
    Row: { background: "#2a2a33", borderColor: "#666666", borderWidth: 1 },
    Text: { color: "#7df9ff" },
    TextInput: { color: "#ffffff", background: "#222831", borderColor: "#7df9ff" }
  },
  () =>
    new Column()
      .add(
        new Text("playground + js_juce"),
        new Row().add(
          new Button("Left").background("#3e5a9b").color("#ffffff"),
          new Button("Right").background("#5a3e9b").color("#ffffff")
        ),
        new Slider(),
        new TextInput({ text: "Live edit this file" })
      )
);

JuceUI.render(tree);
