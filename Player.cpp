#include "Player.h"
#include <iostream>

Player::Player(glm::vec3 startPosition, float speed)
    : m_position(startPosition), m_speed(speed), m_yaw(-90.0f), m_pitch(0.0f) {
    // Инициализация базовой ориентации в пространстве
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Player::update(float deltaTime, bool w, bool a, bool s, bool d) {
    // Расчет ортогонального вектора движения "Вправо" через Cross Product
    glm::vec3 right = glm::normalize(glm::cross(m_front, m_up));
    glm::vec3 moveDirection(0.0f);

    // Суперпозиция векторов ввода направления
    if (w) moveDirection += m_front;
    if (s) moveDirection -= m_front;
    if (a) moveDirection -= right;
    if (d) moveDirection += right;

    // Расчет линейного смещения с учетом нормализации скорости по диагонали
    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
        m_position += moveDirection * m_speed * deltaTime;
    }
}

void Player::rotateCamera(float xOffset, float yOffset) {
    float sensitivity = 0.1f; // Коэффициент чувствительности мыши
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;

    // Ограничение угла тангажа (Pitch) для предотвращения эффекта Gimbal Lock и переворота камеры
    if (m_pitch > 89.0f)  m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    // Перевод сферических координат углов Эйлера в декартов вектор направления взгляда
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
}
