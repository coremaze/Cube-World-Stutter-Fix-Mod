#include "main.h"

UINT_PTR base;


void DLL_EXPORT ASMSQLFunction(){

    asm("call [_shouldUpdatePtr]");
    asm("test al, al");
    asm("jnz 0f");

    asm("mov eax, [_base]"); //skip saving
    asm("add eax, 0x20C83D");
    asm("jmp eax");

    asm("0:"); //continue as normal
    asm("mov eax, [_base]");
    asm("add eax, 0x20C723");
    asm("lea ecx, [edi+0x0AC]"); //original code
    asm("jmp eax");

}

bool isFloatWithinThreshold(float f, float thres){
    return f <= thres && f >= -thres;
}

bool shouldUpdate(){
    DWORD entityaddr = (DWORD)(base + 0x36b1c8);
    entityaddr = *(DWORD*)entityaddr;
    entityaddr += 0x39C;
    entityaddr = *(DWORD*)entityaddr;
    float* velocityx = (float*)(entityaddr+0x34);
    float* velocityy = (float*)(entityaddr+0x38);
    float* velocityz = (float*)(entityaddr+0x3C);
    float threshold = 0.01;
    //Stop the SQL stuff from happening if the player is moving.
    //When the player is standing still, it will resume saving as normal.
    if (isFloatWithinThreshold(*velocityx, threshold) &&
        isFloatWithinThreshold(*velocityy, threshold) &&
        isFloatWithinThreshold(*velocityz, threshold)){
        return true;
    }
    else {
        return false;
    }
}
DWORD shouldUpdatePtr = (DWORD)&shouldUpdate;


void WriteJMP(BYTE* location, BYTE* newFunction){
	DWORD dwOldProtection;
	VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(( (unsigned INT32)newFunction - (unsigned INT32)location ) - 5);
	VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:
            WriteJMP((BYTE*)(base + 0x20C71D), (BYTE*)&ASMSQLFunction);
            break;
    }
    return TRUE;
}
