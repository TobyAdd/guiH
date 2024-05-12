#include <Windows.h>
#include "guiH.hpp"
#include "hookH.hpp"

WNDPROC PrevProc;
bool inited_Wnd = false;

bool logs = false;
bool yes_no = false;
std::string text;

LRESULT CALLBACK NewWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (guiH::WndProc(hWnd, Msg, wParam, lParam)) {
        return false;
    }

    return CallWindowProc(PrevProc, hWnd, Msg, wParam, lParam);
}

guiH_Window Window1;
guiH_Window Window2;
guiH_Window Window3;

typedef BOOL(__stdcall* SwapBuffersType)(HDC hdc);
SwapBuffersType SwapBuffersOriginal;

BOOL __stdcall SwapBuffersHook(HDC hdc) {
    guiH::Frame_Update(hdc);
    guiH::Frame_Begin(hdc);

    Window1.Render();
    Window2.Render();
    Window3.Render();

	if (logs) { 
		std::string log;
		log += std::to_string(guiH::IO.MousePos.X) + "; " + std::to_string(guiH::IO.MousePos.Y) + "\n";
		log += std::to_string(guiH::IO.MouseDown[0]) + "; " + std::to_string(guiH::IO.MouseHold[0]) + "; " + std::to_string(guiH::IO.MouseRelease[0]) + "\n";
		log += std::string(yes_no ? "Checked" : "Not Checked") + "\n";
		log += "Input text: " + text;
		guiH::Graphics::Font::RenderText(5, 5, log.c_str(), {255, 255, 255});
	}

    guiH::Frame_End(hdc);

    if (!inited_Wnd) {
        inited_Wnd = true;
        PrevProc = (WNDPROC)GetWindowLongPtr(guiH::IO.Window, GWLP_WNDPROC);
        SetWindowLongPtr(guiH::IO.Window, GWLP_WNDPROC, (LONG_PTR)NewWndProc);
    }
    return SwapBuffersOriginal(hdc);
}

void Main() {
	Window1.Caption = "Hello 1";
    Window1.Position = {10, 10};
    Window1.Size = {200, 200};
    guiH_Button Button1 = {"Enable Logs", {5, 5}, {190, 20}};
	Button1.setButtonPressHandler([]() 
    {
		logs = true;
    });

    Window1.Buttons.push_back(Button1);

	guiH_Checkbox Checkbox1 = {"Test", {5, 30}};
	Checkbox1.setCheckboxPressHandler([](bool checked) 
    {
		yes_no = checked;
    });

	guiH_TextBox textbox = {"", {5, 55}, {180, 18}};
	textbox.TextHint = "Enter your text here...";
	textbox.setEditedTextHandler([](std::string edited_text) 
    {
		text = edited_text;
    });

    Window1.Buttons.push_back(Button1);
	Window1.Checkboxes.push_back(Checkbox1);
	Window1.TextBoxes.push_back(textbox);

    Window2.Caption = "Hello 2";
    Window2.Position = {220, 10};
    Window2.Size = {200, 200};
    guiH_Button Button2 = {"Disable Logs", {5, 5}, {190, 20}};
	Button2.setButtonPressHandler([]() 
    {
		logs = false;
    });

    Window2.Buttons.push_back(Button2);

    Window3.Caption = "Hello 3";
    Window3.Position = {430, 10};
    Window3.Size = {200, 200};
    guiH_Button Button3 = {"Show Message", {5, 5}, {190, 20}};
	Button3.setButtonPressHandler([]() 
    {
		MessageBoxA(0,0,0,0);
    });

    Window3.Buttons.push_back(Button3);

    SwapBuffersOriginal = (SwapBuffersType)GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");
    SwapBuffersOriginal = (SwapBuffersType)HookH::TrampHook32((BYTE*)SwapBuffersOriginal, (BYTE*)SwapBuffersHook, 5);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        Main();
    }
    return TRUE; 
}
