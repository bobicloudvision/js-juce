JuceUI.render(
  new Column({}, [
    new Text("playground + js_juce"),
    new Row({}, [
      new Button("Left"),
      new Button("Right")
    ]),
    new Slider({}),
    new TextInput({ text: "Live edit this file" })
  ])
);
