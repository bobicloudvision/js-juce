const Btn = JSJuce.Button;
const Txt = JSJuce.Text;
const HRow = JSJuce.Row;
const VBox = JSJuce.Column;
const Input = JSJuce.TextInput;
const Range = JSJuce.Slider;

const THEME = {
  colors: {
    bg: {
      appFrom: "#0f1422",
      appTo: "#231734",
      header: "#151b2c",
      surface: "#171f33",
      input: "#243252"
    },
    text: {
      primary: "#ffffff",
      muted: "#c7d2ff",
      title: "#7df9ff",
      section: "#9bc2ff",
      accent: "#8caeff",
      dark: "#111111"
    },
    tab: {
      active: "#3478f6",
      inactive: "#25314f"
    },
    accent: {
      a: "#2962ff",
      b: "#7c4dff",
      c: "#ff4081",
      d: "#00bfa5",
      play: "#2452b5",
      stop: "#6f2fa3",
      record: "#9a2d45"
    }
  },
  space: {
    xxs: 2,
    xs: 3,
    sm: 6,
    md: 8,
    lg: 10
  },
  size: {
    tabMinW: 110,
    tabMinH: 30
  },
  styles: {
    card: function() {
      return {
        backgroundColor: THEME.colors.bg.surface,
        padding: THEME.space.md
      };
    }
  }
};

const TABS = {
  LAYOUT: "layout",
  SIZING: "sizing",
  CONTROLS: "controls"
};

let activeTab = TABS.LAYOUT;

function cardBase() {
  return THEME.styles.card();
}

function makeTabButton(label, tabId) {
  const isActive = activeTab === tabId;
  const tabButton = Btn(label, 0, 0).css({
    backgroundColor: isActive ? THEME.colors.tab.active : THEME.colors.tab.inactive,
    color: THEME.colors.text.primary,
    marginRight: THEME.space.sm
  });
  tabButton
    .setMinSize(THEME.size.tabMinW, THEME.size.tabMinH);
  tabButton.style({ onControl: function() {
    activeTab = tabId;
    renderApp();
  }});
  return tabButton;
}

function buildLayoutTab() {
  const title = Txt("Layout: direction/wrap/justify/align/items").setColor(THEME.colors.text.section);
  const flexRow = HRow("FlexRow", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.sm,
      rowGap: THEME.space.md,
      columnGap: THEME.space.lg,
      minHeight: 90
    });

  const cellA = Btn({ text: "A" }).setBackground(THEME.colors.accent.a).setColor(THEME.colors.text.primary)
    .setSize(90, 28)
    .setMargin(THEME.space.xxs);

  const cellB = Btn({ text: "B" }).setBackground(THEME.colors.accent.b).setColor(THEME.colors.text.primary)
    .setSize(110, 28)
    .setMargin(THEME.space.xxs);

  const cellC = Btn({ text: "C" }).setBackground(THEME.colors.accent.c).setColor(THEME.colors.text.primary)
    .setSize(100, 28)
    .setMargin(THEME.space.xxs);

  const cellD = Btn({ text: "D" }).setBackground(THEME.colors.accent.d).setColor(THEME.colors.text.dark)
    .setSize(90, 28)
    .setMargin(THEME.space.xxs);

  flexRow.add(cellA, cellB, cellC, cellD);
  return [title, flexRow];
}

function buildSizingTab() {
  const title = Txt("Sizing: width/min/max + grow/shrink/basis").setColor(THEME.colors.text.section);
  const sizingDemo = HRow("SizingDemo", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.md
    });

  const fixedLeft = Input({ text: "Fixed width=180, margin=6" })
    .setColor(THEME.colors.text.primary)
    .setBackground(THEME.colors.bg.input)
    .setSize(180, 30)
    .setMargin(THEME.space.sm);

  const flexibleCenter = Input({ text: "grow=2 shrink=1 basis=120 min=120 max=420" })
    .setColor(THEME.colors.text.primary)
    .setBackground(THEME.colors.bg.input)
    .setMinSize(120, 30)
    .setMaxSize(420, 30)
    .setMargin(THEME.space.sm);

  const flexibleRight = Input({ text: "grow=1 shrink=2 basis=90" })
    .setColor(THEME.colors.text.primary)
    .setBackground(THEME.colors.bg.input)
    .setMinSize(90, 30)
    .setMargin(THEME.space.sm);

  sizingDemo.add(fixedLeft, flexibleCenter, flexibleRight);
  return [title, sizingDemo];
}

function buildControlsTab() {
  const title = Txt("Controls with shorthand + css(...)").setColor(THEME.colors.text.section);
  const controls = VBox("Controls", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.sm
    });

  const playButton = Btn("Play", 20, 20).css({
    backgroundColor: THEME.colors.accent.play,
    color: THEME.colors.text.primary,
    marginBottom: THEME.space.xxs
  });

  const stopButton = Btn("Stop", 20, 20).css({
    backgroundColor: THEME.colors.accent.stop,
    color: THEME.colors.text.primary
  });

  const recordButton = Btn("Record", 20, 20).css({
    backgroundColor: THEME.colors.accent.record,
    color: THEME.colors.text.primary
  });

  const gainSlider = Range("Gain", 0, 0).setMargin(THEME.space.xxs);
  const panSlider = Range("Pan", 0, 0).setMargin(THEME.space.xxs);
  const mixSlider = Range("Mix", 0, 0).setMargin(THEME.space.xxs);

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
  const title = Txt("CSS Features Playground").setColor(THEME.colors.text.title);
  const subtitle = Txt("Tabbed examples for all style props").setColor(THEME.colors.text.muted);

  const header = VBox("Header", 0, 0)
    .setBackground(THEME.colors.bg.header)
    .setPadding(THEME.space.md)
    .setGap(THEME.space.xs)
    .add(title, subtitle);

  const tabBarTitle = Txt("Tabs").setColor(THEME.colors.text.accent);
  const tabBar = HRow("TabBar", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.sm
    })
    .add(
      makeTabButton("Layout", TABS.LAYOUT),
      makeTabButton("Sizing", TABS.SIZING),
      makeTabButton("Controls", TABS.CONTROLS)
    );

  const content = buildTabContent();

  const root = VBox("Root", 0, 0)
    .setGradient(THEME.colors.bg.appFrom, THEME.colors.bg.appTo, true)
    .setPadding(THEME.space.lg)
    .add(header, tabBarTitle, tabBar, ...content);

  JuceUI.render(root);
}

renderApp();
