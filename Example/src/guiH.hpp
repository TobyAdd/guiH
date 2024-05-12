#define _USE_MATH_DEFINES
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#include <Windows.h>
#pragma comment(lib, "opengl32")
#include <math.h>
#include <GL/GL.h>
#include <string>
#include <vector>
#include <functional>


struct guiH_Point {
    float X, Y;
};

struct guiH_Size {
    float Width, Height;
};

struct guiH_RGB {
    BYTE Red, Green, Blue;
};

struct guiH_LineDrawFlags {
    bool drawTop;
    bool drawRight;
    bool drawBottom;
    bool drawLeft;
};

struct guiH_IO { 
    HWND Window;
    HGLRC Context_Original;
    HGLRC Context_New; 

    guiH_Size DisplaySize;
    guiH_Size prevDisplaySize;

    guiH_Point MousePos;
    
    bool MouseDown[1]; // 0 - Left, 1 - Right
    bool MouseHold[1]; // 0 - Left, 1 - Right
    bool MouseRelease[1]; // 0 - Left, 1 - Right

    bool WindowWantToDrag = false;

    std::string active_window;

    bool TextboxFocused = false;
    bool WantToWriteTextboxChar = false;
    char WantTextboxChar;
};

struct guiH_Button {
    std::string Caption;
    guiH_Point Position;
    guiH_Size Size;

    std::function<void()> handlerButton = []() {};
    void setButtonPressHandler(std::function<void()> func) { handlerButton = func; }
#if 0
    guiH_Button Button;
    Button.setButtonPressHandler([]() 
    {
        //Action on press
    });
#endif
};

struct guiH_Checkbox {
    std::string Caption;
    guiH_Point Position;
    bool Checked = false;

    std::function<void(bool)> handlerCheckbox = [](bool) {};
    void setCheckboxPressHandler(std::function<void(bool)> func) { handlerCheckbox = func; }
#if 0
    guiH_Button Button;
    Button.setButtonPressHandler([]() 
    {
        //Action on press
    });
#endif
};

struct guiH_TextBox {
    std::string Text;
    guiH_Point Position;
    guiH_Size Size;

    std::string TextHint;

    bool Focused = false;

    std::function<void(std::string)> handleTextboxEdited = [](std::string) {};
    void setEditedTextHandler(std::function<void(std::string)> func) { handleTextboxEdited = func; }
#if 0
    guiH_TextBox Textbox;
    std::string text;
    Textbox.setEditedTextHandler([](std::string edited_text) 
    {
        text = edited_text;
    });
#endif
};


struct guiH_Window {
    std::string Caption;
    guiH_Point Position;
    guiH_Size Size;
    bool isDraggingWindow = false;
    bool WantToClickOnButton = false;
    bool WantToClickOnCheckbox = false;

    std::vector<guiH_Button> Buttons;
    std::vector<guiH_Checkbox> Checkboxes;
    std::vector<guiH_TextBox> TextBoxes;

    void Render();
};

namespace guiH {
    bool Inited = false; 
    guiH_IO IO;

    namespace Graphics {
        void DrawFilledRect(float x, float y, float width, float height, guiH_RGB RGB) {
            glColor3ub(RGB.Red, RGB.Green, RGB.Blue);
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + width, y);
            glVertex2f(x + width, y + height);
            glVertex2f(x, y + height);
            glEnd();
        }

        void DrawOutLine(float x, float y, float width, float height, float lineWidth, guiH_RGB RGB, guiH_LineDrawFlags drawFlags = {true, true, true, true}) {
            glLineWidth(lineWidth);
            glBegin(GL_LINES);
            glColor3ub(RGB.Red, RGB.Green, RGB.Blue);
            
            if (drawFlags.drawTop) {
                glVertex2f(x, y);
                glVertex2f(x + width, y);
            }
            if (drawFlags.drawRight) {
                glVertex2f(x + width, y);
                glVertex2f(x + width, y + height);
            }
            if (drawFlags.drawBottom) {
                glVertex2f(x + width, y + height);
                glVertex2f(x, y + height);
            }
            if (drawFlags.drawLeft) {
                glVertex2f(x, y + height + 1);
                glVertex2f(x, y);
            }
            
            glEnd();
        }

        void DrawFilledCircle(float cx, float cy, float radius, guiH_RGB RGB) {
            glColor3ub(RGB.Red, RGB.Green, RGB.Blue);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            for (float angle = 0; angle <= 360; angle += 1) {
                float radian = angle * ((float)M_PI / 180);
                glVertex2f(cx + radius * cosf(radian), cy + radius * sinf(radian));
            }
            glEnd();
        }

        void DrawArrowDown(float x, float y, float size, guiH_RGB RGB) {
            float halfSize = size / 2.0f;
            float tipY = y + size;
            float tipX = x + halfSize;
            float baseY = y;
            float baseX1 = x - halfSize;
            float baseX2 = x + size + halfSize;
            
            glColor3ub(RGB.Red, RGB.Green, RGB.Blue);
            glBegin(GL_TRIANGLES);
            glVertex2f(tipX, tipY);
            glVertex2f(baseX1, baseY);
            glVertex2f(baseX2, baseY);
            glEnd();
        }

        namespace Font {
            GLuint Base = 0;
            int Height = 14;
            HFONT hFont = nullptr;
            HDC hdc = nullptr;

            void InitializeFont() {
                if (Base == 0) {
                    Base = glGenLists(256);
                    hFont = CreateFontA(-Height, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                        FF_DONTCARE | DEFAULT_PITCH, "MS Sans Serif");
                    hdc = wglGetCurrentDC();
                    SelectObject(hdc, hFont);
                    wglUseFontBitmaps(hdc, 0, 255, Base);
                }
            }

            void RenderText(float x, float y, const char* text, guiH_RGB RGB) {
                InitializeFont();
                glColor3ub(RGB.Red, RGB.Green, RGB.Blue);

                const char* lineStart = text;
                const char* c;
                for (c = text; *c != '\0'; ++c) {
                    if (*c == '\n') {
                        glRasterPos2f(x, y + Font::Height);
                        glListBase(Font::Base);
                        glCallLists(static_cast<GLsizei>(c - lineStart), GL_UNSIGNED_BYTE, lineStart);
                        y += Font::Height;
                        lineStart = c + 1;
                    }
                }

                if (c != lineStart) {
                    glRasterPos2f(x, y + Font::Height);
                    glListBase(Font::Base);
                    glCallLists(static_cast<GLsizei>(c - lineStart), GL_UNSIGNED_BYTE, lineStart);
                }
            }


            float CalculateTextWidth(const char* text) {
                if (hdc == nullptr || hFont == nullptr) {
                    return -1.0f;
                }

                SIZE size;
                HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);
                GetTextExtentPoint32A(hdc, text, static_cast<int>(strlen(text)), &size);
                SelectObject(hdc, hFontOld);

                return static_cast<float>(size.cx);
            }
        }
    }

    void Frame_Update(HDC hdc) {
        IO.Window = WindowFromDC(hdc);
        HWND Window = IO.Window;

        if (!Inited) {
            Inited = true;
            IO.Context_New = wglCreateContext(hdc);
        }

        RECT Window_Rect;
        GetClientRect(Window, &Window_Rect);
        IO.DisplaySize.Width = (float)Window_Rect.right;
        IO.DisplaySize.Height = (float)Window_Rect.bottom;

        POINT MousePosition;
        GetCursorPos(&MousePosition);
        ScreenToClient(Window, &MousePosition);

        IO.MousePos.X = (float)MousePosition.x;
        IO.MousePos.Y = (float)MousePosition.y;

        if (IO.DisplaySize.Width != IO.prevDisplaySize.Width || IO.DisplaySize.Height != IO.prevDisplaySize.Height) {
            wglDeleteContext(IO.Context_New);
            IO.Context_New = wglCreateContext(hdc);

            IO.prevDisplaySize.Width = IO.DisplaySize.Width;
            IO.prevDisplaySize.Height = IO.DisplaySize.Height;

            Graphics::Font::Base = 0;
        }
    }

    void Frame_Begin(HDC hdc) {
        IO.Context_Original = wglGetCurrentContext();
        wglMakeCurrent(hdc, IO.Context_New);

        glPushMatrix();
        glOrtho(0, IO.DisplaySize.Width, IO.DisplaySize.Height, 0, -1, 1);
    }

    void Frame_End(HDC hdc) {
        glPopMatrix();
        wglMakeCurrent(hdc, IO.Context_Original);
    }

    bool WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
        bool ret = false;

        if (IO.Window == GetForegroundWindow()) {
            if (Msg == WM_LBUTTONDOWN) {
                IO.MouseDown[0] = true;
                IO.MouseHold[0] = true;

                IO.MouseRelease[0] = false;
            }
            else if (Msg == WM_LBUTTONUP) {
                IO.MouseDown[0] = false;
                IO.MouseHold[0] = false;

                IO.MouseRelease[0] = true;
            }
            else if (Msg == WM_CHAR && IO.TextboxFocused) {
                IO.WantToWriteTextboxChar = true;
                IO.WantTextboxChar = static_cast<char>(wParam);
            }
        }
        return ret;
    }
}

float windowOriginalX;
float windowOriginalY;
int mouseXOffset = 0;
int mouseYOffset = 0;

void guiH_Window::Render() {
    using namespace guiH;

    float screenWidth = IO.DisplaySize.Width;
    float screenHeight = IO.DisplaySize.Height;

    // if (IO.MousePos.X >= Position.X && IO.MousePos.X <= Position.X + Size.Width &&
    //     IO.MousePos.Y >= Position.Y && IO.MousePos.Y <= Position.Y + Size.Height) {
    //     if (IO.MouseHold[0] && !IO.WindowWantToDrag) {
    //         IO.active_window = Caption;
    //     }
    // }

    if (IO.MousePos.X >= Position.X && IO.MousePos.X <= Position.X + Size.Width &&
        IO.MousePos.Y >= Position.Y && IO.MousePos.Y <= Position.Y + 20) {
        if (IO.MouseHold[0] && !IO.WindowWantToDrag && !isDraggingWindow && !WantToClickOnButton) {
            isDraggingWindow = true;
            IO.WindowWantToDrag = true;
            mouseXOffset = (int)IO.MousePos.X;
            mouseYOffset = (int)IO.MousePos.Y;
            windowOriginalX = Position.X;
            windowOriginalY = Position.Y;
        }
        else if (!IO.MouseHold[0]) {
            isDraggingWindow = false;
            IO.WindowWantToDrag = false;
        }
    }

    if (IO.WindowWantToDrag && isDraggingWindow) {
        float newX = windowOriginalX + (IO.MousePos.X - mouseXOffset);
        Position.X = max(0, min(screenWidth - Size.Width, newX));

        float newY = windowOriginalY + (IO.MousePos.Y - mouseYOffset);
        Position.Y = max(0, min(screenHeight - Size.Height, newY));
    }

    Graphics::DrawFilledRect(Position.X, Position.Y, Size.Width, 25, {58, 58, 58});
    Graphics::Font::RenderText(Position.X + 5, Position.Y + 5, Caption.c_str(), {255, 255, 255});

    Graphics::DrawFilledRect(Position.X, Position.Y + 25, Size.Width, Size.Height - 25, {27, 27, 29});
    Graphics::DrawOutLine(Position.X, Position.Y, Size.Width, Size.Height, 1, {255, 255, 255});

    for (size_t i = 0; i < Buttons.size(); i++) {
        bool hover = false;
        bool over = false;

        if (IO.MousePos.X >= Position.X + Buttons[i].Position.X && IO.MousePos.X <= Position.X + Buttons[i].Position.X + Buttons[i].Size.Width &&
            IO.MousePos.Y >= Position.Y + Buttons[i].Position.Y + 25 && IO.MousePos.Y <= Position.Y + Buttons[i].Position.Y + 25 + Buttons[i].Size.Height) {
            if (!IO.WindowWantToDrag) {
                hover = true;

                if (IO.MouseDown[0]) {
                    over = true;
                }

                if (IO.MouseDown[0] && !WantToClickOnButton) {
                    WantToClickOnButton = true;
                }

                if (WantToClickOnButton && IO.MouseRelease[0]) {
                    WantToClickOnButton = false;
                    Buttons[i].handlerButton();
                }
            }
        }
        else if (IO.MouseRelease[0]) {
            WantToClickOnButton = false;
        }

        float textWidth = Graphics::Font::CalculateTextWidth(Buttons[i].Caption.c_str());
        float textX = Position.X + Buttons[i].Position.X + (Buttons[i].Size.Width - textWidth) / 2;
        float textY = Position.Y + Buttons[i].Position.Y + 25 + (Buttons[i].Size.Height - Graphics::Font::Height) / 2;

        Graphics::DrawFilledRect(Position.X + Buttons[i].Position.X, Position.Y + 25 + Buttons[i].Position.Y, Buttons[i].Size.Width, Buttons[i].Size.Height,
            over ? guiH_RGB({131, 160, 203}) : (hover ? guiH_RGB({147, 180, 228}) : guiH_RGB({164, 201, 254}))
        );
        Graphics::Font::RenderText(textX, textY, Buttons[i].Caption.c_str(), {27, 27, 29});
    }

    for (size_t i = 0; i < Checkboxes.size(); i++) {
        bool hover = false;
        bool over = false;

        if (IO.MousePos.X >= Position.X + Checkboxes[i].Position.X && IO.MousePos.X <= Position.X + Checkboxes[i].Position.X + 25 + Graphics::Font::CalculateTextWidth(Checkboxes[i].Caption.c_str()) &&
            IO.MousePos.Y >= Position.Y + Checkboxes[i].Position.Y + 25 && IO.MousePos.Y <= Position.Y + Checkboxes[i].Position.Y + 20 + 25) {
            hover = true;

            if (IO.MouseDown[0]) {
                over = true;
            }

            if (IO.MouseDown[0] && !WantToClickOnCheckbox) {
                WantToClickOnCheckbox = true;
            }

            if (WantToClickOnCheckbox && IO.MouseRelease[0]) {
                WantToClickOnCheckbox = false;
                Checkboxes[i].Checked = !Checkboxes[i].Checked;
                Checkboxes[i].handlerCheckbox(Checkboxes[i].Checked);
            }
        }
        else if (IO.MouseRelease[0]) {
            WantToClickOnCheckbox = false;
        }

        Graphics::DrawFilledRect(Position.X + Checkboxes[i].Position.X, Position.Y + 25 + Checkboxes[i].Position.Y, 20, 20,
            (Checkboxes[i].Checked ? guiH_RGB({164, 201, 254}) : guiH_RGB({67, 72, 78}))
        );

        if (Checkboxes[i].Checked) {
            float startX = Position.X + Checkboxes[i].Position.X + 5;
            float startY = Position.Y + 25 + Checkboxes[i].Position.Y + 10;
            float endX1 = startX + 3;
            float endY1 = startY + 5;
            float endX2 = startX + 8;
            float endY2 = startY - 5;

            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glColor3ub(27, 27, 29);
            glVertex2f(startX, startY);
            glVertex2f(endX1, endY1);
            glVertex2f(endX1, endY1);
            glVertex2f(endX2, endY2);
            glEnd();
        }

        Graphics::Font::RenderText(Position.X + Checkboxes[i].Position.X + 25, Position.Y + 25 + (Checkboxes[i].Position.Y), Checkboxes[i].Caption.c_str(), {255, 255, 255});
    }

    for (size_t i = 0; i < TextBoxes.size(); i++) {
        Graphics::DrawFilledRect(Position.X + TextBoxes[i].Position.X, Position.Y + 25 + TextBoxes[i].Position.Y, TextBoxes[i].Size.Width, TextBoxes[i].Size.Height, {52, 52, 52});
        
        if (TextBoxes[i].Focused) {
            float cursorX = Position.X + TextBoxes[i].Position.X + Graphics::Font::CalculateTextWidth(TextBoxes[i].Text.c_str()) + 3;
            float cursorY = Position.Y + 25 + TextBoxes[i].Position.Y + 1;

            Graphics::DrawFilledRect(cursorX, cursorY, 1, TextBoxes[i].Size.Height - 2, {255, 255, 255});
        }

        if (TextBoxes[i].Text.empty() && !TextBoxes[i].TextHint.empty() && !TextBoxes[i].Focused) {
            Graphics::Font::RenderText(Position.X + TextBoxes[i].Position.X + 3, Position.Y + 25 + TextBoxes[i].Position.Y + 1, TextBoxes[i].TextHint.c_str(), {128, 128, 128});
        }

        if (!TextBoxes[i].Text.empty()) {
            Graphics::Font::RenderText(Position.X + TextBoxes[i].Position.X + 3, Position.Y + 25 + TextBoxes[i].Position.Y + 1, TextBoxes[i].Text.c_str(), {255, 255, 255});
        }

        if (TextBoxes[i].Focused && IO.WantToWriteTextboxChar) {
            if (IO.WantTextboxChar == '\b') {
                if (!TextBoxes[i].Text.empty()) {
                    TextBoxes[i].Text.pop_back();
                }
            } else if (IO.WantTextboxChar >= 32 && IO.WantTextboxChar <= 126) {
                TextBoxes[i].Text += IO.WantTextboxChar;
            }

            TextBoxes[i].handleTextboxEdited(TextBoxes[i].Text);

            IO.WantToWriteTextboxChar = false;
            IO.WantTextboxChar = NULL;
        } 

        if (IO.MousePos.X >= Position.X + TextBoxes[i].Position.X && IO.MousePos.X <= Position.X + TextBoxes[i].Position.X + TextBoxes[i].Size.Width &&
            IO.MousePos.Y >= Position.Y + 25 + TextBoxes[i].Position.Y && IO.MousePos.Y <= Position.Y + 25 + TextBoxes[i].Position.Y + TextBoxes[i].Size.Height) {
            if (IO.MouseDown[0]) {
                for (auto& tb : TextBoxes) {
                    tb.Focused = false;
                }
                TextBoxes[i].Focused = true;
                IO.TextboxFocused = true;
            }
        } else {
            if (IO.MouseDown[0]) {
                if (TextBoxes[i].Focused) {
                    TextBoxes[i].Focused = false;
                    IO.TextboxFocused = false;
                }
            }
        }
    }
}
