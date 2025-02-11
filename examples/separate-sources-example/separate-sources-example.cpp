#include "../../clayman.hpp"
#include "../../include/raylib/clay_renderer_raylib.c"
#include "layout.hpp"

//Application entry
int main(void) {
    //Reserve memory for raylib fonts
    Font fonts[1];

    //Initialize ClayMan instance
    ClayMan clayMan(1024, 786, Raylib_MeasureText, fonts);

    //Initialize Raylib
    Clay_Raylib_Initialize(
        clayMan.getWindowWidth(), 
        clayMan.getWindowHeight(), 
        "ClayMan C++ Separate Sources Example", 
        FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT
    );

    //Initialize the fonts (after initializing raylib)
    fonts[0] = LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400);

     //Set Raylib's texture filter for the fonts
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);

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


        clayMan.beginLayout(); //Prep for building layout
        myLayout(clayMan); //Your layout from a separate source file

        //Call endLayout() to get Clay_RenderCommandArray
        //Which, in-turn, get's passed to the renderer, along with the fonts, to be drawn
        Clay_Raylib_Render(clayMan.endLayout(), fonts); 
        
        EndDrawing(); //Rayblib native command
    }
    return 0;
}