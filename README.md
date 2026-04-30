# js-juce

`js_juce` is a JUCE module that renders native JUCE GUI from JS/JSX using QuickJS.

## v1 features

- QuickJS runtime inside JUCE app
- Class-first JS API (`View`, `Text`, `Button`, `Slider`, `TextInput`, `Row`, `Column`)
- JSX-like element factory API (`JuceUI.createElement`) is still available
- Native widgets: `View`, `Text`, `Button`, `Slider`, `TextInput`, `Row`, `Column`
- Auto live reload by watching the entry JS file

## JS contract

Your JS entry file must call `JuceUI.render(...)`.

Example (class syntax):

```js
JuceUI.render(
  new Column({}, [
    new Text("Hello from JS"),
    new Button("Click"),
    new Slider({}),
    new TextInput({ text: "Edit me" })
  ])
);
```

## C++ usage

```cpp
js_juce::JsBridge bridge;
bridge.attach(*this, juce::File("/absolute/path/to/app.js"), true);
```

Set `true` as third argument to enable automatic reload on file save.

## JSX usage

JSX can be used with your own transform setup by mapping JSX factory to `JuceUI.createElement`.
This module intentionally keeps the runtime simple and does not include a JSX compiler.