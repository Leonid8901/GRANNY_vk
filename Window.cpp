#include "Window.h"

Window::Window(int width, int height, const std::wstring& title) {
    m_hInstance = GetModuleHandle(nullptr);

    // Регистрация структуры класса окна в подсистеме Windows
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"GrannyEngineWindowClass";

    RegisterClassExW(&wc);

    // Вычисление корректных геометрических размеров окна с учетом системных рамок и заголовка
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    // Инстанцирование дескриптора окна (HWND)
    m_hwnd = CreateWindowExW(
        0, wc.lpszClassName, title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_hInstance, this // Проброс указателя 'this' для привязки к Callback
    );

    ShowWindow(m_hwnd, SW_SHOW);
}

Window::~Window() {
    if (m_hwnd) DestroyWindow(m_hwnd);
    UnregisterClassW(L"GrannyEngineWindowClass", m_hInstance);
}

bool Window::shouldClose() {
    return m_shouldClose;
}

void Window::pollEvents() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* window = nullptr;

    // Паттерн Thunking: связывание статического Си-интерфейса WinAPI с контекстом класса C++
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else {
        window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (window && uMsg == WM_CLOSE) {
        window->m_shouldClose = true;
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
