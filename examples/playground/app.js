const Button = JSJuce.Button;
const Text = JSJuce.Text;
const Row = JSJuce.Row;
const Column = JSJuce.Column;
const TextInput = JSJuce.TextInput;
const Slider = JSJuce.Slider;

const root = Column("Root", 0, 0)
  .setBackground("#1f1f24")
  .setBorder("#404040", 1)
  .add(
    Text("playground + js_juce").setColor("#7df9ff"),
    Row("ButtonRow", 0, 0)
      .setBackground("#2a2a33")
      .setBorder("#666666", 1)
      .add(
        Button("PlayButton", 20, 20)
          .setBackground("#3e5a9b")
          .setColor("#ffffff"),
        Button("StopButton", 20, 20)
          .setBackground("#5a3e9b")
          .setColor("#ffffff")
      ),
    Slider("Gain", 0, 0),
    TextInput({ text: "Live edit this file" })
      .setColor("#ffffff")
      .setBackground("#222831")
      .setBorder("#7df9ff", 1)
  );

root.children[1].children[0].onControl = function(value) {
  console.log("Play: " + value);
};

JuceUI.render(root);
