const Btn = JSJuce.Button;
const Txt = JSJuce.Text;
const HRow = JSJuce.Row;
const VBox = JSJuce.Column;
const Input = JSJuce.TextInput;
const Range = JSJuce.Slider;
const Div = JSJuce.div;

const APP_FONT = JSJuce.Font({
  fontFile: "examples/playground/assets/fonts/Outfit[wght].ttf",
  fontFamily: "Outfit",
  fontSize: 14,
  fontWeight: "400"
});

const GLOBAL_STYLE = {
  boxSizing: "border-box"
};
JuceUI.setGlobalStyle(GLOBAL_STYLE);

const THEME = {
  colors: {
    bg: {
      appFrom: "#111318",
      appTo: "#161a22",
      header: "#1a1f28",
      surface: "#202632",
      input: "#252d3a"
    },
    text: {
      primary: "#f3f6fb",
      muted: "#aeb8c8",
      title: "#dbe7ff",
      section: "#c5d5f2",
      accent: "#9db4d8",
      dark: "#111111"
    },
    tab: {
      active: "#3f5f8f",
      inactive: "#2b3443"
    },
    accent: {
      a: "#4c6488",
      b: "#506688",
      c: "#556b8e",
      d: "#5b7194",
      play: "#4d6386",
      stop: "#586d8f",
      record: "#4a607f"
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
    tabMinW: 100,
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
  BLOCK: "block",
  SIZING: "sizing",
  POSITION: "position",
  CONTROLS: "controls",
  TYPOGRAPHY: "typography",
  DEBUG: "debug"
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
  const title = Txt("Layout: flex + gaps").setColor(THEME.colors.text.section);
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

function buildBlockTab() {
  const title = Txt("Block: display:block stacks children").setColor(THEME.colors.text.section);
  const blockHost = Div("BlockHost", 0, 0).css({
    ...cardBase(),
    display: "block",
    paddingTop: 10,
    paddingBottom: 10,
    paddingLeft: 12,
    paddingRight: 12,
    borderTopWidth: 2,
    borderBottomWidth: 2,
    borderLeftWidth: 1,
    borderRightWidth: 1,
    borderColor: THEME.colors.accent.a,
    boxSizing: "border-box"
  });

  const a = Btn("Block item 1", 0, 0).setBackground(THEME.colors.accent.b).setColor(THEME.colors.text.primary)
    .setSize(200, 28)
    .setMargin(THEME.space.xxs);
  const b = Btn("Block item 2", 0, 0).setBackground(THEME.colors.accent.c).setColor(THEME.colors.text.primary)
    .setSize(240, 28)
    .setMargin(THEME.space.xxs);

  blockHost.add(a, b);
  return [title, blockHost];
}

function buildSizingTab() {
  const title = Txt("Sizing: flex + % width + border-box").setColor(THEME.colors.text.section);
  const sizingDemo = HRow("SizingDemo", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.md
    });

  const fixedLeft = Input({ text: "width=180" })
    .setColor(THEME.colors.text.primary)
    .setBackground(THEME.colors.bg.input)
    .setSize(180, 30)
    .setMargin(THEME.space.sm);

  const pctCenter = Input({ text: "width ~45% of row" })
    .setColor(THEME.colors.text.primary)
    .setBackground(THEME.colors.bg.input)
    .css({ width: "45%", minWidth: 120, height: 30, boxSizing: "border-box" })
    .setMargin(THEME.space.sm);

  const flexibleRight = Input({ text: "grow=1" })
    .setColor(THEME.colors.text.primary)
    .setBackground(THEME.colors.bg.input)
    .setMinSize(90, 30)
    .setFlex(1, 1, 0)
    .setMargin(THEME.space.sm);

  sizingDemo.add(fixedLeft, pctCenter, flexibleRight);
  return [title, sizingDemo];
}

function buildPositionTab() {
  const title = Txt("Position: absolute + zIndex").setColor(THEME.colors.text.section);
  const host = Div("PosHost", 0, 0).css({
    ...cardBase(),
    backgroundColor: "#252d3a",
    minHeight: 100,
    padding: 8,
    position: "relative",
    boxSizing: "border-box"
  });
  const hint = Txt("Corner badge is position:absolute inside relative host").setColor(THEME.colors.text.muted)
    .setMargin(4);
  const badge = Btn("Absolute", 0, 0).setFontSize(13).css({
    position: "absolute",
    right: 10,
    bottom: 10,
    width: 160,
    height: 34,
    zIndex: 2,
    backgroundColor: "#3f5f8f",
    color: "#f3f6fb",
    textAlign: "center"
  });
  const zDemo = Txt("z-order: grey behind, violet on top").setColor(THEME.colors.text.muted).css({ marginTop: 52 });
  const back = Div("ZBack", 0, 0).css({ zIndex: 0, backgroundColor: "#2b3443", width: 80, height: 40, position: "absolute", left: 8, top: 8 });
  const front = Div("ZFront", 0, 0).css({ zIndex: 3, backgroundColor: "#556b8e", width: 80, height: 40, position: "absolute", left: 28, top: 20 });
  host.add(hint, badge, zDemo, back, front);
  return [title, host];
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

function buildTypographyTab() {
  const title = Txt("Typography: weight 100–900, italic, lineHeight, letterSpacing, textAlign").setColor(THEME.colors.text.section);
  const typography = VBox("Typography", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.sm
    });

  const display = Txt("Outfit 24 / weight 800").setColor(THEME.colors.text.title)
    .setFontSize(24)
    .setFontWeight("800");

  const italic = Txt("Italic + letterSpacing").setColor(THEME.colors.text.section)
    .css({ fontStyle: "italic", letterSpacing: 1.2, fontSize: 15 });

  const spaced = Txt("Line height ~1.8 (multiplier)").setColor(THEME.colors.text.primary)
    .css({ lineHeight: 1.8, fontSize: 14 });

  const centered = Txt("Centered label").setColor(THEME.colors.text.accent)
    .css({ textAlign: "center", width: "100%", fontSize: 14 });

  const fallbackNote = Txt("fontFamily fallbacks: \"Georgia, serif\"").setColor(THEME.colors.text.muted)
    .css({ fontFamily: "Georgia, serif", fontSize: 12 });

  const input = Input({ text: "Input: centered text" })
    .setBackground(THEME.colors.bg.input)
    .setColor(THEME.colors.text.primary)
    .css({ textAlign: "center", fontSize: 14 });

  const action = Btn("Button 600 weight", 0, 0).css({
    backgroundColor: THEME.colors.accent.play,
    color: THEME.colors.text.primary,
    fontWeight: "600",
    fontSize: 14
  });

  const semantic = Txt("Semantic: JuceUI.h(\"p\", ...) maps to Text (same as Txt)").setColor(THEME.colors.text.section)
    .css({ color: THEME.colors.text.muted, fontSize: 12 });

  typography.add(display, italic, spaced, centered, fallbackNote, input, action, semantic);
  return [title, typography];
}

function buildDebugTab() {
  const title = Txt("Debug: layout overlay + style tree log").setColor(THEME.colors.text.section);
  const flags = JuceUI.getDebugFlags();
  const row = VBox("DebugCol", 0, 0).css({ ...cardBase(), gap: THEME.space.sm });
  const state = Txt("layout=" + flags.layout + " styleDump=" + flags.styleDump).setColor(THEME.colors.text.muted);
  const b1 = Btn("Toggle layout overlay", 0, 0).css({ backgroundColor: THEME.colors.accent.a, color: THEME.colors.text.primary });
  b1.style({ onControl: function() {
    const f = JuceUI.getDebugFlags();
    JuceUI.setDebugFlags({ layout: !f.layout });
    renderApp();
  }});
  const b2 = Btn("Toggle style console dump", 0, 0).css({ backgroundColor: THEME.colors.accent.b, color: THEME.colors.text.primary });
  b2.style({ onControl: function() {
    const f = JuceUI.getDebugFlags();
    JuceUI.setDebugFlags({ styleDump: !f.styleDump });
    renderApp();
  }});
  row.add(state, b1, b2);
  return [title, row];
}

function buildTabContent() {
  if (activeTab === TABS.BLOCK) return buildBlockTab();
  if (activeTab === TABS.SIZING) return buildSizingTab();
  if (activeTab === TABS.POSITION) return buildPositionTab();
  if (activeTab === TABS.CONTROLS) return buildControlsTab();
  if (activeTab === TABS.TYPOGRAPHY) return buildTypographyTab();
  if (activeTab === TABS.DEBUG) return buildDebugTab();
  return buildLayoutTab();
}

function renderApp() {
  const title = Txt("CSS Features Playground").setColor(THEME.colors.text.title);
  const subtitle = Txt("Milestones A–D: box model, sizing, position, typography, debug").setColor(THEME.colors.text.muted);

  const header = VBox("Header", 0, 0)
    .setBackground(THEME.colors.bg.header)
    .setPadding(THEME.space.md)
    .setGap(THEME.space.xs)
    .add(title, subtitle);

  const tabBarTitle = Txt("Tabs").setColor(THEME.colors.text.accent);
  const tabBar = HRow("TabBar", 0, 0)
    .css({
      ...cardBase(),
      gap: THEME.space.sm,
      flexWrap: "wrap"
    })
    .add(
      makeTabButton("Layout", TABS.LAYOUT),
      makeTabButton("Block", TABS.BLOCK),
      makeTabButton("Sizing", TABS.SIZING),
      makeTabButton("Position", TABS.POSITION),
      makeTabButton("Controls", TABS.CONTROLS),
      makeTabButton("Typography", TABS.TYPOGRAPHY),
      makeTabButton("Debug", TABS.DEBUG)
    );

  const content = buildTabContent();

  const root = VBox("Root", 0, 0)
    .setGradient(THEME.colors.bg.appFrom, THEME.colors.bg.appTo, true)
    .setPadding(THEME.space.lg)
    .add(header, tabBarTitle, tabBar, ...content);
  APP_FONT.applyTo(root);

  JuceUI.render(root);
}

renderApp();
