const Btn = JSJuce.Button;
const Txt = JSJuce.Text;
const HRow = JSJuce.Row;
const VBox = JSJuce.Column;
const Input = JSJuce.TextInput;
const Range = JSJuce.Slider;

const title = Txt("playground + js_juce").setColor("#7df9ff");

const playButton = Btn("Play", 20, 20)
  .setBackground("#3e5a9b")
  .setColor("#ffffff");

const stopButton = Btn("Stop", 20, 20)
  .setBackground("#5a3e9b")
  .setColor("#ffffff");

const controlsRow = HRow("ControlsRow", 0, 0)
  .setBackground("#2a2a33")
  .setBorder("#666666", 1)
  .add(playButton, stopButton);

const gainSlider = Range("Gain", 0, 0);

const messageInput = Input({ text: "Live edit this file" })
  .setColor("#ffffff")
  .setBackground("#222831")
  .setBorder("#7df9ff", 1);

const root = VBox("Root", 0, 0)
  .setGradient("#1a1f2e", "#2b1f3a", true)
  .setBorder("#404040", 1)
  .add(title, controlsRow, gainSlider, messageInput);

playButton.onControl = function(value) {
  console.log("Play: " + value);
};

JuceUI.render(root);
