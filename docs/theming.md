The SynthCore library includes a very powerful theming system, that allows complete UI customisation with images and JSON files. Themes are defined in JSON, but SynthCore also supports [HJSON](https://hjson.github.io/), which is a more readable version of JSON. All examples on this page will be in HSJON.

# Theme File
The theme file is written in JSON (or HJSON). This JSON file has a certain layout, which is mostly defined by a [schema](#schema), but also has some hard-coded sections:
```yaml
{
  theme-name: <name-of-theme>

  functions: { ... }
  variables: { ... }
  images: { ... }
  fonts: { ... }

  ...
}
```

## Variable
A variable, as defined in the variables section, is nothing more than some JSON that will be pasted as-is in the location where it is used. So for example:
```yaml
{
  variables: {
    $myvar: [0, 233, 125]
  }

  some-color: $myvar
}
```
Would be evaluated to:
```yaml
{
  some-color: [0, 233, 125]
}
```
This will also evaluate recursive variable dependencies up to a depth of 30, if it exceeds this depth, it will fail to load the theme.


## Function
A function is a string containing an expression which can be used in other expressions in the rest of the theme file. Here's an example:
```yaml
$myfun: sin($0) + cos($1)
```
The function is called `myfun` and has 2 arguments, this function can later be used inside other expressions like this:
```yaml
field: $myfun(1.0, $value)
```
In this case the value of `field` will be evaluated by calling `myfun` with the arguments `1.0`, and whatever value is in `$value`. `$value` is a variable that is defined by the theme element. For example, if this field would be a graphic for a `Knob`, the `$value` variable would contain the value of that knob. 

## Image
In the images section you can embed image files:
```yaml
{
  images: {
    $myimage::png: <some-base64-encoded-image-file>
  }

  background-image: $myimage::png
}
```
All these images will be loaded first, and then reused wherever they are used in the theme. This way the image is only loaded once. The name does not need to end in the file extension like in the example. The way the images are embedded is by converting the file as binary data to base64. This is the same way a [Blob](https://developer.mozilla.org/en-US/docs/Web/API/Blob) works in JavaScript.

Instead of embedding the image like this, you can also use a path relative to this JSON file:
```yaml
{
  background-image: myimage.png
}
```
However, this is not practical if you want to distribute this theme.

## Font
In the font section you can embed font files:
```yaml
{
  fonts: {
    $myfont::ttf: <some-base64-encoded-font-file>
  }

  main-font: {
    font: $myfont::ttf
    size: 12
  }
}
```
This works exactly the same way as the images section. The fonts are loaded first, and then reused wherever they are used in the theme.

Just like images, you don't have to embed the font files in the JSON file:
```yaml
{
  main-font: {
    font: myfont.ttf
    size: 12
  }
}
```

## Extend
Another useful feature of the theme file is the ability to extend variables. This is useful when you have several graphics that all have something in common. A good use of this would be text:

```yaml
{
  theme-name: My Theme

  variables: {
    $font: {
      font: my-font.ttf
      size: 12
    }

    $text-color: [200]

    $text: {
      font: $font
      text-color: $text-color
    }
  }

  my-drawable-element: {
    extends: $text
    text: Some string of text
  }

  other-drawable-element: {
    extends: $text
    text: Another string of text
  }
}
```
In this example we define a `$text` variable that sets the font and text color, so we can extend this variable in every place where we want to draw some text, so we do not need to set the font and text color everywhere separately.

The way this extend feature works is pretty simple. The JSON objects are merged, so in the above example, after evaluation it would be this:
```yaml
{
  theme-name: My Theme

  my-drawable-element: {
    font: {
      font: my-font.ttf
      size: 12
    }
    text-color: [200]
    text: Some string of text
  }

  other-drawable-element: {
    font: {
      font: my-font.ttf
      size: 12
    }
    text-color: [200]
    text: Another string of text
  }
}
```

# Stateful
Pretty much all fields in all of these theme elements support stateful values. Meaning, different values depending on the component's state, like Hovering, Pressed, Selected, etc. There are 2 ways to define a value for a certain state; the first one is inside the field itself:
```yaml
theme-element: {
  field: {
    value: <normal-value>
    hovering: <hovering-value>
  }
}
```
or you can have a whole section inside a theme element to define values for a certain state, like this:
```yaml
theme-element: {
  field: <normal-value>
  hovering: {
    field: <hovering-value>
  }
}
```
You can also define values for a combination of states:
```yaml
theme-element: {
  field: {
    hovering-selected: <value>
  }
}
```
In the example above, the value will be used when the ui component has both the hovering and selected state. It does not matter in which order you write `hovering-selected`, as long as both `hovering` and `selected` are in the key. You could also write `hovering selected` or `hovering and selected`.

The available states are: `hovering`, `pressed`, `selected`, `enabled`, `disabled`, and `focused`.

# Animated
Some of the fields also support animations, which will happen whenever the value is changed, whether that is because of a state change, or some other change. You can define the transition time and easing curve of a field like this:
```yaml
theme-element: {
  field: {
    transition: <transition-time-in-millis>
    curve: <easing-curve>
    value: <normal-value>

    hovering: {
      transition: <hovering-transition-time>
      curve: <hovering-easing-curve>
      value: <hovering-value>
    }
  }
}
```
As you can see, you can define transition times, and easing curves individually for each state as well. The transition time may not be an [Expression](#expression), it has to be a number. The easing curve can either be a function definition with a single argument, or one of the pre-defined easing curves:
```yaml
ease-in
ease-out
ease-in-out
ease
ease-in-sine
ease-out-sine
ease-in-out-sine
ease-in-quad
ease-out-quad
ease-in-out-quad
ease-in-cubic
ease-out-cubic
ease-in-out-cubic
ease-in-quart
ease-out-quart
ease-in-out-quart
ease-in-quint
ease-out-quint
ease-in-out-quint
ease-in-expo
ease-out-expo
ease-in-out-expo
ease-in-circ
ease-out-circ
ease-in-out-circ
ease-in-back
ease-out-back
ease-in-out-back
ease-in-elastic
ease-out-elastic
ease-in-out-elastic
```
See https://easings.net/ for the definitions of each of these easing curves.


# Expression
Some fields in theme elements support expressions. These are strings that may contain variables, function calls, and other operations like addition, multiplication etc. Here are some examples of expressions:
```yaml
my-color: [255, "$value * 255"]
image-position-x: "20 + sin($value * 10)"
rect-stroke-weight: "$myfun($value, 10)"
```
In the first example, the color's alpha goes from 0-255 depending on the `$value` variable. 

In the second example we move the image x-position also based on `$value`, here we also use the `sin` function, this is one of the globally defined functions you can use in expressions. 

In the final example we call our own custom function to set the rectangle stroke weight. See [Function](#function) how you can define custom functions.

There are a couple global functions you can use inside expressions:
```js
floor(val)           // Floor the float value
trunc(val)           // Trunc the float value
ceil(val)            // Ceil the float value
round(val)           // Round the float value
abs(val)             // Take the absolute value
sqrt(val)            // Square root of value
sin(val)             // Sine
cos(val)             // Cosine
log(val)             // Natural logarithm
exp(val)             // e to the power value
min(a, b)            // Returns the smallest of a and b
max(a, b)            // Returns the biggest of a and b
fmod(a, b)           // Floating point modulo
pow(a, b)            // a to the power b
clamp(val, min, max) // Clamps the value between min and max
```

Expressions can not contain variables defined in the variables section. They are not substituted inside expressions, so unless the variable is defined by the theme element, the variable will evaluate to `0` inside the expression.

# Theme Element Types
There are a couple different theme element types.

## Color
The color element is an RGBA color that is either defined by red, green, blue, and alpha values in the range 0-255, or as a hex color code:
```yaml
color-element: [<grayscale>]
color-element: [<grayscale>, <alpha>]
color-element: [<r>, <g>, <b>]
color-element: [<r>, <g>, <b>, <alpha>]
color-element: "#RGB"
color-element: "#RGBA"
color-element: "#RRGGBB"
color-element: "#RRGGBBAA"
color-element: {
    rgb: [<greyscale>]
    rgb: [<r>, <g>, <b>]
    rgb: "#RGB"
    rgb: "#RRGGBB"
    r: <r>
    g: <g>
    b: <b>
    a: <alpha>
}
```
The color element is both [Stateful](#stateful) and [Animated](#animated), and all the individual color channels, r, g, b, and a, also support [Expression](#expression)s. Here's an example of a color element that makes use of this:
```yaml
color-element: {
    transition: 50
    value: ["$value * 255", 0, 0]
    hovering: [0, "$value * 255", 0]
}
```
In this example the color changes from red to green over a period of 50 milliseconds when you hover over it, and its brightness will depend on `$value`.

You can define a transition time and stateful values for each individual channel as well:
```yaml
color-element: {
  r: {
    transition: 50
    value: $value * 255
    hovering: 0
  }
  g: {
    transition: 100
    value: 0
    hovering: $value * 255
  }
}
```

## Font
A font element is either a font-file, like TTF, or a custom font defined by an image and a json file:
```yaml
font-element: {
  font: font-file.ttf
  size: 16
}
font-element: {
  map: map.png
  description: map.json
}
```
When using a font-file it is mandatory to also define a size. The font can also be a variable that links to a font that is defined in the [fonts](#font) section.

When using a custom font defined by an image and a json file, this is done by an image that contains all the characters you want to support, and a json file that contains the information where to find each character (this does not need to be a separate file, and can also be done inline):
```yaml
{
  default-spacing: 1
  a: {
    location: [0, 0, 6, 14]
  }
  b: {
    location: [6, 0, 6, 14]
  }
  ...
}
```
For each character you want to support you simply add its location in the image. The default spacing is the default amount of pixels between each character, you can modify this per character as well:
```yaml
{
  default-spacing: 1
  ...
  J: {
    location: [71, 0, 6, 14]
    exceptions: [{ 
      post-spacing: 0
      before: ["a", "b", ...]
    }, {
      pre-spacing: -3
      after: ["f", "r"]
    }]
  }
  ...
} 
```
In this instance the capital letter 'J' has some spacing exceptions, you can define as many exceptions as you want. By default the pre-spacing of a character is set to 0, and the post-spacing is set to the `default-spacing`. In this instance, instead of using the default spacing of 1, it will use a post-spacing of 0 if the letter is before any of the characters defined in the `before` field. And instead of the default pre-spacing of 0, it will use a pre-spacing of -3 pixels if it occurs after any of the letters defined in the `after` field. 

For every exception, it will use the defined pre or post-spacing whenever the character before is in the `before` **or** the character after is in `after`. It will take the first exception in the array that matches the contraint.

## Rectangle
A rectangle element is an array of 4 values that are all [Stateful](#stateful), [Animated](#animated), and support [Expression](#expression)s. There are a couple ways of setting the values of a rectangle:
```yaml
rectangle-element: [<x>, <y>, <width>, <height>]
rectangle-element: {
  position: [<x>, <y>]
  size: [<width>, <height>]
  x: <x>
  y: <y>
  width: <width>
  height: <height>
}
```
All of these fields; `x`, `y`, `width`, and `height` can be set independently with animations, expressions, etc.
```yaml
rectangle-element: {
  size: [50, 50]
  x: {
    transition: 50
    value: 0
    hovering: 15
  }
  y: $value * 25
}
```
In this example, the size of the rectangle is fixed. The x-position is animated from a value of 0, to 15 when you hover over it. And the y-position is set by an expression that depends on the variable `$value`.

You can also define a global transition time and curve:
```yaml
rectangle-element: {
  transition: 50
  curve: ease-in

  size: ...
  x: ...
  y: ...
}
```
## Point
A point element works similarly to a [Rectangle](#rectangle), except it has just 2 fields; `x`, and `y`:
```yaml
point-element: [<x>, <y>]
point-element: {
  x: <x>
  y: <y>
}
```

Both of these fields are, just like in the [Rectangle](#rectangle), [Stateful](#stateful), [Animated](#animated), and support [Expression](#expression)s.

## Value
A value element is a single value that is [Stateful](#stateful), [Animated](#animated), and supports [Expression](#expression)s.
```yaml
value-element: <value>
```

So also here you can define state, animations, and use expressions:
```yaml
value-element: {
  transition: 80
  value: $value * 10
  hovering: 50
}
```

## Drawable
A drawable element is the most complex theme element. It allows for layered drawing of images, rectangles, and text. A drawable consists of an arbitrary amount of layers, and each layer can contain a [background color](#background-color), [text](#text), a [rectangle](#rect), and an [image](#image-1).

```yaml
drawable-element: {
  ...
  layers: {
    layer-name: {
      ...
    }
    other-layer: {
      ...
    }
  }
}
```

### Background Color
The background color is the simplest of them all, it's a single field called `background-color` that is [Color](#color).

```yaml
drawable-element: {
  background-color: <some-color>
}
```

### Text
The text part of a drawable element can draw a string of text. The fields of the text part can be defined in a couple ways:
```yaml
drawable-element: {
  text: {
    font: <font>
    content: <text>
    color: <color>
    position: <point>
    position-x: <value>
    position-y: <value>
    align: <align>
    frames: <nof-frames>
    overflow: <overflow>
    round-position: <round-mode>
  }
  text: <text>
  font: <font>
  text-font: <font>
  text-color: <color>
  text-position: <point>
  text-position-x: <value>
  text-position-y: <value>
  text-align: <align>
  text-frames: <integer>
  text-overflow: <overflow>
  text-round-position: <round-mode>
}
```
`<font>` is a [Font](#font) element.

`<color>` is a [Color](#color) element.

`<point>` is a [Point](#point) element.

`<value>` is a [Value](#value) element.

`<align>` is a string containing a valid text align: 
`top-left`, `top-center`, `top-right`, `center-left`, `center`, `center-right`, `bottom-left`, `bottom-center`, `bottom-right`, `top`, `bottom`, `left`, or `right`. This will align the text to a point within the bounds of the drawable. So for example when it is set to `center`, the text will be aligned to the center of the bounds of the drawable. This value is [Stateful](#stateful). The `position` of the text is relative to wherever the `align` put it. So for example, when it is set to `center`, and you have a position of `[0, 10]`, the text will be positioned 10 pixels below the center of the bounds of the drawable. The default value is `top-left`.

`<overflow>` is a string containing a valid overflow mode, there are currently 2: `visible` and `dots`. The default is `visible`, this means if the text overflows outside of the available range, it will simply keep on drawing. In most cases this means the text will be cutoff, as for every element there is a clip region. In the case of `dots`, it will cut the string short, and add three dots '...' to the end. This value is [Stateful](#stateful).

`<round-mode>` can be used to snap the text to an integer position. This is useful when you have an align `center`, and it would have a non-integer position. The available round modes are: `floor`, `ceil`, `round`, `trunc`, and `none`. The default value is `trunc`. This value is [Stateful](#stateful).

`<text>` can either be a string, or an array:
```yaml
text: A string of text
text: ["First", "Second", "Third"]
text: Thing $value
```
In the first instance, this text part simply displays a static string of text. 

The 2nd example will show one of the strings of text defined in the array, based on the linked variable. For example, by default for a knob this would be linked to its value. This is useful if the knob has a discrete amount of values.

The 3rd example is a string that contains a variable. In this case, `$value` will be substituted with the formatted value of the linked parameter, if there is one. There are a couple variables you can use in a text string:
```js
// These can be used anywhere
$frame            // 1-indexed frame number
$0frame           // 0-indexed frame number
$normalized-value // normalized value
// These can be used when it is linked to a parameter
$value            // Formatted value of the parameter
$name             // Name of the parameter
$short-name       // Short name of the parameter
$identifier       // Full identifier of the parameter
$short-identifier // Short identifier of the parameter
$var-name         // Variable name of the parameter
$full-var-name    // Full variable name of the parameter
$steps            // Number of steps as defined in the parameter
```
The first three of these can be used for any drawable element, and the rest only work when it is linked to a parameter. 

The `$frame` variable can be set in 2 ways: the first way is to define the text as an array, as we saw before. The `$frame` variable will then be set to the index in the array depending on the `$normalized-value` of the drawable element. The other way is to explicitly define how many frames there are using the `frames` field:
```yaml
frames: 3
frames: {
  value: 3
  hovering: 5
}
```
The frames field also supports [Stateful](#stateful), it does not support [Animated](#animated) or [Expression](#expression)s.

### Rect
The rect part of the drawable element draws a rectangle:
```yaml
rect: {
  fill: <color>
  stroke: <color>
  stroke-weight: <value>
  align: <align>
  dimensions: <rectangle>
  position: <point>
  size: <point>
  x: <value>
  y: <value>
  width: <value>
  height: <value>
}
rect-fill: <color>
rect-stroke: <color>
rect-stroke-weight: <value>
rect-align: <align>
rect-dimensions: <rectangle>
rect-position: <point>
rect-size: <point>
rect-x: <value>
rect-y: <value>
rect-width: <value>
rect-height: <value>
```

`<color>` is a [Color](#color) element.

`<rectangle>` is a [Rectangle](#rectangle) element.

`<point>` is a [Point](#point) element.

`<value>` is a [Value](#value) element.

`<align>` is a string containing a valid align: 
`top-left`, `top-center`, `top-right`, `center-left`, `center`, `center-right`, `bottom-left`, `bottom-center`, `bottom-right`, `top`, `bottom`, `left`, or `right`. This will align the rectangle to a point within the bounds of the drawable. So for example when it is set to `center`, the rectangle will be aligned to the center of the bounds of the drawable. This value is [Stateful](#stateful). The `position` of the rectangle is relative to wherever the `align` put it. So for example, when it is set to `center`, and you have a position of `[0, 10]`, the rectangle will be positioned 10 pixels below the center of the bounds of the drawable. The default value is `top-left`.

### Image
The image part of the drawable draws an image. These are the available fields:
```yaml
image: {
  source: <image>
  align: <align>
  clip: <rectangle>
  offset: <point>
  offset-x: <value>
  offset-y: <value>
  size: <point>
  width: <value>
  height: <value>
  position: <rectangle>
  position-x: <value>
  position-y: <value>
  position-width: <value>
  position-height: <value>
  # For tiled:
  edges: [<integer>, <integer>, <integer>, <integer>]
  edges: [<integer>, <integer>]
  # For multiframe:
  frames: <integer>
  frames-per-row: <integer>
  frames-repeat: <integer>
}

image: <image>
image-align: <align>
image-clip: <rectangle>
image-offset: <point>
image-offset-x: <value>
image-offset-y: <value>
image-size: <point>
image-width: <value>
image-height: <value>
image-position: <rectangle>
image-position-x: <value>
image-position-y: <value>
image-position-width: <value>
image-position-height: <value>
# For tiled:
edges: [<integer>, <integer>, <integer>, <integer>]
edges: [<integer>, <integer>]
# For multiframe:
frames: <integer>
frames-per-row: <integer>
frames-repeat: <integer>
```

`<color>` is a [Color](#color) element.

`<rectangle>` is a [Rectangle](#rectangle) element.

`<point>` is a [Point](#point) element.

`<value>` is a [Value](#value) element.

`<image>` is a string containing either a variable that links to an image defined in the images section, or a relative path to an image file.

`<align>` is a string containing a valid align: 
`top-left`, `top-center`, `top-right`, `center-left`, `center`, `center-right`, `bottom-left`, `bottom-center`, `bottom-right`, `top`, `bottom`, `left`, or `right`. This will align the image to a point within the bounds of the drawable. So for example when it is set to `center`, the image will be aligned to the center of the bounds of the drawable. This value is [Stateful](#stateful). The `position` of the image is relative to wherever the `align` put it. So for example, when it is set to `center`, and you have a position of `[0, 10]`, the image will be positioned 10 pixels below the center of the bounds of the drawable. The default value is `top-left`.

The `clip`, `offset-x`, `offset-y`, `width`, `height`, `offset`, and `size` fields are all for selecting a part in the image. Imagine you have a 100x100 image, and you set clip to `[0, 0, 50, 50]`, you take only the top-left 50x50 part of the image.

A multi-frame image is an image that draws a certain part of the image, based on a linked value. You must define at least the amount of frames in a multi-frame. If you define a `size`, it will use that size to determine where to take a certain frame from. If you do not define a `size`, it will automatically calculate the size using the defined `frames` and `frames-per-row`. The `frames-repeat` field just loops the frames n times. By default this is set to 1.

A tiled image is an image is a 9-tiled image. Also refered to as a [9-patch image](https://medium.com/flobiz-blog/create-resizable-bitmaps-9-patch-files-48c774db4526). You can define the edge sizes using the `edges` field. 

The `position` field positions the image inside the bounds of the drawable. This is done relative to the `align`.

### Layer
A drawable element can consist of multiple layers. Each layer can contain an [Image](#image-1), [Text](#text), [Rect](#rect), and [BackgroundColor](#background-color):

```yaml
drawable-element: {
  rect: { ... }
  background-color: <color>
  text: { ... }
  image: { ... }

  layers: {
    my-layer: {
      rect: { ... }
      background-color: <color>
      text: { ... }
      image: { ... }
    }

    another-layer: {
      rect: { ... }
      background-color: <color>
      text: { ... }
      image: { ... }
    }
  }
}
```

A layer can also be conditionally active based on an expression:
```yaml
drawable-element: {
  layers: {
    my-layer: {
      if: $value > 0.5
      ...
    }
  }
}
```

In the example above, the layer will only be visible if the `$value` is bigger than 0.5. You can use any expression inside the `if` field of a layer, the result will always be cast to a boolean.

By default, a multi-frame image is linked to the main value of a drawable element. In the case of a knob, this would be its value. But sometimes a theme element defines multiple values, in this case you can link a layer to another variable:
```yaml
drawable-element: {
  layers: {
    my-layer: {
      link: $modulated-value
      ...
    }
  }
}
```
In the above example the layer is now linked to the modulated value of the knob, instead of the normal value. The available variables depend on the element.

This linking also affects the text part of the layer, if the `content` field of the text was defined as an array of strings.

## TextArea
A text area element is simply a collection of other elements, namely:
```yaml
font: <font>
text-color: <color>
placeholder-color: <color>
selection-color: <color>
caret-color: <color>
background: <drawable>
```

`<font>` is a [Font](#font) element.

`<color>` is a [Color](#color) element.

`<drawable>` is a [Drawable](#drawable) element.

# Schema
The available theme elements are defined by the synth in a schema JSON file. This is a JSON file that contains the names of all theme elements, and their types. Here is a small example of a schema file:
```json
{
  "background": "drawable",
  "description": "text-area",
  "oscillator": {
    "background": "drawable",
    "oscilloscope-color": "color",
    "parameters": {
      "pitch": "drawable",
      "gain": "drawable"
    }
  }
}
```

A possible theme file could then be:
```yaml
{
  theme-name: My Theme

  variables: {
    $font: {
      font: my-font.ttf
      size: 12
    }

    $text-color: [200]

    $text: {
      font: $font
      text-color: $text-color
    }
  }

  background: background.png
  description: {
    extends: $text
    caret-color: $text-color
    selection-color: [50, 50, 255, 50]
  }

  oscillator: {
    background: oscillator-background.png
    oscilloscope-color: "#12FFA3"
    parameters: {
      pitch: {
        image: pitch-knob.png
        frames: 100
        frames-per-row: 10
      }
      gain: {
        image: gain-knob.png
        frames: 100
        frames-per-row: 10

        extends: $text
        text: $value
        text-align: center
        text-position-y: 10
      }
    }
  }
}
```