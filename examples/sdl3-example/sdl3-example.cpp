#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <cstddef>
#include <vector>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include "../../clayman.hpp"
#include "../../include/SDL3/clay_renderer_SDL3.c"

typedef struct app_state {
    ClayMan clayMan;
    Clay_SDL3RendererData rendererData;
    Clay_RenderCommandArray renderCommands;
    Clay_TextElementConfig textConfig;
    Clay_TextElementConfig buttonTextConfig;
    float mouseX = 0;
    float mouseY = 0;
    float scrollX = 0;
    float scrollY = 0;
    int width = -1;
    int height = -1;
    SDL_Surface *sample_image;
    SDL_Window *window;
    bool mouseDown = false;
    size_t fontCount = 0;
    Uint64 NOW = 1;
    Uint64 LAST = 0;
    double deltaTime = 1;
} AppState;

std::vector<std::string> theStrings;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
    SDL_SetHint(SDL_HINT_LOGGING, "0");
    // SDL_SetHint(SDL_HINT_LOGGING, "1");
    // SDL_SetHint(SDL_HINT_LOGGING, "3");

    (void) argc;
    (void) argv;

    if (!TTF_Init()) {
        return SDL_APP_FAILURE;
    }

    AppState *state = (AppState*)SDL_calloc(1, sizeof(AppState));
    if (!state) {
        return SDL_APP_FAILURE;
    }
    *appstate = state;

    if (!SDL_CreateWindowAndRenderer("ClayMan SDL3-EXAMPLE", 800, 600, 0, &state->window, &state->rendererData.renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowResizable(state->window, true);

    state->rendererData.textEngine = TTF_CreateRendererTextEngine(state->rendererData.renderer);
    if (!state->rendererData.textEngine) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create text engine from renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->rendererData.fonts = (TTF_Font**)SDL_calloc(1, sizeof(TTF_Font *));
    if (!state->rendererData.fonts) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate memory for the font array: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont("resources/Roboto-Regular.ttf", 24);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->rendererData.fonts[0] = font;
    state->fontCount = 1;

    state->sample_image = IMG_Load("resources/sample.png");

    int width, height;
    SDL_GetWindowSize(state->window, &width, &height);

    state->clayMan = ClayMan(width, height, SDL_MeasureText, state->rendererData.fonts);
    state->width = width;
    state->height = height;
    state->clayMan.updateClayState(state->width, state->height, state->mouseX, state->mouseY, state->scrollX, state->scrollY, 0.01f, state->mouseDown); 
    
    state->textConfig = {
        .textColor = {255,255,255,123},
        .fontId = 0,
        .fontSize = 16
    };

    state->buttonTextConfig = {
        .textColor = {0,0,0,255},
        .fontId = 0,
        .fontSize = 16
    };

    state->NOW = SDL_GetPerformanceCounter();
    state->LAST = 0;
    state->deltaTime = 0;

    theStrings.push_back("This is a string!");

    *appstate = state;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
    AppState *state = (AppState*) appstate;
    SDL_AppResult ret_val = SDL_APP_CONTINUE;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            ret_val = SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            state->width = event->window.data1;
            state->height = event->window.data2;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            state->mouseX = event->motion.x;
            state->mouseY = event->motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(event->button.button == SDL_BUTTON_LEFT){
                state->mouseDown = true;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if(event->button.button == SDL_BUTTON_LEFT){
                state->mouseDown = false;
            }
        case SDL_EVENT_MOUSE_WHEEL:
            state->scrollX = event->wheel.x;
            state->scrollY = event->wheel.y;
            break;
        default:
            break;
    };

    return ret_val;
}

void myActions(void *appstate){
    AppState *state = (AppState*) appstate;
    ClayMan& clayMan = state->clayMan;

    if(clayMan.mousePressed()){
        if(clayMan.pointerOver("Button1")){
            theStrings[0] = "The Button Has Been Pressed!";

        }else{
            
        }
    }else{
        if(clayMan.pointerOver("Button1")){
            state->buttonTextConfig.textColor = {255,255,255,255};
        }else{
            state->buttonTextConfig.textColor = {0,0,0,255};
        }
    }
}

void myLayout(void *appstate){
    AppState *state = (AppState*) appstate;
    ClayMan& clayMan = state->clayMan;

    clayMan.element({
        .id = clayMan.hashID("OuterContainer"),
        .layout = {
            .sizing = clayMan.expandXY(),
            .childGap = 10,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = {10,10,10,255}
    },[&](){
        clayMan.textElement(theStrings[0], state->textConfig);
        clayMan.element({
            .id = clayMan.hashID("ImageContainer"),
            .layout = {
                .sizing = clayMan.fixedSize(23, 42)
            }
        }, [&](){
            clayMan.element({
                .id = clayMan.hashID("TheImage"),
                .layout = {
                    .sizing = clayMan.expandXY()
                },
                .image = {
                    .imageData = state->sample_image,
                    .sourceDimensions = {23,42}
                }
            });
        });
        clayMan.element({
            .id = clayMan.hashID("Button1"),
            .layout = {
                .padding = clayMan.padAll(6)
            },
            .backgroundColor = {150, 150, 150, 150}
        },[&](){
            clayMan.textElement("BUTTON", state->buttonTextConfig);
        });
    });
}


SDL_AppResult SDL_AppIterate(void *appstate){
    AppState *state = (AppState*) appstate;
    
    state->LAST = state->NOW;
    state->NOW = SDL_GetPerformanceCounter();
    state->deltaTime = (double)((state->NOW - state->LAST)*1000 / (double)SDL_GetPerformanceFrequency() );

    state->clayMan.updateClayState(state->width, state->height, state->mouseX, state->mouseY, state->scrollX, state->scrollY, state->deltaTime, state->mouseDown); 

    myActions(state);

    state->clayMan.beginLayout();
    myLayout(state);
    state->renderCommands = state->clayMan.endLayout();

    SDL_SetRenderDrawColor(state->rendererData.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->rendererData.renderer);
    SDL_Clay_RenderClayCommands(&state->rendererData, &state->renderCommands);
    SDL_RenderPresent(state->rendererData.renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
    (void) result; if (result != SDL_APP_SUCCESS) {SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Application failed to run");}

    AppState *state = (AppState*) appstate;

    if (state) {
        if (state->rendererData.renderer) SDL_DestroyRenderer(state->rendererData.renderer);
        if (state->window) SDL_DestroyWindow(state->window);
        if (state->rendererData.fonts) {
            for (size_t i = 0; i < state->fontCount; i++) {TTF_CloseFont(state->rendererData.fonts[i]);}
            SDL_free(state->rendererData.fonts);
        }
        if (state->rendererData.textEngine) TTF_DestroyRendererTextEngine(state->rendererData.textEngine);
        SDL_free(state);
    }

    TTF_Quit();
}