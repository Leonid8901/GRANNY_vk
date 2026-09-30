#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

/**
 * @brief Перечисление состояний конечного автомата (FSM) искусственного интеллекта.
 */
enum class GrannyState {
    Patrolling, // Патрулирование по узлам навигационной сетки
    Resting,    // Фиксированная задержка на точке (предиктивный анализ маршрута)
    Chasing     // Преследование цели в непрерывном пространстве (векторная математика)
};

/**
 * @brief Структура логической зоны (комнаты) для триггеров навигации.
 */
struct Room {
    std::string name;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    inline bool contains(const glm::vec3& point) const {
        return (point.x >= minBounds.x && point.x <= maxBounds.x) &&
            (point.y >= minBounds.y && point.y <= maxBounds.y) &&
            (point.z >= minBounds.z && point.z <= maxBounds.z);
    }
};

/**
 * @brief Класс управления глобальной навигационной сеткой (Waypoints).
 */
class HouseMap {
public:
    std::vector<Room> rooms;
    std::vector<glm::vec3> waypoints;

    inline HouseMap() {
        // Инициализация статической геометрии уровней (2 этаж)
        rooms.push_back({ "Спальня", glm::vec3(-5.0f, 0.0f, -5.0f), glm::vec3(5.0f, 4.0f, 5.0f) });
        rooms.push_back({ "Главный Коридор", glm::vec3(5.0f, 0.0f, -2.0f), glm::vec3(15.0f, 4.0f, 2.0f) });

        // Размещение опорных точек для глобального поиска пути ИИ
        waypoints.push_back(glm::vec3(-2.0f, 0.0f, -2.0f));
        waypoints.push_back(glm::vec3(2.0f, 0.0f, 2.0f));
        waypoints.push_back(glm::vec3(8.0f, 0.0f, 0.0f));
    }

    inline std::string getRoomOfObject(const glm::vec3& pos) {
        for (const auto& room : rooms) {
            if (room.contains(pos)) return room.name;
        }
        return "Вне дома";
    }
};

/**
 * @brief Класс контроллера ИИ противника. Реализует комбинированный поиск пути.
 */
class Granny {
public:
    inline Granny(glm::vec3 startPos, float speed)
        : m_position(startPos), m_speed(speed), m_state(GrannyState::Patrolling),
        m_targetIdx(0), m_stateTimer(0.0f), m_noisePos(0.0f), m_hasNoise(false) {
    }

    inline void update(float deltaTime, const glm::vec3& playerPos, const std::vector<glm::vec3>& wps) {
        float distToPlayer = glm::distance(m_position, playerPos);

        // Детерминированный триггер переключения в состояние погони
        if (distToPlayer < 5.0f && m_state != GrannyState::Chasing) {
            m_state = GrannyState::Chasing;
            std::cout << "[AI] Режим погони активирован.\n";
        }

        switch (m_state) {
        case GrannyState::Patrolling: {
            if (wps.empty()) return;
            glm::vec3 target = m_hasNoise ? m_noisePos : wps[m_targetIdx];
            glm::vec3 dir = target - m_position;
            float dist = glm::length(dir);

            if (dist > 0.2f) {
                m_position += glm::normalize(dir) * m_speed * deltaTime;
            }
            else {
                if (m_hasNoise) m_hasNoise = false;

                // Переход в состояние покоя/кэширования предиктивных путей
                m_state = GrannyState::Resting;
                m_stateTimer = 2.5f;
                std::cout << "[AI] Достигнут целевой локатор. Анализ маршрутов...\n";

                if (!m_hasNoise && !wps.empty()) {
                    m_targetIdx = (m_targetIdx + 1) % wps.size();
                }
            }
            break;
        }
        case GrannyState::Resting: {
            m_stateTimer -= deltaTime;
            if (m_stateTimer <= 0.0f) {
                m_state = GrannyState::Patrolling;
            }
            break;
        }
        case GrannyState::Chasing: {
            // Свободное преследование по вектору направления (кинематика)
            glm::vec3 dir = playerPos - m_position;
            m_position += glm::normalize(dir) * (m_speed * 1.5f) * deltaTime;

            if (distToPlayer > 10.0f) {
                m_state = GrannyState::Patrolling;
                std::cout << "[AI] Потеря видимости цели. Возврат к патрулированию.\n";
            }
            break;
        }
        }
    }

    inline void hearNoise(const glm::vec3& noisePos) {
        if (m_state == GrannyState::Chasing) return;
        m_noisePos = noisePos;
        m_hasNoise = true;
        m_state = GrannyState::Patrolling;
    }

    inline glm::vec3 getPosition() const { return m_position; }
    inline std::string getStateString() const {
        if (m_state == GrannyState::Patrolling) return "Патрулирование";
        if (m_state == GrannyState::Resting) return "Предиктивный расчет";
        return "Перехват цели";
    }

private:
    glm::vec3 m_position;
    float m_speed;
    GrannyState m_state;
    size_t m_targetIdx;
    float m_stateTimer;
    glm::vec3 m_noisePos;
    bool m_hasNoise;
};
