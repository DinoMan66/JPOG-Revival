// Toshi::TApplication - reconstruction of Z:\Toshi\Source\TApplication\TApplication.cpp
// Target: installed TApplication.dll (SHA1 61af5110...), Intel C++ 5.0.1, -O2 -Gz -vmg -vmm
//
// STATUS: COMPLETE - 20/20 functions match the installed TApplication.dll:
//   16 exported functions, FindConsoleWindow, operator new/delete, and the vector deleting
//   destructor (??_E at 0x10d0). Verify with:
//     py tools\compare.py original\TApplication.dll results\TApplication.txt
//        --map notes\TApplication.map --tracker notes\function_tracker.csv
//
// KEY DISCOVERIES (also in notes\MATCHING_NOTES.md):
//   * the class is declared __declspec(dllexport) -> compiler emits the vector deleting dtor
//   * -Gz makes free functions __stdcall; operator new/delete stay __cdecl
//   * bool tests are written "x == true"; "return" shapes follow if(!x) return false; return true;
//   * m_pKernel is an intrusive ref-counted pointer whose inlined code re-checks null after storing

#define TAPPLICATION_EXPORTS      // the real DLL exports the whole class (see TApplication.h)
#include <windows.h>
#include <stdlib.h>
#include "TApplication.h"

// ---------------------------------------------------------------- global memory operators
// Non-exported in the DLL (0x1520 new / 0x1510 delete). Defined BEFORE use so that Create()
// and the constructor inline tmalloc(size, 0, -1) exactly as the original does.
void* __cdecl operator new(size_t size)
{
    return tmalloc((int)size, 0, -1);
}

void __cdecl operator delete(void* p)
{
    tfree(p);
}

namespace Toshi {

// ---------------------------------------------------------------- file-local helper (0x1000)
// Finds this process's console window by giving the console a unique title, locating that
// title with FindWindow, then restoring the old title. (No GetConsoleWindow on Win9x.)
static HWND FindConsoleWindow()
{
    CHAR oldTitle[512];
    TCString title("{1B5D8052-529A-4c88-9DDE-EE523D440B00}");
    char hex[24];

    _ultoa(timeGetTime(), hex, 16);
    title += hex;

    if (GetConsoleTitleA(oldTitle, 512) == 0)
        return 0;

    SetConsoleTitleA((const char*)title);
    HWND hwnd = FindWindowA(0, (const char*)title);
    SetConsoleTitleA(oldTitle);
    return hwnd;
}

// ---------------------------------------------------------------- construction / destruction
TApplication::TApplication()
{
    m_flags = 0;
    m_verbose = true;
    m_consoleVisible = true;
    m_pConsoleHwnd = (void**)new HWND;
    *m_pConsoleHwnd = FindConsoleWindow();
    ShowConsole(false);
}

TApplication::TApplication(const TApplication& rhs)
    : m_pKernel(rhs.m_pKernel), m_name(rhs.m_name)
{
    m_flags = rhs.m_flags;
    m_verbose = rhs.m_verbose;
    m_consoleVisible = rhs.m_consoleVisible;
    m_pConsoleHwnd = rhs.m_pConsoleHwnd;
}

TApplication::~TApplication()
{
    // members are destroyed in reverse order: m_name, then the m_pKernel smart pointer
}

TApplication& TApplication::operator=(TApplication& rhs)
{
    m_pKernel = rhs.m_pKernel;
    m_name = rhs.m_name;
    m_flags = rhs.m_flags;
    m_verbose = rhs.m_verbose;
    m_consoleVisible = rhs.m_consoleVisible;
    m_pConsoleHwnd = rhs.m_pConsoleHwnd;
    return *this;
}

// ---------------------------------------------------------------- lifecycle
bool TApplication::Create(const char* name, int argc, char** const argv)
{
    m_name = name;
    m_pKernel = new TKernelInterface(argc, argv, m_verbose);

    if (!OnCreate(argc, argv))
        return false;
    return true;
}

bool TApplication::Execute()
{
    if (m_pKernel->Update() == true)
    {
        do
        {
            float dt = m_pKernel->GetSystemTimer()->GetDelta();
            if (!OnUpdate(dt) || (m_flags & 2))
                break;
        } while (m_pKernel->Update() == true);
    }

    if (!OnDestroy())
        return false;
    return true;
}

void TApplication::DisplayErrorMessageBox(TCString message)
{
    MessageBoxA((HWND)*m_pConsoleHwnd, message.GetString(0), "Error", MB_OK);
}

// ---------------------------------------------------------------- matched (batch 1 + 2)
bool TApplication::OnCreate(int, char** const)
{
    m_flags |= 1;
    return true;
}

bool TApplication::OnUpdate(float)
{
    if (m_flags & 2)
        return false;
    return true;
}

bool TApplication::OnDestroy()
{
    return true;
}

bool TApplication::IsCreated()
{
    return (m_flags & 1) != 0;
}

void TApplication::Destroy()
{
    m_flags |= 2;
}

TKernelInterface* TApplication::GetKernel()
{
    return m_pKernel;
}

void TApplication::SetVerbose(bool verbose)
{
    m_verbose = verbose;
}

bool TApplication::ShowConsole(bool show)
{
    bool wasVisible = m_consoleVisible;
    ShowWindow((HWND)*m_pConsoleHwnd, (show == true) ? SW_SHOWNORMAL : SW_HIDE);
    UpdateWindow((HWND)*m_pConsoleHwnd);
    m_consoleVisible = show;
    return wasVisible;
}

bool TApplication::ToggleConsole()
{
    ShowConsole((m_consoleVisible == true) ? false : true);
    return m_consoleVisible;
}

} // namespace Toshi
