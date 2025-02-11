
#include "layout.hpp"


//User layout callback function, do not call Clay_BeginLayout() or Clay_EndLayout()
void myLayout(ClayMan& clayMan){
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

    //Here we are using openElement(). We need to call closeElement() after children block
    clayMan.openElement({
        .id = clayMan.hashID("OuterContainer"),
        .layout = {
            .sizing = clayMan.expandXY(),
            .padding = clayMan.padAll(16),
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = {50, 50, 50, 255}
    });{ //Children of OuterContainer are in this block. The block is not necessary, but helps with structure.
        //Text Elements are self-contained
        clayMan.textElement("The outer container was made with openElement function and needs closed manually", textConfig);

        //Here we are using the lambda method, we do not need to call closeElement()
        clayMan.element(
            {
                .id = clayMan.hashID("Button"),
                .layout = {
                    .sizing = clayMan.fixedSize(100, 40),
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
                },
                .backgroundColor = clayMan.pointerOver("Button")? //ternary in-line config property
                    Clay_Color{120, 120, 120, 255}
                    : Clay_Color{100, 100, 100, 255}
            },
            [&]{ //children of "Button" are in this lambda
                clayMan.textElement("Click Me", textConfig);
            }
        ); //do not call closeElement() for "Button"

        clayMan.textElement(theText, textConfig);
        
        //Here is another manual element
        clayMan.openElement( //we manually open the element
            {
                .id = clayMan.hashID("ManualElement"),
                .layout = {
                    .padding = clayMan.padXY(26, 16),
                    .childGap = 16,
                    .childAlignment = clayMan.centerXY()
                },
                .backgroundColor = {0,0,0,255}
            }
        );{//Children of "ManualElement" are in this block, the block is not necessary, but helps with structure
            clayMan.textElement("This parent element was made manually.", textConfig);

            //Another lambda element
            clayMan.element(
                {
                    .id = clayMan.hashID("LambdaElement"),
                    .layout = {.padding = clayMan.padAll(16)},
                    .backgroundColor = {120,120,120,255}
                },
                [&]{
                    clayMan.textElement("This element was made with the lambda method using element function.", textConfig);
                }
            );

            //A classic macro element
            CLAY({
                .id = CLAY_ID("ClayElement"),
                .layout = { 
                    .padding = CLAY_PADDING_ALL(16), 
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER, 
                        .y = CLAY_ALIGN_Y_CENTER
                    }
                },
                .backgroundColor =  (Clay_Hovered()? (Clay_Color){200,200,200,255}: (Clay_Color){100,100,100,255}),
                .cornerRadius = 10
            }){
                CLAY_TEXT(CLAY_STRING("This element was made with the standard Clay macros inside a lambda element"), CLAY_TEXT_CONFIG(textConfig));
            }
        }
        clayMan.closeElement(); //we manually close the "ManualElement" since closeElement() was used


        clayMan.element(
            {
                .id = clayMan.hashID("AnotherElement"),
                .layout = {
                    .sizing = clayMan.expandX(),
                    .padding = clayMan.padAll(16),
                    .childAlignment = clayMan.centerXY()
                },
                .backgroundColor = {0,0,0,125},
                .cornerRadius = {8,8,8,8},
                .border = {
                    .color = {123,123,0,255},
                    .width = {5,5,5,5,0}
                }
            },
            [&]{
                clayMan.element(
                    {
                        .layout = {
                            .padding = clayMan.padAll(16)
                        },
                        .backgroundColor = {255,255,255,123}
                    },
                    [&]{
                        clayMan.textElement("This element was made using the lambda method", textConfig);
                    }
                );
            }
        );
        
    } clayMan.closeElement(); //We close the outer container manually, since openElement() was used.
}