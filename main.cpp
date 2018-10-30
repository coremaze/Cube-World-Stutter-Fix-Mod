#include "main.h"

UINT_PTR base;

char query[] = "PRAGMA synchronous=OFF;\0";
unsigned int query_ptr = (unsigned int)&query;
unsigned int sqlite_exec;
unsigned int ASMSQLiteInjection_JMP_back;
void ASMSQLiteInjection(){
    asm("push 0");
    asm("push 0");
    asm("push 0");
    asm("push [_query_ptr]");
    asm("push dword ptr [edi+0x4]");
    asm("call [_sqlite_exec]");
    asm("add esp, 0x14");

    //original code
    asm("push esi");
    asm("mov esi,[ebp+0x08]");
    asm("cmp dword ptr [esi+0x14],0x10");
    asm("jmp [_ASMSQLiteInjection_JMP_back]");
}

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
            sqlite_exec = base + 0x120760; //function we'll use to exec sql
            ASMSQLiteInjection_JMP_back = base + 0x49A24;
            WriteJMP((BYTE*)(base + 0x49A1C), (BYTE*)&ASMSQLiteInjection);
            break;
    }
    return TRUE;
}
