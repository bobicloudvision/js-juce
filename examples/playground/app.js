const Btn = JSJuce.Button;
const Txt = JSJuce.Text;
const HRow = JSJuce.Row;
const VBox = JSJuce.Column;
const Input = JSJuce.TextInput;
const Range = JSJuce.Slider;

const TABS = {
  LAYOUT: "layout",
  SIZING: "sizing",
  CONTROLS: "controls"
};

let activeTab = TABS.LAYOUT;

function cardBase() {
  return {
    backgroundColor: "#171f33",
    borderColor: "#3f5797",
    borderWidth: 1,
    padding: 8
  };
}

function makeTabButton(label, tabId) {
  const isActive = activeTab === tabId;
  const tabButton = Btn(label, 0, 0).css({
    backgroundColor: isActive ? "#3478f6" : "#25314f",
    color: "#ffffff",
    marginRight: 6
  });
  tabButton
    .setMinSize(110, 30)
    .setFlexGrow(0)
    .setFlexShrink(0)
    .setFlexBasis(-2);
  tabButton.style({ onControl: function() {
    activeTab = tabId;
    renderApp();
  }});
  return tabButton;
}

function buildLayoutTab() {
  const title = Txt("Layout: direction/wrap/justify/align/items").setColor("#9bc2ff");
  const flexRow = HRow("FlexRow", 0, 0)
    .css({
      ...cardBase(),
      flexDirection: "row",
      flexWrap: "wrap",
      justifyContent: "space-between",
      alignItems: "center",
      alignContent: "space-around",
      gap: 6,
      rowGap: 8,
      columnGap: 10,
      minHeight: 90
    });

  const cellA = Btn("A", 0, 0).setBackground("#2962ff").setColor("#fff")
    .setSize(90, 28)
    .setMargin(2)
    .setFlexGrow(0)
    .setFlexShrink(0)
    .setAlignSelf("flex-start");

  const cellB = Btn("B", 0, 0).setBackground("#7c4dff").setColor("#fff")
    .setSize(110, 28)
    .setMargin(2)
    .setFlex(1, 1, 120);

  const cellC = Btn("C", 0, 0).setBackground("#ff4081").setColor("#fff")
    .setSize(100, 28)
    .setMargin(2)
    .setAlignSelf("center")
    .style({ order: 2 });

  const cellD = Btn("D", 0, 0).setBackground("#00bfa5").setColor("#111")
    .setSize(90, 28)
    .setMargin(2)
    .setAlignSelf("stretch")
    .style({ order: 1 });

  flexRow.add(cellA, cellB, cellC, cellD);
  return [title, flexRow];
}

function buildSizingTab() {
  const title = Txt("Sizing: width/min/max + grow/shrink/basis").setColor("#9bc2ff");
  const sizingDemo = HRow("SizingDemo", 0, 0)
    .css({
      ...cardBase(),
      justifyContent: "flex-start",
      alignItems: "stretch",
      gap: 8
    });

  const fixedLeft = Input({ text: "Fixed width=180, margin=6" })
    .setColor("#fff")
    .setBackground("#243252")
    .setBorder("#5a78c3", 1)
    .setSize(180, 30)
    .setMargin(6)
    .setFlexGrow(0)
    .setFlexShrink(0);

  const flexibleCenter = Input({ text: "grow=2 shrink=1 basis=120 min=120 max=420" })
    .setColor("#fff")
    .setBackground("#243252")
    .setBorder("#5a78c3", 1)
    .setMinSize(120, 30)
    .setMaxSize(420, 30)
    .setFlex(2, 1, 120)
    .setMargin(6);

  const flexibleRight = Input({ text: "grow=1 shrink=2 basis=90" })
    .setColor("#fff")
    .setBackground("#243252")
    .setBorder("#5a78c3", 1)
    .setMinSize(90, 30)
    .setFlex(1, 2, 90)
    .setMargin(6);

  sizingDemo.add(fixedLeft, flexibleCenter, flexibleRight);
  return [title, sizingDemo];
}

function buildControlsTab() {
  const title = Txt("Controls with shorthand + css(...)").setColor("#9bc2ff");
  const controls = VBox("Controls", 0, 0)
    .css({
      ...cardBase(),
      flexDirection: "column",
      gap: 6
    });

  const playButton = Btn("Play", 20, 20).css({
    backgroundColor: "#2452b5",
    color: "#ffffff",
    marginBottom: 2
  });

  const stopButton = Btn("Stop", 20, 20).css({
    backgroundColor: "#6f2fa3",
    color: "#ffffff"
  });

  const recordButton = Btn("Record", 20, 20).css({
    backgroundColor: "#9a2d45",
    color: "#ffffff"
  });

  const gainSlider = Range("Gain", 0, 0).setMargin(2);
  const panSlider = Range("Pan", 0, 0).setMargin(2);
  const mixSlider = Range("Mix", 0, 0).setMargin(2);

  playButton.style({ onControl: function(value) { console.log("Play: " + value); }});
  stopButton.style({ onControl: function(value) { console.log("Stop: " + value); }});
  recordButton.style({ onControl: function(value) { console.log("Record: " + value); }});

  controls.add(playButton, stopButton, recordButton, gainSlider, panSlider, mixSlider);
  return [title, controls];
}

function buildTabContent() {
  if (activeTab === TABS.SIZING) return buildSizingTab();
  if (activeTab === TABS.CONTROLS) return buildControlsTab();
  return buildLayoutTab();
}

function renderApp() {
  const title = Txt("CSS Features Playground").setColor("#7df9ff");
  const subtitle = Txt("Tabbed examples for all style props").setColor("#c7d2ff");

  const header = VBox("Header", 0, 0)
    .setBackground("#151b2c")
    .setBorder("#334268", 1)
    .setPadding(8)
    .setGap(3)
    .add(title, subtitle);

  const tabBarTitle = Txt("Tabs").setColor("#8caeff");
  const tabBar = HRow("TabBar", 0, 0)
    .css({
      ...cardBase(),
      flexDirection: "row",
      gap: 6,
      justifyContent: "flex-start",
      alignItems: "center"
    })
    .add(
      makeTabButton("Layout", TABS.LAYOUT),
      makeTabButton("Sizing", TABS.SIZING),
      makeTabButton("Controls", TABS.CONTROLS)
    );

  const content = buildTabContent();

  const root = VBox("Root", 0, 0)
    .setGradient("#0f1422", "#231734", true)
    .setBorder("#334268", 1)
    .setPadding(10)
    .setGap(10)
    .setDirection("column")
    .setJustify("flex-start")
    .setAlignItems("stretch")
    .add(header, tabBarTitle, tabBar, ...content);

  JuceUI.render(root);
}

renderApp();
