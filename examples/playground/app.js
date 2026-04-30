const Btn = JSJuce.Button;
const Txt = JSJuce.Text;
const HRow = JSJuce.Row;
const VBox = JSJuce.Column;
const Input = JSJuce.TextInput;
const Range = JSJuce.Slider;

const title = Txt("Nova Playground").setColor("#7df9ff");
const subtitle = Txt("Live UI demo for js_juce").setColor("#c7d2ff");

const header = VBox("Header", 0, 0)
  .setBackground("#151b2c")
  .setBorder("#334268", 1)
  .setPadding(8)
  .setGap(4)
  .add(title, subtitle);

const playButton = Btn("Play", 20, 20)
  .setBackground("#2452b5")
  .setColor("#ffffff");

const stopButton = Btn("Stop", 20, 20)
  .setBackground("#6f2fa3")
  .setColor("#ffffff");

const recordButton = Btn("Record", 20, 20)
  .setBackground("#9a2d45")
  .setColor("#ffffff");

const transport = HRow("Transport", 0, 0)
  .setBackground("#1d2336")
  .setBorder("#334268", 1)
  .setPadding(8)
  .setGap(8)
  .add(playButton, stopButton, recordButton);

const gainSlider = Range("Gain", 0, 0);
const panSlider = Range("Pan", 0, 0);
const mixSlider = Range("Mix", 0, 0);

const mixerTitle = Txt("Mixer").setColor("#9bc2ff");
const mixer = VBox("Mixer", 0, 0)
  .setBackground("#1a2032")
  .setBorder("#334268", 1)
  .setPadding(8)
  .setGap(6)
  .add(mixerTitle, gainSlider, panSlider, mixSlider);

const patchInput = Input({ text: "Patch: Warm Pad / BPM: 124 / Key: A minor" })
  .setColor("#ffffff")
  .setBackground("#1f273d")
  .setBorder("#4f6ab3", 1);

const notesInput = Input({ text: "Notes: Automate filter in the chorus section." })
  .setColor("#ffffff")
  .setBackground("#1f273d")
  .setBorder("#4f6ab3", 1);

const statusText = Txt("Status: ready").setColor("#9ce6b7");

const footer = VBox("Footer", 0, 0)
  .setBackground("#151b2c")
  .setBorder("#334268", 1)
  .setPadding(8)
  .setGap(6)
  .add(statusText, patchInput, notesInput);

const root = VBox("Root", 0, 0)
  .setGradient("#0f1422", "#231734", true)
  .setBorder("#334268", 1)
  .setPadding(10)
  .setGap(10)
  .add(header, transport, mixer, footer);

playButton.onControl = function(value) {
  console.log("Play: " + value);
};

stopButton.onControl = function(value) {
  console.log("Stop: " + value);
};

recordButton.onControl = function(value) {
  console.log("Record: " + value);
};

JuceUI.render(root);
