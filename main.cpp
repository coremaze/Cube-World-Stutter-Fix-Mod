#include <windows.h>
#include "callbacks.h"

unsigned int base;
float multiplier = 1.0;
float maxcrafting = 1.0;

unsigned int CraftingProgressInjection_JMP_back;
__declspec(naked) __declspec(dllexport) void  CraftingProgressInjection(){
    //Just apply the multiplier
    asm("mulss xmm0, [_multiplier]");

    asm("addss xmm0, xmm1"); //original code
    asm("comiss xmm0, [_maxcrafting]");

    asm("jmp [_CraftingProgressInjection_JMP_back]");
}

//When the cursor leaves the button.
__declspec(naked) void __declspec(dllexport) CraftingDone(){
    asm("mov dword ptr _multiplier, 0x3F800000"); //reset multiplier
    asm("mov byte ptr [edi + 0x3C4], 0x0");

    asm("mov esi, [_base]"); //jump back
    asm("add esi, 0x34711");
    asm("jmp esi");

}

__stdcall void __attribute__((noinline)) __declspec(dllexport) HandleFinishCrafting(){
    /*
    This seems to be the correct calculation.
    Wollay says it increases by 20% each time, and this max speed seems to be correct.
    */
    if (multiplier < 5.0){
        multiplier *= 1.20;
    }
}


void WriteJMP(BYTE* location, BYTE* newFunction){
    DWORD dwOldProtection;
    VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(( (unsigned INT32)newFunction - (unsigned INT32)location ) - 5);
    VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}


DWORD WINAPI __attribute__((noinline)) __declspec(dllexport) RegisterCallbacks(){
        RegisterCallback("RegisterFinishCraftingCallback", HandleFinishCrafting);
        return 0;
}



extern "C" __declspec(dllexport) bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (unsigned int)GetModuleHandle(NULL);
    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:
            CraftingProgressInjection_JMP_back = base + 0x8F20E;
            WriteJMP((BYTE*)(base + 0x8F203), (BYTE*)&CraftingProgressInjection);

            WriteJMP((BYTE*)(base + 0x3470A), (BYTE*)&CraftingDone);
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return true;
}
