#pragma once
#include "Window.h"
#include <memory>
#include "VulkanContext.h"
#include <chrono> 
#include "Player.h"
#include "HouseAI.hpp" 
#include "MapSystem.hpp"

/**
 * @brief Главный управляющий класс игрового движка (Ядро системы).
 * Реализует паттерн Фасад для координации подсистем ввода, логики и рендеринга.
 */
class Engine {
public:
    Engine();
    ~Engine();

    /**
     * @brief Точка входа для запуска главного игрового цикла симуляции.
     */
    void run();

private:
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Системные флаги конечного автомата состояний игры
    bool m_isPaused = false;
    bool m_escWasPressed = false;

    // Изолированные аппаратные и системные модули
    std::unique_ptr<Window> m_window;
    std::unique_ptr<VulkanContext> m_vulkan;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;

    // Сущности игрового пространства и акторы (Dependency Injection)
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Granny> m_granny;
    std::unique_ptr<MapSystem> m_mapSystem;

    HouseMap m_houseMap;
    std::string m_currentRoomName;
};
