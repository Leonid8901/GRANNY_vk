#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

/**
 * @brief Класс-обёртка над WinAPI для управления системным окном ОС Windows.
 * Реализует паттерн RAII для автоматической регистрации класса и деструкции дескрипторов.
 */
class Window {
public:
    Window(int width, int height, const std::wstring& title);
    ~Window();

    // Запрет копирования ресурса окна для предотвращения дублирования дескрипторов (Handles)
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief Проверяет статус закрытия окна пользователем.
     * @return true, если отправлен сигнал WM_CLOSE.
     */
    bool shouldClose();

    /**
     * @brief Неблокирующий опрос очереди системных сообщений операционной системы Windows.
     */
    void pollEvents();

    // Инкапсулированные геттеры дескрипторов операционной системы для Vulkan WSI
    HWND getHWND() const { return m_hwnd; }
    HINSTANCE getHINSTANCE() const { return m_hInstance; }

private:
    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    bool m_shouldClose = false;

    /**
     * @brief Системная функция обратного вызова (Callback) для обработки оконных сообщений.
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
