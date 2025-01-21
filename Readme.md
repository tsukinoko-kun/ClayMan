# ClayMan

### ClayMan is a C++ wrapper library for the `clay.h` library found [here](https://github.com/nicbarker/clay/). This library (as well as Clay), is a work in progress, but is ready to be used.

## About Clay
If you are not already familiar with `Clay`, a high-performance UI layout library in C, [go check it out](https://github.com/nicbarker/clay/)!
The Clay library is a masterclass in high-performance low-level layout management that handles your UI calculations in real time in immediate mode with blazing speeds.
The original Clay API is based around a macro system, where you call the macro structure each frame, like this:

```C
Clay_BeginLayout();

// Parent element with 8px of padding
CLAY(CLAY_LAYOUT({ .padding = CLAY_PADDING_ALL(8) })) {
    // Child element 1
    CLAY_TEXT(CLAY_STRING("Hello World"), CLAY_TEXT_CONFIG({ .fontSize = 16 }));
    // Child element 2 with red background
    CLAY(CLAY_RECTANGLE({ .color = COLOR_RED })) {
        // etc
    }
}

Clay_RenderCommandArray renderCommands = Clay_EndLayout();
```
The result is a nice package of all the information a renderer needs to show the layout (and Clay has several options!).
While this is an innovative and clever way to do things, you may find yourself wanting to avoid macros or to use a more object-oriented approach. This is extremely difficult to do in C. However, in C++, we have a few tools at our disposal. Enter, `ClayMan`.

## About ClayMan

`ClayMan` is mainly a wrapper class for ClayMan. It handles the boilerplate and provides a flexible interface for Clay with a few handy additions

### Features

- Handles the initialization of ClayMan.
- Exposes the underlying functions of Clay in a simplified form.
- Manages Clay_Strings to allow for std::string usage.
- Provides handy utility functions for configuration parameters.
- Allows to still use classic Clay layout macros.
- Holds closely to Clay's original macro API.
- Provides several flexible ways to create your layout:

Pass params and child-lambda:
```cpp
ClayMan.element(
    "someID", 
    (Clay_LayoutConfig){.padding = {1, 2, 3, 4}},
    [&]{
        /*children here*/
    }
);
```

Or pass params then manually close:
```cpp
ClayMan.openElementWithParams(
    "someID", 
    {.padding = {1, 2, 3, 4}}
);

/*children here*/

ClayMan.closeElement();
```

Or manually open, configure, and close:
```cpp
ClayMan.openElement();
ClayMan.applyID("someID");
ClayMan.applyLayout({.padding = {1, 2, 3, 4}});
ClayMan.endConfig();

/*children here*/

ClayMan.closeElement();
```

Or use classic Clay macros:
```cpp
CLAY(
    CLAY_ID("someID")
    CLAY_LAYOUT({.padding = {1, 2, 3, 4}})
){
    /*children here*/
}
```

Or mix and match, just be sure that each element sticks to one method:
```cpp
ClayMan.openElementWithParams(
    "greatgreatgrandparent",
    (Clay_LayoutConfig){.padding = {4, 5, 6, 7}}
);{ //children of "greatgreatgrandparent" inside this block
    ClayMan.element(
        "greatgrandparent",
        (Clay_LayoutConfig){.padding = {4, 5, 6, 7}}
        [&]{ //children of "greatgrandparent" inside this lambda
            CLAY(
                CLAY_ID("grandparent")
                CLAY_LAYOUT({.padding = 1, 2, 3, 4})
            ){ //children of "grandparent" inside this block
                ClayMan.openElement(); //open "parent"
                ClayMan.applyID("parent");
                ClayMan.applyLayout({.padding = {4, 5, 6, 7}});
                ClayMan.endConfig(); //end "parent" config
                {//children of "parent" in this block
                    ClayMan.openElementWithParams("child", {.padding = {1, 2, 3, 4}});
                    {
                        //etc...
                    }
                    ClayMan.closeElement(); //close "child"
                }
                ClayMan.closeElement(); //close "parent"

                ClayMan.element(
                    "sibling",
                    (Clay_LayoutConfig){.padding = {4, 5, 6, 7}},
                    [&]{
                        //children of "sibling"
                    }
                );
            }
        }
    );
}ClayMan.closeElement(); //close "greatgreatgrandparent"
```

## Closing Elements
If using the classic Clay macros or using ClayMan's `element` function, closing of elements is automatic. The main catch with `openElement` and `openElementWithParams` is that you must close each element manually. This can be a bit tricky to keep track of when deeply nesting child elements. The ClayMan class does automatically close any forgotten elements to prevent crashes, but it may not close them where you need them closed. One good way to make things easier is to isolate all child elements in a block and close afterward:

```cpp
ClayMan.openElementWithParams(
    "someID", 
    (Clay_LayoutConfig){.padding = {1, 2, 3, 4}}
);{
    //children here
} ClayMan.closeElement();
```

## Strings

In Clay, you must use the `Clay_String` struct to interact with the Clay library. This requires string literals or manual pointer setup, and can be a challenge. With ClayMan, you can use `std::string` for your textual needs, allowing for dynamics and variable passing:

```cpp
std::string elementID = "Element1";

Clay_TextElementConfig textConfig = { 
    .textColor = COLOR_WHITE, 
    .fontId = 0, 
    .fontSize = 16 
};

ClayMan.element(
    elementID, 
    [&]{
        std::string text1 = "Here is some text.";
        ClayMan.textElement(text1, textConfig);
    }
);

ClayMan.openElementWithParams(
    "Element2", 
);{
    std::string text2 = "Here is some other text.";
    ClayMan.textElement(text2, textConfig);
} ClayMan.closeElement();
```

## How to use ClayMan (with Raylib renderer)
To use this library in your project, simply copy the `clayman.hpp` header, as well as the `include` folder (which has the compatible version of `clay.h` as well as the Raylib renderer) into your working directory. Then, in your project:

```cpp
#include "clayman.hpp"
#include "include/raylib/clay_renderer_raylib.c"

 //Initialize ClayMan object using the constructor
ClayMan Clay(1024, 786, Raylib_MeasureText);

//Function for initializing the Raylib Renderer
void initRaylib(const char* title, const char* fontPath){
    Clay_Raylib_Initialize(ClayMan.width, ClayMan.height, title, FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    Raylib_fonts[0] = (Raylib_Font) { 
        .fontId = 0,
        .font = LoadFontEx(fontPath, 48, 0, 400)
    };

    SetTextureFilter(Raylib_fonts[0].font.texture, TEXTURE_FILTER_BILINEAR);
}

//Function for calling the raylib renderer
void raylibRender(Clay_RenderCommandArray renderCommands){
    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands);
    EndDrawing();
}

//Your layout
void myLayout(){
    //Example full-window parent container
    ClayMan.element(
        "YourElement", 
        (Clay_LayoutConfig){
            .sizing = ClayMan.expandXY(), 
            .padding = ClayMan.padAll(16), 
            .childGap = 16, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
        [&]{
            //Child elements here
        }
    );
}

//Your project's main entry
int main(void) {

    //Initialize Raylib
    initRaylib("Your Project Title", "resources/Roboto-Regular.ttf");
    
    //Raylib render loop
    while (!WindowShouldClose()) { 
       
        //Raylib mouse position and scroll vectors
        Vector2 mousePosition = GetMousePosition(); 
        Vector2 scrollDelta = GetMouseWheelMoveV(); 
        
        //Update clay state (window size, mouse position/scroll, time delta, left button state)
        ClayMan.updateClayState(
            GetScreenWidth(), //raylib window width
            GetScreenHeight(), //raylib window height
            mousePosition.x, 
            mousePosition.y, 
            scrollDelta.x, 
            scrollDelta.y, 
            GetFrameTime(), //raylib frame delta
            IsMouseButtonDown(0) //raylib left button down
        );

        //Pass your layout to the manager to get the render commands
        Clay_RenderCommandArray renderCommands = ClayMan.buildLayout(myLayout); 

        //Pass the render commands to your renderer
        raylibRender(renderCommands);
        
    }
    return 0;
}

```

**NOTE**: To use a different renderer, simply replace all of the Raylib related parts with the appropriate counter-parts (like the SDL2 renderer).

## Events
The Clay library does not handle events. It does, however, have some on-hover and related functionality.
Normally, with Clay, you would use the `Clay_Hovered` and `Clay_OnHover` function with a callback. 
You can still use these functions if you are using the original macros or `openElement`; however, if using the `element` or `openElementWithParams` functions, you will need to take an alternative approach:

```cpp

//Define elementID string, onClick function, and configs here...

//These two lines replace Clay_Hovered
bool overElement = ClayMan.pointerOver(elementID);
bool mousePressed = ClayMan.mousePressed();

//This replaces Clay_OnHover and callback
if(overElement && mousePressed){
    onClick();
} 

//This is the element 
ClayMan.openElementWithParams(
    elementID,
    elementLayoutConfig,
    overElement? 
        hoverRectangleConfig //example conditional config
        : {} //empty config
); 
ClayMan.closeElement();
```

Other events will need to be handled/created with other means, and are not covered in the scope of Clay or ClayMan.

## Element Parameters

In Clay, the `CLAY` macro takes configurations and/or ID macros as parameters. Configurations such as layout, rectangle, scroll, floating, and border configs are passed into macros like `CLAY_LAYOUT` or `CLAY_RECTANGLE` accordingly; then, in-turn, into CLAYMan. In ClayMan, those parameters are either passed directly into `element` or `openElementWithParams` (without needing to call `endConfig`), or they are passed to individual functions like `applyLayoutConfig` or `applyRectangleConfig` between the calls of `openElement` and `endConfig`. The parameters can be in any order, and are optional. For the `element` or `openElementWithParams` functions, you must cast the configurations to each type (i.e. Clay_LayoutConfig), but if applying the configs directly, you do not.

The `CLAY_TEXT` macro usually takes it's own configuration and is an isolated element without children. ClayMan captures this as its own function, `textElement`. See Strings section for an example.

## API

The `ClayMan` class has the following public functions:

### Constructor 
- `ClayMan`
    - Purpose: Initializes the Clay context and creates the ClayMan instance.
    - Params:
        - int **windowWidth**, the initial window width.
        - int **windowHeight**, the initial window height.
        - **Clay_Dimensions (*measureTextFunction)(Clay_String *text, Clay_TextElementConfig *config)**, the renderer's text measuring function.
    - Returns: **ClayMan** instance.

### Primary
The following functions are the primary calls to operate the ClayMan instance, and are required.
- `updateClayState`
    - Purpose: Updates the Clay context with mouse, window, and frame information. These values typically come from the renderer. Call this once per frame, usually before calling buildLayout.
    - Params:
        - int **windowWidth**, the current window width.
        - int **windowHeight**, the current window height.
        - float **mouseX**, the current mouse position x-coordinate.
        - float **mouseY**, the current mouse position y-coordinate.
        - float **scrollDeltaX**, the current mousewheel x-delta.
        - float **scrollDeltaY**, the current mousewheel y-delta.
        - float **frameTime**, the time-length of the last frame.
        - bool **leftButtonDown**, the current state of the left mouse button.
    - Returns: None.
- `buildLayout`
    - Purpose: Wraps the user layout callback between Clay_BeginLayout and Clay_EndLayout and calls them in order. Call this once per frame, before rendering.
    - Params: **void (*userLayoutFunction)()**, the user's callback function for their layout.
    - Returns: **Clay_RenderCommandArray** instance, to be used by renderer.

### Elements
Clay elements can be nested in any heiarchy structure of parents and children, opening each layer as you go in. The last opened element is the current element. Once an element is closed, the parent becomes the current element again, until all elements are closed.
- `element`
    - Purpose: Opens a new element, calls all configuration functions, finalizes configuration, calles child-lambda, and closes the new element automatically.
    - Params:
        Any or all of the following can be passed, in any order. If multiples of any type are passed, they will all be applied in the order in which they were passed. If mutiple ID's are passed, there will be an error from ClayMan. You should pass the lambda last, anything after will be ignored.
        - std::string (or string literal) **id**
        - Clay_LayoutConfig **layoutConfig**
        - Clay_RectangleElementConfig **rectangleConfig**
        - Clay_FloatingElementConfig **floatingConfig**
        - Clay_ScrollElementConfig **scrollConfig**
        - Clay_BorderElementConfig **borderConfig**
        - Clay_ImageElementConfig **imageConfig**
        - std::function<void()> **childElements** (lambda [&]{})
    - Returns: None.
- `openElementWithParams`
    - Purpose: Opens a new element, calls all configuration functions, and finalizes configuration automatically. The element will need to be closed manually using `closeElement` after all children.
    - Params:
        Any or all of the following can be passed, in any order. If multiples of any type are passed, they will all be applied in the order in which they were passed. If mutiple ID's are passed, there will be an error from ClayMan.
        - std::string (or string literal) **id**
        - Clay_LayoutConfig **layoutConfig**
        - Clay_RectangleElementConfig **rectangleConfig**
        - Clay_FloatingElementConfig **floatingConfig**
        - Clay_ScrollElementConfig **scrollConfig**
        - Clay_BorderElementConfig **borderConfig**
        - Clay_ImageElementConfig **imageConfig**
    - Returns: None.
- `openElement`
    - Purpose: Opens a new element. The element will need to be configured manually, such as with `applyLayout`, then call `endConfig`. The element will aslo need to be manually closed using `closeElement` after all children.
    - Params: None.
    - Returns: None.
- `closeElement`
    - Purpose: Closes the current element. Required for every call to openElementWithParams or openElement. DO NOT CALL if `element` or `openElementWithParams` was used to open the element.
    - Params: None.
    - Returns: None.
- `endConfig`
    - Purpose: Finalizes the current element's configuration region. Required for every call to openElement. DO NOT CALL if `openElementWithParams` was used to open the element.
    - Params: None.
    - Returns: None.
- `textElement`
    - Purpose: Creates a text element in-place and is self-contained. DO NOT CALL `closeElement` or `endConfig`. DO NOT manually apply configurations.
    - Params:
        - std::string (or string literal) **text**, the text for the element.
        - Clay_TextElementConfig **textElementConfig**, the text configuration.
    - Returns: None.

### Configuration
- `applyID`
    - Purpose: Applies an ID to the current element.
    - Params: std::string (or string literal) **id**, the element ID.
    - Returns: None.
- `applyLayoutConfig`
    - Purpose: Applies a layout configuration to the current element.
    - Params: Clay_LayoutConfig **layoutConfig**, the layout configuration.
    - Returns: None.
- `applyRectangleConfig`
    - Purpose: Applies a rectangle configuration to the current element.
    - Params: Clay_RectangleElementConfig **rectangleConfig**, the rectangle configuration.
    - Returns: None.
- `applyScrollConfig`
    - Purpose: Applies a scroll configuration to the current element.
    - Params: Clay_ScrollElementConfig **scrollConfig**, the scroll configuration.
    - Returns: None.
- `applyFloatingConfig`
    - Purpose: Applies a floating configuration to the current element.
    - Params: Clay_FloatingElementConfig **floatingConfig**, the floating configuration.
    - Returns: None.
- `applyBorderConfig`
    - Purpose: Applies a border configuration to the current element.
    - Params: Clay_BorderElementConfig **borderConfig**, the border configuration.
    - Returns: None.
- `applyImageConfig`
    - Purpose: Applies an image configuration to the current element.
    - Params: Clay_ImageElementConfig **imageConfig**, the image configuration.
    - Returns: None.

### Sizing
The following are conveniece funcitons for creating Clay_Sizing instances.
- `fixedSize`
    - Purpose: Creates a laout .sizing configuration with fixed X, Y dimensions.
    - Params:
        - uint32_t w, the width of the sizing.
        - uint32_t h, the height of the sizing.
    - Returns: **Clay_Sizing**, a layout sub-configuration.
- `expandXY`
    - Purpose: Creates a laout .sizing configuration for expanding both directions.
    - Params: None.
    - Returns: **Clay_Sizing**, a layout sub-configuration.
- `expandX`
    - Purpose: Creates a laout .sizing configuration for expanding X direction.
    - Params: None.
    - Returns: **Clay_Sizing**, a layout sub-configuration.
- `expandY`
    - Purpose: Creates a laout .sizing configuration for expanding X direction.
    - Params: None.
    - Returns: **Clay_Sizing**, a layout sub-configuration.
- `expandXfixedY`
    - Purpose: Creates a laout .sizing configuration for expanding X direction with a fixed Y dimension.
    - Params: uint32_t **h**, the fixed height of the sizing.
    - Returns: **Clay_Sizing**, a layout sub-configuration.
- `expandYfixedX`
    - Purpose: Creates a laout .sizing configuration for expanding Y direction with a fixed X dimension.
    - Params: uint32_t **w**, the fixed width of the sizing.
    - Returns: **Clay_Sizing**, a layout sub-configuration.

### Padding
The following are convenience functions for creating Clay_Padding instances.
- `padAll`
    - Purpose: Creates a laout .padding configuraion with all sides equal padding.
    - Params: uint16_t **p**, the padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padX`
    - Purpose: Creates a laout .padding configuraion with Left and Right sides equal, Top and Bottom 0.
    - Params: uint16_t **p**, the padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padY`
    - Purpose: Creates a laout .padding configuraion with Top and Bottom sides equal, Left and Right 0.
    - Params: uint16_t **p**, the padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padXY`
    - Purpose: Creates a laout .padding configuraion with Left and Right sides equal and Top and Bottom sides equal.
    - Params:
        - uint16_t **px**, the padding x-dimension.
        - uint16_t **py**, the padding y-dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padLeft`
    - Purpose: Creates a laout .padding configuraion for the Left only, all other sides 0.
    - Params: uint16_t **pl**, the left padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padRight`
    - Purpose: Creates a laout .padding configuraion for the Right only, all other sides 0.
    - Params: uint16_t **pr**, the right padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padTop`
    - Purpose: Creates alaout . padding configuraion for the Top only, all other sides 0.
    - Params: uint16_t **pt**, the top padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.
- `padBottom`
    - Purpose: Creates a laout .padding configuraion for the Bottom only, all other sides 0.
    - Params: uint16_t **pb**, the bottom padding dimension.
    - Returns: **Clay_Padding**, a layout sub-configuration.

### Child Alignment
The following are convenience functions for creating Clay_ChildAlignment instances.
- `centerXY`
    - Purpose: Creates a laout .childAlignment configuration for centering children in both axes.
    - Params: None.
    - Returns: **Clay_ChildAlignment**, a layout sub-configuration

## Utility
The following functions are utility functions for convenience.
- `mousePressed`
    - Purpose: Checks if left mouse button was pressed this frame.
    - Params: None.
    - Returns: **bool**
- `pointerOver`
    - Purpose: Checks if pointer is over an object with given ID.
    - Params: std::string or Clay_String (or string literal) **id**, the element ID.
    - Returns: **bool**
- `getWindowWidth`
    - Purpose: Gets current window width.
    - Params: None.
    - Returns: **int**
- `getWindowHeight`
    - Purpose: Gets current window height.
    - Params: None.
    - Returns: **int**
- `getFramecount`
    - Purpose: Gets current framecount from the manager.
    - Params: None.
    - Returns: **uint32_t**, how many frames have been processed, resets at maximum uint32_t.
- `toClayString`
    - Purpose: Creates an instance of Clay_String from string or string literal.
    - Params: std::string (or string literal) **str**, the text to capture.
    - Returns: **Clay_String** instance.

## Final Notes

Since this library is just a wrapper for Clay; any original clay structs, macros, or functions can still be called. However, there is no need to initialize Clay or create a Clay_Arena of your own. Furthermore, doing so could cause issues or lead to unforseen behavior. Do keep in mind, this library (and Clay), are in developement, and anything could change. The version of clay provided in this repo (and renderer), are guaranteed to be compatible, and will be updated accordingly. Any future updates to this library will be posted in the Change Log section. Happy Coding!

## Change Log

- January 19, 2025
    - ClayMan was published.