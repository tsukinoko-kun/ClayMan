#include "../../clayman.hpp"
#include "../../include/raylib/clay_renderer_raylib.c"

//Define Clay_ErrorHandler handler
void handleErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

//Application entry
int main(void) {
    //Initialize ClayMan using alternate Constructor
    ClayMan clayMan(1024, 786);

    //Manually set up Clay_Arena
    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));

    //Manually initialize Clay
    Clay_Initialize(clayMemory, (Clay_Dimensions) {
    .width = (float)clayMan.getWindowWidth(),
    .height = (float)clayMan.getWindowHeight()
    }, (Clay_ErrorHandler) handleErrors);


    //Initialize Raylib
    Clay_Raylib_Initialize(
        clayMan.getWindowWidth(), 
        clayMan.getWindowHeight(), 
        "ClayMan C++ Quick Example", 
        FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT
    );

    //Create fonts after initializing raylib
    Font fonts[] = {LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400)};

    //Set fonts texture filters
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);

    //Manually set text measure function
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts); //for raylib, userData is Fonts array

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

        

        clayMan.beginLayout(); //Prep for layout

        //Build your immediate-mode layout here (or create a function for it)
        Clay_TextElementConfig textConfig = {
            .textColor = {255, 255, 255, 255},
            .fontId = 0,
            .fontSize = 20
        };

        clayMan.element(
            {
                .id = clayMan.hashID("Container"),
                .layout = {
                    .sizing = clayMan.expandXY(),
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
                },
                .backgroundColor = {120,120,120,255}
            },
            [&](){
                clayMan.textElement("This is an advanced use example!", textConfig);
            }
        );

        //Manually call endLayout() to get render commands and pass to renderer
        Clay_RenderCommandArray renderCommands = clayMan.endLayout();

        BeginDrawing(); //Rayblib native command
        ClearBackground(BLACK); //Rayblib native command
        Clay_Raylib_Render(renderCommands, fonts); 
        EndDrawing(); //Rayblib native command
    }
    return 0;
}