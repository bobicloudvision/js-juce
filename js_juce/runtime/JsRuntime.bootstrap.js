const __jsJuceStyleContextStack = [Object.create(null)];

function __jsJuceCurrentStyleContext() {
  return __jsJuceStyleContextStack[__jsJuceStyleContextStack.length - 1];
}

function __jsJuceResolveContextStyle(typeName, ownProps) {
  const scope = __jsJuceCurrentStyleContext();
  const defaults = (scope && scope[typeName]) || {};
  return { ...defaults, ...(ownProps || {}) };
}

function __jsJuceFlattenChildren(input, out = []) {
  if (Array.isArray(input)) {
    for (const item of input) __jsJuceFlattenChildren(item, out);
    return out;
  }
  if (input !== undefined && input !== null) out.push(input);
  return out;
}

function __jsJuceNormalizeStyleProps(props) {
  const source = props || {};
  const style = source.style || {};
  const merged = { ...source };
  for (const key of Object.keys(style)) merged[key] = style[key];

  if (style.backgroundColor !== undefined) merged.background = style.backgroundColor;
  if (style.border !== undefined) merged.borderColor = style.border;
  if (style.borderSize !== undefined) merged.borderWidth = style.borderSize;
  if (style.flexDirection !== undefined) merged.direction = style.flexDirection;
  if (style.justifyContent !== undefined) merged.justify = style.justifyContent;
  if (style.alignItems !== undefined) merged.alignItems = style.alignItems;
  if (style.alignContent !== undefined) merged.alignContent = style.alignContent;
  if (style.alignSelf !== undefined) merged.alignSelf = style.alignSelf;
  if (style.flexWrap !== undefined) merged.wrap = style.flexWrap;
  if (style.flexGrow !== undefined) merged.grow = style.flexGrow;
  if (style.flexShrink !== undefined) merged.shrink = style.flexShrink;
  if (style.flexBasis !== undefined) merged.basis = style.flexBasis;
  if (style.gap !== undefined) merged.gap = style.gap;
  if (style.rowGap !== undefined) merged.rowGap = style.rowGap;
  if (style.columnGap !== undefined) merged.columnGap = style.columnGap;
  if (style.margin !== undefined) merged.margin = style.margin;
  if (style.marginTop !== undefined) merged.marginTop = style.marginTop;
  if (style.marginRight !== undefined) merged.marginRight = style.marginRight;
  if (style.marginBottom !== undefined) merged.marginBottom = style.marginBottom;
  if (style.marginLeft !== undefined) merged.marginLeft = style.marginLeft;
  if (style.gradientFrom !== undefined) merged.gradientFrom = style.gradientFrom;
  if (style.gradientTo !== undefined) merged.gradientTo = style.gradientTo;
  if (style.gradientVertical !== undefined) merged.gradientVertical = style.gradientVertical ? 1 : 0;
  if (style.borderColor !== undefined) merged.borderColor = style.borderColor;
  if (style.borderWidth !== undefined) merged.borderWidth = style.borderWidth;
  if (style.padding !== undefined) merged.padding = style.padding;
  if (style.color !== undefined) merged.color = style.color;
  if (style.background !== undefined) merged.background = style.background;

  delete merged.style;
  return merged;
}

let __jsJuceNextCallbackId = 1;
const __jsJuceControlCallbacks = Object.create(null);

function __jsJuceWrapFunctionsForTransport(input) {
  if (Array.isArray(input)) return input.map(__jsJuceWrapFunctionsForTransport);
  if (input && typeof input === "object") {
    const out = {};
    for (const k of Object.keys(input)) out[k] = __jsJuceWrapFunctionsForTransport(input[k]);
    return out;
  }
  if (typeof input === "function") {
    const id = "cb_" + (__jsJuceNextCallbackId++);
    __jsJuceControlCallbacks[id] = input;
    return { __fnId: id };
  }
  return input;
}

function __jsJuceNormalizeNode(node) {
  if (node == null) {
    return __jsJuceCreateElement("View", {});
  }

  if (node && typeof node.toElement === "function") {
    return __jsJuceNormalizeNode(node.toElement());
  }

  if (typeof node === "string" || typeof node === "number" || typeof node === "boolean") {
    return __jsJuceCreateElement("Text", { text: String(node) });
  }

  const rawChildren = Array.isArray(node.children) ? node.children : [];
  const children = __jsJuceFlattenChildren(rawChildren).map(__jsJuceNormalizeNode);
  return __jsJuceCreateElement(
    node.type || "View",
    __jsJuceWrapFunctionsForTransport(__jsJuceNormalizeStyleProps(node.props || {})),
    ...children
  );
}

class View {
  constructor(props = {}, children = []) {
    const hasProps = props != null && typeof props === "object" && !Array.isArray(props) && typeof props.toElement !== "function";
    const effectiveProps = hasProps ? props : {};
    const effectiveChildren = hasProps ? children : props;
    this.props = __jsJuceResolveContextStyle(this.type(), __jsJuceNormalizeStyleProps(effectiveProps));
    this.children = __jsJuceFlattenChildren(effectiveChildren);
  }
  type() { return "View"; }
  style(nextProps = {}) {
    this.props = { ...this.props, ...__jsJuceNormalizeStyleProps(nextProps) };
    return this;
  }
  css(nextStyle = {}) { return this.style({ style: nextStyle }); }
  setStyle(nextStyle = {}) { return this.css(nextStyle); }
  background(value) { return this.style({ background: value }); }
  setBackground(value) { return this.background(value); }
  gradient(from, to, vertical = true) {
    return this.style({ gradientFrom: from, gradientTo: to, gradientVertical: vertical ? 1 : 0 });
  }
  setGradient(from, to, vertical = true) { return this.gradient(from, to, vertical); }
  setPluginBackground(value) { return this.setBackground(value); }
  setPluginGradient(from, to, vertical = true) { return this.setGradient(from, to, vertical); }
  border(color, width = 1) { return this.style({ borderColor: color, borderWidth: width }); }
  setBorder(color, width = 1) { return this.border(color, width); }
  padding(value) { return this.style({ padding: value }); }
  setPadding(value) { return this.padding(value); }
  margin(value) { return this.style({ margin: value }); }
  setMargin(value) { return this.margin(value); }
  setSize(width, height) { return this.style({ width, height }); }
  setMinSize(minWidth, minHeight) { return this.style({ minWidth, minHeight }); }
  setMaxSize(maxWidth, maxHeight) { return this.style({ maxWidth, maxHeight }); }
  setFlex(grow = 1, shrink = 1, basis = 0) { return this.style({ grow, shrink, basis }); }
  setFlexGrow(grow) { return this.style({ grow }); }
  setFlexShrink(shrink) { return this.style({ shrink }); }
  setFlexBasis(basis) { return this.style({ basis }); }
  setAlignSelf(value) { return this.style({ alignSelf: value }); }
  setDirection(value) { return this.style({ direction: value }); }
  setWrap(value) { return this.style({ wrap: value }); }
  setJustify(value) { return this.style({ justify: value }); }
  setAlignItems(value) { return this.style({ alignItems: value }); }
  setAlignContent(value) { return this.style({ alignContent: value }); }
  setGap(value) { return this.style({ gap: value }); }
  setRowGap(value) { return this.style({ rowGap: value }); }
  setColumnGap(value) { return this.style({ columnGap: value }); }
  setFontFamily(value) { return this.style({ fontFamily: value }); }
  setFontFile(value) { return this.style({ fontFile: value }); }
  setFontSize(value) { return this.style({ fontSize: value }); }
  setFontWeight(value) { return this.style({ fontWeight: value }); }
  color(value) { return this.style({ color: value }); }
  setColor(value) { return this.color(value); }
  setPosition(x, y) { return this.style({ x, y }); }
  add(...children) {
    this.children.push(...__jsJuceFlattenChildren(children));
    return this;
  }
  toElement() { return __jsJuceCreateElement(this.type(), this.props, ...this.children.map(__jsJuceNormalizeNode)); }
}

class Row extends View { type() { return "Row"; } }
class Column extends View { type() { return "Column"; } }

class Text extends View {
  constructor(textOrProps, props = {}) {
    if (typeof textOrProps === "string" || typeof textOrProps === "number" || typeof textOrProps === "boolean") {
      super({ ...props, text: String(textOrProps) }, []);
      return;
    }
    const p = textOrProps || {};
    const text = p.text ?? p.children ?? "";
    super({ ...p, text: String(text) }, []);
  }
  type() { return "Text"; }
}

class Button extends View {
  constructor(textOrProps, props = {}) {
    if (typeof textOrProps === "string" || typeof textOrProps === "number" || typeof textOrProps === "boolean") {
      super({ ...props, text: String(textOrProps) }, []);
      return;
    }
    const p = textOrProps || {};
    const text = p.text ?? p.children ?? "";
    super({ ...p, text: String(text) }, []);
  }
  type() { return "Button"; }
}

class Slider extends View {
  constructor(props = {}) { super(props, []); }
  type() { return "Slider"; }
}

class TextInput extends View {
  constructor(props = {}) { super(props, []); }
  type() { return "TextInput"; }
}

function h(type, props, ...children) {
  if (typeof type === "function") {
    return type({ ...(props || {}), children });
  }
  return __jsJuceCreateElement(
    type,
    __jsJuceWrapFunctionsForTransport(__jsJuceNormalizeStyleProps(props || {})),
    ...__jsJuceFlattenChildren(children).map(__jsJuceNormalizeNode)
  );
}

const view = (props, ...children) => new View(props, children);
const row = (props, ...children) => new Row(props, children);
const column = (props, ...children) => new Column(props, children);
const text = (value, props) => new Text(value, props);
const button = (value, props) => new Button(value, props);
const slider = (props) => new Slider(props);
const textInput = (props) => new TextInput(props);

globalThis.JuceUI = {
  createElement: function(type, props, ...children) {
    return h(type, props, ...children);
  },
  h,
  render: function(tree) {
    __jsJuceRender(__jsJuceNormalizeNode(tree));
  },
  withStyleContext: function(contextObject, buildFn) {
    const parent = __jsJuceCurrentStyleContext();
    const merged = Object.create(parent || null);
    for (const k of Object.keys(contextObject || {}))
      merged[k] = { ...(parent && parent[k] ? parent[k] : {}), ...(contextObject[k] || {}) };

    __jsJuceStyleContextStack.push(merged);
    try {
      return buildFn();
    } finally {
      __jsJuceStyleContextStack.pop();
    }
  }
};

globalThis.__jsJuceInvokeControlCallback = function(callbackId, value) {
  const fn = __jsJuceControlCallbacks[callbackId];
  if (typeof fn === "function")
    fn(value);
};

globalThis.JSJuce = {
  View: (nameOrProps, x, y) => new View({ name: nameOrProps, x, y }),
  Row: (nameOrProps, x, y) => new Row({ name: nameOrProps, x, y }),
  Column: (nameOrProps, x, y) => new Column({ name: nameOrProps, x, y }),
  Text: (nameOrText, x, y) => new Text(nameOrText, { x, y }),
  Button: (nameOrText, x, y) => new Button(nameOrText, { x, y }),
  Slider: (nameOrProps, x, y) => new Slider({ name: nameOrProps, x, y }),
  TextInput: (nameOrProps, x, y) => new TextInput({ name: nameOrProps, x, y })
};

globalThis.View = View;
globalThis.Row = Row;
globalThis.Column = Column;
globalThis.Text = Text;
globalThis.Button = Button;
globalThis.Slider = Slider;
globalThis.TextInput = TextInput;
globalThis.view = view;
globalThis.row = row;
globalThis.column = column;
globalThis.text = text;
globalThis.button = button;
globalThis.slider = slider;
globalThis.textInput = textInput;
