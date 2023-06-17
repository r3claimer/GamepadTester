#include <iostream>
#include <iomanip>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_gfxPrimitives_font.h>

#define STICK_MAX_VAL (float(0x7fff))
#define WIN_W 1100
#define WIN_H 800

std::string cStringToStdString(const char* str, const std::string& or_="N/A")
{
    return str ? str : or_;
}

SDL_GameController* openControllerAndPrintInfo(int index)
{
    SDL_GameController* gcont = SDL_GameControllerOpen(index);
    std::cout << "\tName: " << cStringToStdString(SDL_GameControllerName(gcont)) << '\n';
    std::cout << "\tPath: " << cStringToStdString(SDL_GameControllerPath(gcont)) << '\n';
    std::cout << "\tVID:PID: " << std::hex << std::setfill('0')
        << std::setw(4) << SDL_GameControllerGetVendor(gcont) << ':'
        << std::setw(4) << SDL_GameControllerGetProduct(gcont) << std::dec << '\n';
    std::cout << "\tRumble support? " << (SDL_GameControllerHasRumble(gcont) ? "YES" : "NO") << '\n';
    return gcont;
}

struct StickState
{
    float x{};
    float y{};
};

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Gamepad Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_W, WIN_H, 0);
    SDL_Renderer* rend = SDL_CreateRenderer(window, 0, 0);
    gfxPrimitivesSetFont(gfxPrimitivesFontdata, 8, 8);

    SDL_GameControllerAddMappingsFromFile("../gamecontrollerdb.txt");

    SDL_GameController* gcont = nullptr;
    StickState leftStickState{};
    StickState rightStickState{};
    bool buttonStates[SDL_CONTROLLER_BUTTON_MAX]{};
    uint8_t dpadState = 0;
    float lTriggerState = 0.0f;
    float rTriggerState = 0.0f;

    bool running = true;
    while (true)
    {
        SDL_Event event;
        //SDL_PumpEvents();
        while (running && SDL_PollEvent(&event))
        //while (running && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_CONTROLLERDEVICEADDED:
                std::cout << "Controller added\n";
                gcont = openControllerAndPrintInfo(event.cdevice.which);
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                std::cout << "Controller removed\n";
                gcont = nullptr;
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                std::cout << "Controller button down: " << +event.cbutton.button  << '\n';
                SDL_GameControllerRumble(gcont, 0, 0xffff/2, 200);
                buttonStates[event.cbutton.button] = true;
                break;

            case SDL_CONTROLLERBUTTONUP:
                std::cout << "Controller button up: " << +event.cbutton.button  << '\n';
                buttonStates[event.cbutton.button] = false;
                break;

            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.button == 8)
                    buttonStates[SDL_CONTROLLER_BUTTON_GUIDE] = true;
                break;

            case SDL_JOYBUTTONUP:
                if (event.jbutton.button == 8)
                    buttonStates[SDL_CONTROLLER_BUTTON_GUIDE] = false;
                break;

            case SDL_CONTROLLERAXISMOTION:
                std::cout << "Controller axis movement\n";
                std::cout << "\tAxis: " << +event.caxis.axis << '\n';
                std::cout << "\tValue: " << event.caxis.value/STICK_MAX_VAL << '\n';
                switch (event.caxis.axis)
                {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    leftStickState.x = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    leftStickState.y = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    rightStickState.x = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    rightStickState.y = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                    lTriggerState = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                    rTriggerState = event.caxis.value/STICK_MAX_VAL;
                    break;
                }
                break;

            case SDL_JOYHATMOTION:
                std::cout << "Joyhat motion\n";
                std::cout << +event.jhat.value << '\n';
                dpadState = event.jhat.value;
                break;

            default:
                std::cout << "Event: 0x" << std::hex << event.type << std::dec << std::endl;
                break;
            }
        }
        if (!running) break;


        SDL_SetRenderDrawColor(rend, 40, 40, 40, 255);
        SDL_RenderClear(rend);

        if (gcont)
        {
            stringColor(rend, 10, 10, cStringToStdString(SDL_GameControllerName(gcont), "Unknown").c_str(), 0xffffffff);

            // Left analog stick
            aacircleColor(rend, 300, 300, 60, 0xffffffff);
            aacircleColor(rend, 300+leftStickState.x*30, 300+leftStickState.y*30, 30, 0xffffffff);
            filledCircleColor(rend, 300+leftStickState.x*30, 300+leftStickState.y*30, 30,
                    (buttonStates[SDL_CONTROLLER_BUTTON_LEFTSTICK] ? 0xffffffff : 0));

            // Right analog stick
            aacircleColor(rend, WIN_W-300, WIN_H-300, 60, 0xffffffff);
            aacircleColor(rend, WIN_W-300+rightStickState.x*30, WIN_H-300+rightStickState.y*30, 30, 0xffffffff);
            filledCircleColor(rend, WIN_W-300+rightStickState.x*30, WIN_H-300+rightStickState.y*30, 30,
                    (buttonStates[SDL_CONTROLLER_BUTTON_RIGHTSTICK] ? 0xffffffff : 0));

            // Y button
            aacircleRGBA(rend, WIN_W-300, 300-40, 20, 255, 255, 0, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_Y])
                filledCircleRGBA(rend, WIN_W-300, 300-40, 20, 255, 255, 0, 255);

            // X button
            aacircleRGBA(rend, WIN_W-300-40, 300+40-40, 20, 100, 100, 255, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_X])
                filledCircleRGBA(rend, WIN_W-300-40, 300+40-40, 20, 100, 100, 255, 255);

            // B button
            aacircleRGBA(rend, WIN_W-300+40, 300+40-40, 20, 255, 0, 0, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_B])
                filledCircleRGBA(rend, WIN_W-300+40, 300+40-40, 20, 255, 0, 0, 255);

            // A button
            aacircleRGBA(rend, WIN_W-300, 300+80-40, 20, 0, 255, 0, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_A])
                filledCircleRGBA(rend, WIN_W-300, 300+80-40, 20, 0, 255, 0, 255);

            // D-pad up
            aatrigonRGBA(rend, 300, WIN_H-360, 280, WIN_H-320, 320, WIN_H-320, 255, 255, 255, 255);
            if (dpadState & SDL_HAT_UP)
                filledTrigonRGBA(rend, 300, WIN_H-360, 280, WIN_H-320, 320, WIN_H-320, 255, 255, 255, 255);

            // D-pad down
            aatrigonRGBA(rend, 300, WIN_H-240, 280, WIN_H-280, 320, WIN_H-280, 255, 255, 255, 255);
            if (dpadState & SDL_HAT_DOWN)
                filledTrigonRGBA(rend, 300, WIN_H-240, 280, WIN_H-280, 320, WIN_H-280, 255, 255, 255, 255);

            // D-pad left
            aatrigonRGBA(rend, 240, WIN_H-300, 280, WIN_H-320, 280, WIN_H-280, 255, 255, 255, 255);
            if (dpadState & SDL_HAT_LEFT)
                filledTrigonRGBA(rend, 240, WIN_H-300, 280, WIN_H-320, 280, WIN_H-280, 255, 255, 255, 255);

            // D-pad right
            aatrigonRGBA(rend, 360, WIN_H-300, 320, WIN_H-320, 320, WIN_H-280, 255, 255, 255, 255);
            if (dpadState & SDL_HAT_RIGHT)
                filledTrigonRGBA(rend, 360, WIN_H-300, 320, WIN_H-320, 320, WIN_H-280, 255, 255, 255, 255);

            // Left bumper
            rectangleColor(rend, 300-50, 300-150-20, 300+50, 300-150+20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_LEFTSHOULDER])
                boxColor(rend, 300-50, 300-150-20, 300+50, 300-150+20, 0xffffffff);

            // Right bumper
            rectangleColor(rend, WIN_W-300-50, 300-150-20, WIN_W-300+50, 300-150+20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER])
                boxColor(rend, WIN_W-300-50, 300-150-20, WIN_W-300+50, 300-150+20, 0xffffffff);

            // Left trigger
            rectangleColor(rend, 300-50-80, 300-150-20, 300-50-80+40, 300-150+20+80, 0xffffffff);
            boxColor(rend, 300-50-80, 130, 300-50-80+40, 130+120*lTriggerState, 0xffffffff);

            // Right trigger
            rectangleColor(rend, WIN_W-300+50+40, 300-150-20, WIN_W-300+50+80, 300-150+20+80, 0xffffffff);
            boxColor(rend, WIN_W-300+50+40, 130, WIN_W-300+50+80, 130+120*rTriggerState, 0xffffffff);

            // Back button
            aacircleColor(rend, WIN_W/2-40, WIN_H/2, 20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_BACK])
                filledCircleColor(rend, WIN_W/2-40, WIN_H/2, 20, 0xffffffff);

            // Start button
            aacircleColor(rend, WIN_W/2+40, WIN_H/2, 20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_START])
                filledCircleColor(rend, WIN_W/2+40, WIN_H/2, 20, 0xffffffff);

            // Guide button
            aacircleColor(rend, WIN_W/2, WIN_H/2-60, 30, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_GUIDE])
                filledCircleColor(rend, WIN_W/2, WIN_H/2-60, 30, 0xffffffff);
        }
        else
        {
            stringColor(rend, WIN_W/2-22*8/2, WIN_H/2-8/2, "NO CONTROLLER DETECTED", 0xffffffff);
        }

        SDL_RenderPresent(rend);
        SDL_Delay(16);
    }

    SDL_GameControllerClose(gcont);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
