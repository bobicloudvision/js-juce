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

Default playground entry is `app.js`.
Optional JSX source (`app.jsx`) is supported when you point to it explicitly (for example via `JS_JUCE_APP_JSX`) or place it where discovery can find it.

Install esbuild:

```bash
npm i -g esbuild
```

Then write JSX like:

```jsx
function App() {
  return (
    <>
      <div style={{ display: "flex", gap: 8 }}>
        <button text="Click me" />
      </div>
    </>
  );
}

JuceUI.render(<App />);
```

`JsBridge` detects `.jsx` entry files and transpiles them on reload using:
- JSX factory: `JuceUI.createElement`
- JSX fragment: `JuceUI.Fragment`

If esbuild is not on PATH, set `JS_JUCE_ESBUILD` to its executable path.