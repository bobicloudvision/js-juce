JuceUI.render(
  new Column({}, [
    new Text("js_juce basic app"),
    new Row({}, [
      new Button("Button A"),
      new Button("Button B")
    ]),
    new Slider({}),
    new TextInput({ text: "Type here..." })
  ])
);
