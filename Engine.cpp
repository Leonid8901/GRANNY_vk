#include "Engine.h"
#include <iostream>
#include "Ray.hpp"

#pragma comment(linker, "/SUBSYSTEM:console")

Engine::Engine() : m_currentRoomName("") {
    m_window = std::make_unique<Window>(1280, 720, L"Granny: Code from Scratch");
    initVulkan();

    // Инициализация игровых сущностей и подсистем (Паттерн инверсии зависимостей)
    m_player = std::make_unique<Player>(glm::vec3(0.0f, 1.7f, 0.0f), 4.5f);
    m_granny = std::make_unique<Granny>(glm::vec3(7.0f, 0.0f, 0.0f), 2.2f);
    m_mapSystem = std::make_unique<MapSystem>();
}

Engine::~Engine() {
    cleanup();
}

void Engine::initVulkan() {
    m_vulkan = std::make_unique<VulkanContext>(m_window->getHWND(), m_window->getHINSTANCE());
}

void Engine::mainLoop() {
    std::cout << "[Core] Игровой цикл успешно запущен.\n";
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    float fpsTimer = 0.0f;
    int frameCount = 0;

    ShowCursor(FALSE); // Скрытие системного курсора ОС

    while (!m_window->shouldClose()) {
        m_window->pollEvents();

        // --- Обработка системы Паузы (ESC) с защитой от дребезга ---
        bool escPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        if (escPressed && !m_escWasPressed) {
            m_isPaused = !m_isPaused;
            ShowCursor(m_isPaused ? TRUE : FALSE);
            if (m_isPaused) std::cout << "[Core] Симуляция приостановлена.\n";
            else m_lastFrameTime = std::chrono::high_resolution_clock::now(); // Предотвращение скачка deltaTime
        }
        m_escWasPressed = escPressed;

        // --- Расчет физического времени (DeltaTime) ---
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - m_lastFrameTime).count();
        m_lastFrameTime = currentFrameTime;

        if (m_isPaused) deltaTime = 0.0f; // Остановка кинематических расчетов

        // --- Ввод и обработка мыши (Ось обзора) ---
        if (!m_isPaused) {
            POINT mousePos;
            GetCursorPos(&mousePos);
            int centerX = 1280 / 2;
            int centerY = 720 / 2;

            float xOffset = static_cast<float>(mousePos.x - centerX);
            float yOffset = static_cast<float>(centerY - mousePos.y);

            SetCursorPos(centerX, centerY); // Аппаратная фиксация курсора
            if (xOffset != 0.0f || yOffset != 0.0f) {
                m_player->rotateCamera(xOffset, yOffset);
            }
        }

        // --- Обработка перемещения персонажа (WASD + Коллизии) ---
        bool w = !m_isPaused && (GetAsyncKeyState('W') & 0x8000) != 0;
        bool s = !m_isPaused && (GetAsyncKeyState('S') & 0x8000) != 0;
        bool a = !m_isPaused && (GetAsyncKeyState('A') & 0x8000) != 0;
        bool d = !m_isPaused && (GetAsyncKeyState('D') & 0x8000) != 0;

        glm::vec3 oldPosition = m_player->getPosition();
        m_player->update(deltaTime, w, a, s, d);

        // Расчет твердых коллизий AABB
        if (m_mapSystem->checkPlayerCollision(m_player->getPosition(), 0.3f)) {
            m_player->setPosition(oldPosition); // Откат вектора перемещения при коллизии
        }

        // --- Триггеры пространственных зон ---
        std::string newRoom = m_houseMap.getRoomOfObject(m_player->getPosition());
        if (newRoom != m_currentRoomName) {
            m_currentRoomName = newRoom;
            std::cout << "[Zone] Игрок переместился в сектор: " << m_currentRoomName << "\n";
        }

        // --- Обновление систем ИИ ---
        m_granny->update(deltaTime, m_player->getPosition(), m_houseMap.waypoints);

        // --- Генерация звуковых эмиттеров (Кнопка F) ---
        if (!m_isPaused && (GetAsyncKeyState('F') & 0x8000)) {
            std::cout << "[Audio] Зафиксирован акустический импульс в секторе: " << m_currentRoomName << "\n";
            m_granny->hearNoise(m_player->getPosition());
            Sleep(200);
        }

        // --- Отладка Raycasting (Пробел) ---
        if (!m_isPaused && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            RayHit hit = m_mapSystem->traceRay(m_player->getPosition(), m_player->getFront(), 100.0f);
            if (hit.hit) {
                std::cout << "[Ray] Коллизия луча взгляда на дистанции: " << hit.distance << " м.\n";
            }
            Sleep(150);
        }

        // --- Рендеринг кадра графическим конвейером Vulkan ---
        m_vulkan->drawFrame();

        // --- Метрики производительности (Профайлинг) ---
        fpsTimer += (m_isPaused ? 0.016f : deltaTime);
        frameCount++;
        if (fpsTimer >= 1.0f) {
            std::string state = m_isPaused ? "PAUSE" : m_granny->getStateString();
            std::cout << "[Profile] FPS: " << frameCount << " | AI State: " << state << "\n";
            frameCount = 0;
            fpsTimer = 0.0f;
        }
    }
    ShowCursor(TRUE);
}

int main() {
    SetConsoleOutputCP(65001); // Фикс кракозябр
    try {
        Engine app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Engine::run() { mainLoop(); }
void Engine::cleanup() { std::cout << "[Core] Деструкция систем движка.\n"; }
