#ifndef CLAY_MAN
#define CLAY_MAN

#define CLAY_IMPLEMENTATION
#include "./include/clay.h"

#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <chrono>
#include <optional>
#include <functional>



/*
The original Clay library license can be found here: https://github.com/nicbarker/clay/blob/main/LICENSE.md

In similar fashion, the ClayMan license is as follows:

zlib/libpng license

Copyright (c) 2025 Tim Hoyt

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

//This class initializes Clay.h layout library, manages it's context, and provides functions for convenience
class ClayMan {
    public:
        //This constructor initializes clay automatically. It creates the Clay_Arena and uses a default Clay_ErrorHandler.
        ClayMan(
            const int initialWidth, 
            const int initialHeight, 
            Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, uintptr_t userData)
        ):windowWidth(initialWidth), windowHeight(initialHeight)
        {
            uint64_t clayRequiredMemory = Clay_MinMemorySize();
            Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));

            Clay_Initialize(clayMemory, (Clay_Dimensions) {
            .width = (float)windowWidth,
            .height = (float)windowHeight
            }, (Clay_ErrorHandler) handleErrors);

            Clay_SetMeasureTextFunction(measureTextFunction, {0});
        }

        //This constructor only creates the ClayMan object, you will need to create a Clay_Arena and call Clay_Initialize and Clay_SetMeasureTextFunction before using ClayMan functions
        ClayMan(const int initialWidth, const int initialHeight):windowWidth(initialWidth), windowHeight(initialHeight){

        }

        //Deconstructor
        ~ClayMan(){}

        //Takes mouse, window, and time information and passes it to Clay context
        void updateClayState(
            const int initialWidth, 
            const int initialHeight, 
            const float mouseX, 
            const float mouseY, 
            const float scrollDeltaX, 
            const float scrollDeltaY, 
            const float frameTime, 
            const bool leftButtonDown
        ){
            windowWidth = initialWidth;
            windowHeight = initialHeight;
            Clay_SetLayoutDimensions((Clay_Dimensions) {
                    .width = (float)windowWidth,
                    .height = (float)windowHeight
            });

            Clay_SetPointerState(
                (Clay_Vector2) { mouseX, mouseY },
                leftButtonDown
            );

            Clay_UpdateScrollContainers(
                true,
                (Clay_Vector2) { scrollDeltaX, scrollDeltaY },
                frameTime
            );
        }

        //Call this only once per frame, wraps user callback between Clay_BeginLayout() and CLay_EndLayout(). If you do not want to use this, you will need to call beginLayout() and endLayout() instead of Clay_BeginLayout() and Clay_EndLayout(), or else the string arena will not work correctly.
        Clay_RenderCommandArray buildLayout(void (*userLayoutFunction)()){
            auto start = std::chrono::high_resolution_clock::now();
            countFrames();
            beginLayout();

            userLayoutFunction();

            while(openElementCount > 0){
                if(!warnedAboutClose){
                    printf("WARN: An element was not closed.");
                    warnedAboutClose = true;
                }
                closeElement();
            }

            measureTime(start);
            return endLayout();
        }

        //Takes ID string or literal, as well as layout, rectangle, scroll, floating, border, or image configs, and also a lambda function for child elements. Opens and configures an element, calls child lambda, then closes the element; all automatically.
        template<typename... Args>
        void element(Args... args) {
            openElement();

            bool childcallbackfound = false;
            (checkForChildCallback(childcallbackfound, args), ...);

            if(!childcallbackfound){
                endConfig();
            }
            closeElement();           
        }

        //Takes ID string or literal, as well as layout, rectangle, scroll, floating, border, or image configs. Opens and configures an element automatically. Must be closed manually with closeElement().
        template<typename... Args>
        void openElementWithParams(Args... args){
            openElement();
            (handleElementParam(args), ...);
            endConfig();
        }

        //Manually opens an element. Call endConfigs() after manually apply configurations, then manually close with closeElement()
        void openElement(){
            Clay__OpenElement();
            openElementCount++;
        }

        //Manually closes an element. Call after the children of an element (if any) that was opened manually with openElement() or openElementWithParams()
        void closeElement(){
            Clay__CloseElement();
            if(openElementCount <=0){
                if(!warnedAboutUnderflow){
                    printf("Whoops! All elements are already closed!");
                    warnedAboutUnderflow = true;
                }
            }else{
                openElementCount--;
            }
        }

        //Manually end configuration region of an element that was opened with openElement().
        void endConfig(){
            Clay__ElementPostConfiguration();
        }

        //A self-contained text element, with no children.
        void textElement(const std::string& text, const Clay_TextElementConfig textElementConfig){
            Clay__OpenTextElement(
                toClayString(text), 
                Clay__StoreTextElementConfig(
                (Clay__Clay_TextElementConfigWrapper(textElementConfig)).wrapped
                )
            );
        }

        //A self-contained text element, with no children. Takes a string literal.
        template<size_t N>
        void textElement(const char(&text)[N], const Clay_TextElementConfig textElementConfig){
            Clay__OpenTextElement(
                toClayString(text), 
                Clay__StoreTextElementConfig(
                (Clay__Clay_TextElementConfigWrapper(textElementConfig)).wrapped
                )
            );
        }

        //A self-contained text element, with no children.
        template<size_t N>
        void textElement(const Clay_String& text, const Clay_TextElementConfig textElementConfig){
            Clay__OpenTextElement(
                text, 
                Clay__StoreTextElementConfig(
                (Clay__Clay_TextElementConfigWrapper(textElementConfig)).wrapped
                )
            );
        }
         
        //Call to manually apply an ID after calling openElement(). Takes a string literal.
        template<size_t N>
        void applyID(const char(&id)[N]) {
            Clay__AttachId(Clay__HashString(toClayString(id), 0, 0));
        }

        //Call to manually apply an ID after calling openElement().
        void applyID(const std::string& id) {
            Clay__AttachId(Clay__HashString(toClayString(id), 0, 0));
        }

        //Call to manually apply an ID after calling openElement().
        void applyID(const Clay_String& id) {
            Clay__AttachId(Clay__HashString(id, 0, 0));
        }
        
        //Call to manually apply layout config after calling openElement()
        void applyLayoutConfig(const Clay_LayoutConfig layoutConfig){
            Clay__AttachLayoutConfig(Clay__StoreLayoutConfig((Clay__Clay_LayoutConfigWrapper(layoutConfig)).wrapped));
        }

        //Call to manually apply rectangle config after calling openElement()
        void applyRectangleConfig(const Clay_RectangleElementConfig rectangleConfig){
            Clay__AttachElementConfig(
                Clay_ElementConfigUnion { 
                    .rectangleElementConfig = Clay__StoreRectangleElementConfig(
                        (Clay__Clay_RectangleElementConfigWrapper (rectangleConfig)).wrapped
                    ) 
                }, 
                CLAY__ELEMENT_CONFIG_TYPE_RECTANGLE
            );
        }

        //Call to manually apply scroll config after calling openElement()
        void applyScrollConfig(const Clay_ScrollElementConfig scrollConfig){
                Clay__AttachElementConfig(Clay_ElementConfigUnion { .scrollElementConfig = Clay__StoreScrollElementConfig((Clay__Clay_ScrollElementConfigWrapper { scrollConfig }).wrapped) }, CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER);
        }

        //Call to manually apply floating config after calling openElement()
        void applyFloatingConfig(const Clay_FloatingElementConfig floatingConfig){
            Clay__AttachElementConfig(Clay_ElementConfigUnion { .floatingElementConfig = Clay__StoreFloatingElementConfig((Clay__Clay_FloatingElementConfigWrapper { floatingConfig }).wrapped) }, CLAY__ELEMENT_CONFIG_TYPE_FLOATING_CONTAINER);
        }

        //Call to manually apply border config after calling openElement()
        void applyBorderConfig(const Clay_BorderElementConfig borderConfig){
            Clay__AttachElementConfig(
                Clay_ElementConfigUnion { 
                    .borderElementConfig = Clay__StoreBorderElementConfig(
                        (Clay__Clay_BorderElementConfigWrapper (borderConfig)).wrapped
                    )
                }, 
            CLAY__ELEMENT_CONFIG_TYPE_BORDER_CONTAINER
            );
        }

        //Call to manually apply image config after calling openElement()
        void applyImageConfig(Clay_ImageElementConfig imageConfig){
            Clay__AttachElementConfig(
                Clay_ElementConfigUnion { 
                    .imageElementConfig = Clay__StoreImageElementConfig(
                        (Clay__Clay_ImageElementConfigWrapper {imageConfig}).wrapped
                    ) 
                }, 
                CLAY__ELEMENT_CONFIG_TYPE_IMAGE
            );
        }

        //Convenience funtion for .sizing layout parameter
        Clay_Sizing fixedSize(const uint32_t w, const uint32_t h) {
            return{
                .width = (Clay_SizingAxis { .size = { .minMax = { (float)w, (float)w } }, .type = CLAY__SIZING_TYPE_FIXED }),
                .height = (Clay_SizingAxis { .size = { .minMax = { (float)h, (float)h } }, .type = CLAY__SIZING_TYPE_FIXED }) 
            };
        }

        //Convenience funtion for .sizing layout parameter
        Clay_Sizing expandXY(){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
                .height = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW })
            };
        }

        //Convenience funtion for .sizing layout parameter
        Clay_Sizing expandX(){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
            };
        }

        //Convenience funtion for .sizing layout parameter
        Clay_Sizing expandY(){
            return {
                .height = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
            };
        }

        //Convenience funtion for .sizing layout parameter
        Clay_Sizing expandXfixedY(const uint32_t h){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
                .height = (Clay_SizingAxis { .size = { .minMax = { (float)h, (float)h } }, .type = CLAY__SIZING_TYPE_FIXED }) 
            };
        }

        //Convenience funtion for .sizing layout parameter
        Clay_Sizing expandYfixedX(const uint32_t w){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { (float)w, (float)w } }, .type = CLAY__SIZING_TYPE_FIXED }),
                .height = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW })
            };
        }

        //Convenience funtion for .padding layout parameter
        Clay_Padding padAll(const uint16_t p){
            return {p, p, p, p};
        }
       
        //Convenience funtion for .padding layout parameter
        Clay_Padding padX(const uint16_t p){
            return {p, p, 0, 0};
        }

        //Convenience funtion for .padding layout parameter
        Clay_Padding padY(const uint16_t p){
            return {0, 0, p, p};
        }
        
        //Convenience funtion for .padding layout parameter
        Clay_Padding padXY(const uint16_t px, const uint16_t py){
            return {px, px, py, py};
        }

        //Convenience funtion for .padding layout parameter
        Clay_Padding padLeft(const uint16_t pl){
            return {pl, 0, 0, 0};
        }

        //Convenience funtion for .padding layout parameter
        Clay_Padding padRight(const uint16_t pr){
            return {0, pr, 0, 0};
        }

        //Convenience funtion for .padding layout parameter
        Clay_Padding padTop(const uint16_t pt){
            return {0, 0, pt, 0};
        }

        //Convenience funtion for .padding layout parameter
        Clay_Padding padBottom(const uint16_t pb){
            return {0, 0, 0, pb};
        }

        //Convenience funtion for .childAlignment layout parameter
        Clay_ChildAlignment centerXY(){
            return {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER};
        }

        //Gets clay internal left-mouse-button state this frame
        bool mousePressed(){
            return Clay_GetCurrentContext()->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME;
        }

        bool pointerOver(const Clay_String id){
            return Clay_PointerOver(Clay_GetElementId(id));
        }

        bool pointerOver(const std::string& id){
            return Clay_PointerOver(Clay_GetElementId(toClayString(id)));
        }

        //Takes string literal
        template<size_t N>
        bool pointerOver(const char(&id)[N]){
            return Clay_PointerOver(Clay_GetElementId(toClayString(id)));
        }

        //Caches std::string into a string arena, then creates and returns a Clay_String
        Clay_String toClayString(const std::string& str){
            return (Clay_String){ .length = (int32_t)str.size(), .chars = insertStringIntoArena(str)};
        }

        //Caches string literal into a string arena, then creates and returns a Clay_String
        template<size_t N>
        Clay_String toClayString(const char(&str)[N]){
            return (Clay_String){ .length = static_cast<int32_t>((N - 1)), .chars = insertStringIntoArena(str)};
        }

        //Gets current window width
        int getWindowWidth(){
            return windowWidth;
        }

        //Gets current window height
        int getWindowHeight(){
            return windowHeight;
        }

        //Gets current frame count, resets at max int32_t
        uint32_t getFramecount(){
            return framecount;
        }

    private:
        //One-shot for console warning
        bool warnedAboutClose = false;

        //One-shot for console warning
        bool warnedAboutUnderflow = false;

        int windowWidth;
        int windowHeight;
        uint32_t framecount = 0;
        static constexpr size_t maxStringArenaSize = 100000; 

        //Reusable char arena to cache strings for Clay_String conversions
        char stringArena[maxStringArenaSize];

        //Tracks current position in string arena
        size_t nextStringArenaIndex = 0;

        //Tracks the heiarchy depth of the current element in the layout
        uint32_t openElementCount = 0;

        //Be sure to call this instead of Clay_BeginLayout directly to reset string arena index
        void beginLayout(){
            resetStringArenaIndex();
            Clay_BeginLayout();
        }

        //Same as Clay_EndLayout
        Clay_RenderCommandArray endLayout(){
            return Clay_EndLayout();
        }
        
        //Catches and executes child lambda from element()
        void handleElementParam(std::function<void()> childElements = nullptr){
            if (childElements) {
                childElements();
            }
        }

        //Catches and applies std::string ID from element() and openElementWithParams()
        void handleElementParam(const std::string param){
            Clay__AttachId(Clay__HashString(toClayString(param), 0, 0));
        }

        //Catches and applies string literal ID from element() and openElementWithParams()
        template <size_t N>
        void handleElementParam(const char (&param)[N]) {
                Clay__AttachId(Clay__HashString(toClayString(param), 0, 0));
        } 

        //Catches and applies Clay_String ID from element() and openElementWithParams()
        void handleElementParam(const Clay_String param) {
                Clay__AttachId(Clay__HashString(param, 0, 0));
        } 

        //Catches and applies layout config from element() and openElementWithParams()
        void handleElementParam(const Clay_LayoutConfig& config) {
            applyLayoutConfig(config);
        }

        //Catches and applies rectangle config from element() and openElementWithParams()
        void handleElementParam(const Clay_RectangleElementConfig& config) {
            applyRectangleConfig(config);
        }

        //Catches and applies floating config from element() and openElementWithParams()
        void handleElementParam(const Clay_FloatingElementConfig& config) {
            applyFloatingConfig(config);
        }

        //Catches and applies scroll config from element() and openElementWithParams()
        void handleElementParam(const Clay_ScrollElementConfig& config) {
            applyScrollConfig(config);
        }

        //Catches and applies border config from element() and openElementWithParams()
        void handleElementParam(const Clay_BorderElementConfig& config) {
            applyBorderConfig(config);
        }

        //Catches and applies image config from element() and openElementWithParams()
        void handleElementParam(const Clay_ImageElementConfig& config) {
            applyImageConfig(config);
        }
        
        //Resets index tracker for string arena to 0
        void resetStringArenaIndex() {
            nextStringArenaIndex = 0;
        }
        
        //Caches strings into string arena
        const char* insertStringIntoArena(const std::string& str) {
            size_t strSize = str.size();

            if (nextStringArenaIndex + strSize + 1 > maxStringArenaSize) {
                throw std::overflow_error("StringArena: Not enough space to insert the string.");
            }

            char* startPtr = &stringArena[nextStringArenaIndex];

            for (char ch : str) {
                stringArena[nextStringArenaIndex++] = ch;
            }
            stringArena[nextStringArenaIndex++] = ' ';
            return startPtr;
        }

        /////////for performance measuring/////////
        long long frametime = 0;
        long long maxframetime = 0;

        void countFrames(){
            if(framecount < std::numeric_limits<uint32_t>::max()){
                framecount++;
            }else{
                framecount = 0;
            }
        }

        void measureTime(auto start){
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = end - start;
            long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
            frametime = microseconds;
            
            if(framecount%180 == 0){
                std::cout << "Layout completed in " << frametime << " microseconds." << std::endl;
            }
            
            if(microseconds > maxframetime) {
                maxframetime = microseconds; 
                std::cout << "New maximum layout time of " << maxframetime << " microseconds." << std::endl;
            }
        }

        //Clay_ErrorHandler
        static void handleErrors(Clay_ErrorData errorData) {
            printf("%s", errorData.errorText.chars);
        }

        //Checks params from element() for children callback lambda
        template <typename F, typename... Args>
        void checkForChildCallback(bool& childcallbackfound, F&& f, Args&&... args){
            if(!childcallbackfound){
                bool isfunc = std::invocable<F, Args...>;
                if(isfunc){
                    childcallbackfound = true;
                    endConfig();
                    handleElementParam(f);
                }else{
                    handleElementParam(f);
                }
            }
        }
};

#endif //end of file