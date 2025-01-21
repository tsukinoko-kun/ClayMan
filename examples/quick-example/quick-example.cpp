#include "../../clayman.hpp"
#include "../../include/raylib/clay_renderer_raylib.c"

//Initialize ClayMan
ClayMan clayMan(1024, 786, Raylib_MeasureText);

//User layout callback function, do not call Clay_BeginLayout() or Clay_EndLayout()
void myLayout(){
    static std::string theText = "Click the button to change the text.";

    //A reusable configuration
    Clay_TextElementConfig textConfig = {
        .textColor = {255, 255, 255, 255},
        .fontId = 0,
        .fontSize = 20
    };

    //On-Click for Button element
    if(clayMan.pointerOver("Button") & clayMan.mousePressed()){
        theText = "Button has been clicked!";
    }

    //Here we are using openElementWithParams. We do not need to call endConfigs(), but we DO need to call closeElement() after children block
    clayMan.openElementWithParams(
        "OuterContainer", 
        (Clay_LayoutConfig){ 
            .sizing = clayMan.expandXY(), 
            .padding = clayMan.padAll(16), 
            .childGap = 16, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        (Clay_RectangleElementConfig){ .color = {50, 50, 50, 255} }
        //we do not call endConfigs()
    );{ //Children of OuterContain are in this block. The block is not necessary, but helps with structure.
        //Text Elements are self-contained
        clayMan.textElement("The outer container was made with openElementWithParams function and needs closed manually", textConfig);

        //Here we are using the lambda method, we do not need to call endConfigs or closeElement
        clayMan.element(
            "Button",
            (Clay_LayoutConfig){ 
                .sizing = clayMan.fixedSize(100, 40),
                .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
            },
            (Clay_RectangleElementConfig){ 
                .color = clayMan.pointerOver("Button")? //ternary in-line config property
                    Clay_Color{120, 120, 120, 255}
                    : Clay_Color{100, 100, 100, 255} 
            },
            [&]{ //children of "Buton" are in this lambda, the lambda is the last argument of the parent
                clayMan.textElement("Click Me", textConfig);
            }
        ); //do not call closeElement for "Button"

        clayMan.textElement(theText, textConfig);
        
        //Here is a linear method element
        clayMan.openElement(); //we manually open the element
        clayMan.applyID("ManualElement"); //we manually apply id and configs
        clayMan.applyLayoutConfig(
            { 
                .padding = clayMan.padXY(26, 16),
                .childGap = 16,
                .childAlignment = clayMan.centerXY()
            }
        );
        clayMan.applyRectangleConfig(
            {.color = {0, 0, 0, 255}}
        );
        clayMan.endConfig(); //we manually close the configuration section
        {//Children of "ManualElement" are in this block, the block is not necessary, but helps with structure
            clayMan.textElement("This parent element was made manually.", textConfig);

            //Another lambda element
            clayMan.element(
                "Lambdaelement",
                Clay_LayoutConfig{.padding = clayMan.padAll(16)},
                (Clay_RectangleElementConfig){.color = {120, 120, 120, 255}},
                [&]{
                    clayMan.textElement("This element was made with the lambda method using element function.", textConfig);
                }
            );

            //A classic macro element
            CLAY(
                CLAY_ID("ClayElement"),
                CLAY_LAYOUT({ .padding = CLAY_PADDING_ALL(16), .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}}),
                Clay_Hovered()? CLAY_RECTANGLE({
                    .color = {200, 200, 200, 255},
                    .cornerRadius = 10
                })
                : CLAY_RECTANGLE({
                    .color = {100, 100, 100, 255},
                    .cornerRadius = 10
                })
            ){
                CLAY_TEXT(CLAY_STRING("This element was made with the standard Clay macros inside a lambda element"), CLAY_TEXT_CONFIG(textConfig));
            }
        }
        clayMan.closeElement(); //we manually close the "ManualElement"

        //A reusable border
        Clay_Border border{.width = 5, .color = {123, 123, 0, 123}};

        clayMan.element(
            "AnotherLambdaElement",
            Clay_LayoutConfig{.sizing = clayMan.expandX(), .padding = clayMan.padAll(16), .childAlignment = clayMan.centerXY()},
            Clay_RectangleElementConfig{.color = {0, 0, 0, 125}},
            Clay_BorderElementConfig{.left = border, .right = border, .top = border, .bottom = border},
            [&]{
                clayMan.element(
                    Clay_LayoutConfig{ .padding = clayMan.padAll(16)},
                    Clay_RectangleElementConfig{.color = {255, 255, 255, 123}},
                    [&]{
                        clayMan.textElement("This element was made using the lambda method", textConfig);
                    }
                );
            }
        );
        
    } clayMan.closeElement(); //We close the outer container manually
}

//Application entry
int main(void) {
    //Initialize Raylib
    Clay_Raylib_Initialize(
        clayMan.getWindowWidth(), 
        clayMan.getWindowHeight(), 
        "ClayMan C++ Quick Example", 
        FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT
    );

    //Load a font with Raylib
    Raylib_fonts[0] = (Raylib_Font) { 
        .fontId = 0,
        .font = LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400)
    };

    //Set Raylib's texture filter for the font
    SetTextureFilter(Raylib_fonts[0].font.texture, TEXTURE_FILTER_BILINEAR);

    //Raylib loop
    while (!WindowShouldClose()) {

        //Get mouse info from Raylib
        Vector2 mousePosition = GetMousePosition();
        Vector2 scrollDelta = GetMouseWheelMoveV();

        //Update clay state
        clayMan.updateClayState(
            GetScreenWidth(), //Window dims from Raylib
            GetScreenHeight(), 
            mousePosition.x, 
            mousePosition.y, 
            scrollDelta.x, 
            scrollDelta.y, 
            GetFrameTime(), //Frame delta from Raylib
            IsMouseButtonDown(0) //Left button state from Raylib
        );

        BeginDrawing(); //Rayblib native command
        ClearBackground(BLACK); //Rayblib native command

        //Pass the user callback function into clayman to get Clay_RenderCommandArray
        //Which, in-turn, get's passed to the renderer to be drawn
        Clay_Raylib_Render(clayMan.buildLayout(myLayout)); 
        
        EndDrawing(); //Rayblib native command
    }
    return 0;
}