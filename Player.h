#pragma once
#include <glm/glm.hpp>

/**
 * @brief Класс кинематического контроллера персонажа.
 * Отвечает за расчет тригонометрии углов обзора и трансформацию координат перемещения.
 */
class Player {
public:
    Player(glm::vec3 startPosition, float speed);
    ~Player() = default;

    /**
     * @brief Обновление позиционирования персонажа в пространстве (WASD).
     */
    void update(float deltaTime, bool w, bool a, bool s, bool d);

    /**
     * @brief Перерасчет 3D-вектора взгляда на основе двухосевого смещения курсора мыши.
     */
    void rotateCamera(float xOffset, float yOffset);

    // Геттеры и сеттеры векторов состояния
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }
    void setPosition(const glm::vec3& pos) { m_position = pos; }

private:
    glm::vec3 m_position; // Радиус-вектор положения персонажа {X, Y, Z}
    glm::vec3 m_front;    // Нормализованный вектор направления взгляда вперед
    glm::vec3 m_up;       // Базовый ортонормальный вектор направления "Вверх"

    float m_speed;        // Линейная скорость перемещения актора
    float m_yaw;          // Поворот вокруг вертикальной оси ординат (Горизонт)
    float m_pitch;        // Наклон вокруг поперечной оси абсцисс (Вертикаль)
};
