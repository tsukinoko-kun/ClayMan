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

ClayMan Repo: https://github.com/TimothyHoytBSME/ClayMan
Clay Repo: https://github.com/nicbarker/clay

The original Clay library and license can be found here: https://github.com/nicbarker/clay/blob/main/LICENSE.md

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
        //This constructor initializes clay automatically. It creates the Clay_Arena and uses a default Clay_ErrorHandler. You will need to pass in your renderer's measure text function, as well as the associated userdata (i.e. fonts array for raylib).
        ClayMan(
            const uint32_t initialWidth, 
            const uint32_t initialHeight, 
            Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void* userData),
            void* measureTextUserData
        ):windowWidth(initialWidth), windowHeight(initialHeight)
        {
            if(windowWidth == 0){windowWidth = 1;}
            if(windowHeight == 0){windowHeight = 1;}
            uint64_t clayRequiredMemory = Clay_MinMemorySize();
            Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));

            Clay_Initialize(clayMemory, (Clay_Dimensions) {
            .width = (float)windowWidth,
            .height = (float)windowHeight
            }, (Clay_ErrorHandler) handleErrors);

            Clay_SetMeasureTextFunction(measureTextFunction, measureTextUserData);
        }

        //This constructor only creates the ClayMan object, you will need to create a Clay_Arena and call Clay_Initialize and Clay_SetMeasureTextFunction before using ClayMan functions
        ClayMan(const uint32_t initialWidth, const uint32_t initialHeight):windowWidth(initialWidth), windowHeight(initialHeight){
            if(windowWidth == 0){windowWidth = 1;}
            if(windowHeight == 0){windowHeight = 1;}
        }

        //Deconstructor
        ~ClayMan(){}

        //Takes mouse, window, and time information and passes it to Clay context
        void updateClayState(
            const uint32_t initialWidth, 
            const uint32_t initialHeight, 
            const float mouseX, 
            const float mouseY, 
            const float scrollDeltaX, 
            const float scrollDeltaY, 
            const float frameTime, 
            const bool leftButtonDown
        ){
            windowWidth = initialWidth;
            windowHeight = initialHeight;
            if(windowWidth == 0){windowWidth = 1;}
            if(windowHeight == 0){windowHeight = 1;}
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

        //If not using buildLayout(), call this instead of Clay_BeginLayout directly each frame before creating elements to reset string arena index
        void beginLayout(){
            start = std::chrono::high_resolution_clock::now();
            countFrames();
            resetStringArenaIndex();
            Clay_BeginLayout();
        }

        //If not using buildLayout(), call this instead of Clay_EndLayout directly each frame after creating elements to handle automatic element closures for preventing crashes.
        Clay_RenderCommandArray endLayout(){
            closeAllElements();
            measureTime();
            return Clay_EndLayout();
        }
        
        //Creates an element in-place. Automatically opens, applies configs, calls all child elements, and closes.
        void element(Clay_ElementDeclaration configs, std::function<void()> childLambda = nullptr) {
            openElement();
            applyElementConfigs(configs);
            childLambda();
            closeElement();           
        }
        
        //Creates an element in-place. Automatically opens, applies default configs, calls all child elements, and closes.
        void element(std::function<void()> childLambda = nullptr) {
            openElement();
            if(childLambda != nullptr){
                childLambda();
            }
            closeElement();           
        }

        //Manually opens an element with configurations, call closeElement() after children (if any) to close.
        void openElement(Clay_ElementDeclaration configs){
            openElement();
            applyElementConfigs(configs);
        }

        //Manually opens an element with default configurations, call closeElement() after children (if any) to close.
        void openElement(){
            Clay__OpenElement();
            openElementCount++;
        }

        //Manually closes an element. Call after the children of an element (if any) that was opened manually with openElement()
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

        //Convenience function for .sizing layout parameter
        Clay_Sizing fixedSize(const uint32_t w, const uint32_t h) {
            return{
                .width = (Clay_SizingAxis { .size = { .minMax = { (float)w, (float)w } }, .type = CLAY__SIZING_TYPE_FIXED }),
                .height = (Clay_SizingAxis { .size = { .minMax = { (float)h, (float)h } }, .type = CLAY__SIZING_TYPE_FIXED }) 
            };
        }

        //Convenience function for .sizing layout parameter
        Clay_Sizing expandXY(){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
                .height = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW })
            };
        }

        //Convenience function for .sizing layout parameter
        Clay_Sizing expandX(){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
            };
        }

        //Convenience function for .sizing layout parameter
        Clay_Sizing expandY(){
            return {
                .height = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
            };
        }

        //Convenience function for .sizing layout parameter
        Clay_Sizing expandXfixedY(const uint32_t h){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW }),
                .height = (Clay_SizingAxis { .size = { .minMax = { (float)h, (float)h } }, .type = CLAY__SIZING_TYPE_FIXED }) 
            };
        }

        //Convenience function for .sizing layout parameter
        Clay_Sizing expandYfixedX(const uint32_t w){
            return {
                .width = (Clay_SizingAxis { .size = { .minMax = { (float)w, (float)w } }, .type = CLAY__SIZING_TYPE_FIXED }),
                .height = (Clay_SizingAxis { .size = { .minMax = { {0} } }, .type = CLAY__SIZING_TYPE_GROW })
            };
        }

        //Convenience function for .padding layout parameter
        Clay_Padding padAll(const uint16_t p){
            return {p, p, p, p};
        }
       
        //Convenience function for .padding layout parameter
        Clay_Padding padX(const uint16_t p){
            return {p, p, 0, 0};
        }

        //Convenience function for .padding layout parameter
        Clay_Padding padY(const uint16_t p){
            return {0, 0, p, p};
        }
        
        //Convenience function for .padding layout parameter
        Clay_Padding padXY(const uint16_t px, const uint16_t py){
            return {px, px, py, py};
        }

        //Convenience function for .padding layout parameter
        Clay_Padding padLeft(const uint16_t pl){
            return {pl, 0, 0, 0};
        }

        //Convenience function for .padding layout parameter
        Clay_Padding padRight(const uint16_t pr){
            return {0, pr, 0, 0};
        }

        //Convenience function for .padding layout parameter
        Clay_Padding padTop(const uint16_t pt){
            return {0, 0, pt, 0};
        }

        //Convenience function for .padding layout parameter
        Clay_Padding padBottom(const uint16_t pb){
            return {0, 0, 0, pb};
        }

        //Convenience function for .childAlignment layout parameter
        Clay_ChildAlignment centerXY(){
            return {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER};
        }

        //Hashes string literal into a clay ID
        Clay_ElementId hashID(const Clay_String& id){
            return Clay__HashString(id, 0, 0);
        }

        //Hashes string into a clay ID
        Clay_ElementId hashID(const std::string& id){
            return Clay__HashString(toClayString(id), 0, 0);
        }

        //Gets clay internal left-mouse-button state this frame
        bool mousePressed(){
            return Clay_GetCurrentContext()->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME;
        }

        bool pointerOver(const Clay_String id){
            return Clay_PointerOver(getClayElementId(id));
        }

        bool pointerOver(const std::string& id){
            return Clay_PointerOver(getClayElementId(toClayString(id)));
        }

        template<size_t N>
        bool pointerOver(const char(&id)[N]){
            return Clay_PointerOver(getClayElementId(toClayString(id)));
        }

        Clay_ElementId getClayElementId(const Clay_String id){
            return Clay_GetElementId(id);
        }

        Clay_ElementId getClayElementId(const std::string& id){
            return Clay_GetElementId(toClayString(id));
        }

        template<size_t N>
        Clay_ElementId getClayElementId(const char(&id)[N]){
            return Clay_GetElementId(toClayString(id));
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

    ////////////////////////////////////////////////////////////private//////////////////////////////////////////////////////////////
    private:
        //One-shot for console warning
        bool warnedAboutClose = false;

        //One-shot for console warning
        bool warnedAboutUnderflow = false;

        uint32_t windowWidth;
        uint32_t windowHeight;
        uint32_t framecount = 0;
        std::chrono::_V2::system_clock::time_point start = std::chrono::high_resolution_clock::now();

        static constexpr size_t maxStringArenaSize = 100000; 

        //Reusable char arena to cache strings for Clay_String conversions
        char stringArena[maxStringArenaSize];

        //Tracks current position in string arena
        size_t nextStringArenaIndex = 0;

        //Tracks the heiarchy depth of the current element in the layout
        uint32_t openElementCount = 0;
        
        
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

        void applyElementConfigs(const Clay_ElementDeclaration& configs){
            Clay__ConfigureOpenElement((Clay__Clay_ElementDeclarationWrapper {configs}).wrapped);
        }

        void closeAllElements(){
            while(openElementCount > 0){
                if(!warnedAboutClose){
                    printf("WARN: An element was not closed.");
                    warnedAboutClose = true;
                }
                closeElement();
            }
        }

        //Clay_ErrorHandler
        static void handleErrors(Clay_ErrorData errorData) {
            printf("%s", errorData.errorText.chars);
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

        void measureTime(){
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
};

#endif //end of file