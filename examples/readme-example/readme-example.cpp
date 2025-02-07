#include "../../clayman.hpp"
#include "../../include/raylib/clay_renderer_raylib.c"

//Your project's main entry
int main(void) {

    //Reserve memory for raylib fonts
    Font fonts[1];

    //Initialize ClayMan object using the constructor
    ClayMan clayMan(1024, 786, Raylib_MeasureText, fonts);

    //Initialize Raylib
    Clay_Raylib_Initialize(
        clayMan.getWindowWidth(), 
        clayMan.getWindowHeight(), 
        "Your Project Title", 
        FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT
    );

    //Load fonts after initializing raylib
    fonts[0] = LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400);
    
    //Set fonts texture filters
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);

    //Raylib render loop
    while (!WindowShouldClose()) { 
       
        //Raylib mouse position and scroll vectors
        Vector2 mousePosition = GetMousePosition(); 
        Vector2 scrollDelta = GetMouseWheelMoveV(); 
        
        //Update clay state (window size, mouse position/scroll, time delta, left button state)
        clayMan.updateClayState(
            GetScreenWidth(), //Raylib window width
            GetScreenHeight(), //Raylib window height
            mousePosition.x, 
            mousePosition.y, 
            scrollDelta.x, 
            scrollDelta.y, 
            GetFrameTime(), //Raylib frame delta
            IsMouseButtonDown(0) //Raylib left button down
        );

        //Prep for layout
        clayMan.beginLayout();
        
        //Example full-window parent container
        clayMan.element(
            { //Configure element
                .id = clayMan.hashID("YourElementID"),
                .layout = {
                    .sizing = clayMan.expandXY(), 
                    .padding = clayMan.padAll(16), 
                    .childGap = 16, 
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                },
                .backgroundColor = {50,50,50,255}
            },
            [&]{ //Child elements in here
                clayMan.textElement(
                    "Here is some text",
                    { //Configure text
                        .textColor = {255,255,255,255},
                        .fontId = 0, 
                        .fontSize = 16
                    }
                );
            }
        );
        
        //Pass your layout to the manager to get the render commands
        Clay_RenderCommandArray renderCommands = clayMan.endLayout(); 

        BeginDrawing(); //Start Raylib's draw block
        ClearBackground(BLACK); //Raylib's clear function
        Clay_Raylib_Render(renderCommands, fonts); //Render Clay Layout
        EndDrawing(); //End Raylib's draw block
    }
    return 0;
}