#include "../../clayman.hpp"
#include "../../include/raylib/clay_renderer_raylib.c"

//Initialize ClayMan using alternate Constructor
ClayMan clayMan(1024, 786);

//Define Clay_ErrorHandler handler
void handleErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

//Application entry
int main(void) {
    
    //Manually set up Clay_Arena
    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));

    //Manually initialize Clay
    Clay_Initialize(clayMemory, (Clay_Dimensions) {
    .width = (float)clayMan.getWindowWidth(),
    .height = (float)clayMan.getWindowHeight()
    }, (Clay_ErrorHandler) handleErrors);

    //Manually set text measure function
    Clay_SetMeasureTextFunction(Raylib_MeasureText, {0});

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

        //You can still use the ClayMan.buildLayout(yourCallback) function here, or you can manually call beginLayout() and endLayout()
        //endLayout() will still automatically close elements to prevent crashes if you use openElementWithParams() or openElement() methods.
        clayMan.beginLayout();

        //Build your immediate-mode layout here (or create a function for it)
        Clay_TextElementConfig textConfig = {
            .textColor = {255, 255, 255, 255},
            .fontId = 0,
            .fontSize = 20
        };

        clayMan.element(
            "Container",
            (Clay_LayoutConfig){ 
                .sizing = clayMan.expandXY(),
                .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
            },
            (Clay_RectangleElementConfig){ 
                Clay_Color{120, 120, 120, 255}
            },
            [&]{
                clayMan.textElement("This is an advanced use example!", textConfig);
            }
        );

        //Manually call Clay_EndLayout() to get render commands and pass to renderer
        Clay_RenderCommandArray renderCommands = clayMan.endLayout();
        Clay_Raylib_Render(renderCommands); 

        EndDrawing(); //Rayblib native command
    }
    return 0;
}