#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

/**
 * @brief Структура ограничивающего объема (AABB Хитбокс) для статических плоскостей и пропсов.
 */
struct CollisionBox {
    std::string name;
    glm::vec3 min; // Минимальный базисный вектор объекта {X_min, Y_min, Z_min}
    glm::vec3 max; // Максимальный базисный вектор объекта {X_max, Y_max, Z_max}
};

/**
 * @brief Структура математического описания пересечения луча с полигональной геометрией.
 */
struct RayHit {
    bool hit;
    float distance;
    glm::vec3 point;
    glm::vec3 normal;
};

/**
 * @brief Подсистема просчета твердых столкновений и трассировки лучей в пространстве.
 */
class MapSystem {
public:
    std::vector<CollisionBox> walls;

    inline MapSystem() {
        generateLevel();
    }

    /**
     * @brief Генерация статической геометрической карты уровня (2 этаж).
     */
    inline void generateLevel() {
        walls.clear();

        // Параметры ограждающих конструкций (толщина стен — 0.2м, высота — 4.0м)
        walls.push_back({ "Задняя Стена Спальни", glm::vec3(-5.0f, 0.0f, -5.0f), glm::vec3(5.0f, 4.0f, -4.8f) });
        walls.push_back({ "Левая Стена Спальни",   glm::vec3(-5.0f, 0.0f, -5.0f), glm::vec3(-4.8f, 4.0f, 5.0f) });
        walls.push_back({ "Правая Стена Спальни",  glm::vec3(4.8f, 0.0f, -5.0f),  glm::vec3(5.0f, 4.0f, 5.0f) });

        // Межкомнатные перегородки с учетом технологических проемов дверей
        walls.push_back({ "Перегородка Левая",  glm::vec3(-5.0f, 0.0f, 4.8f),  glm::vec3(1.0f, 4.0f, 5.0f) });
        walls.push_back({ "Перегородка Правая", glm::vec3(3.0f, 0.0f, 4.8f),  glm::vec3(5.0f, 4.0f, 5.0f) });

        // Интегрированные статические объекты интерьера
        walls.push_back({ "Большой Шкаф", glm::vec3(-2.0f, 0.0f, -1.0f), glm::vec3(-1.0f, 2.5f, 1.0f) });
    }

    /**
     * @brief Алгоритмическая проверка коллизии сферы актора с AABB-структурами стен уровня.
     */
    inline bool checkPlayerCollision(const glm::vec3& playerPos, float playerRadius) {
        // Формирование динамического хитбокса вокруг координат ног персонажа
        glm::vec3 pMin = playerPos - glm::vec3(playerRadius, 1.7f, playerRadius);
        glm::vec3 pMax = playerPos + glm::vec3(playerRadius, 0.2f, playerRadius);

        for (const auto& wall : walls) {
            bool intersect = (pMin.x <= wall.max.x && pMax.x >= wall.min.x) &&
                (pMin.y <= wall.max.y && pMax.y >= wall.min.y) &&
                (pMin.z <= wall.max.z && pMax.z >= wall.min.z);
            if (intersect) return true;
        }
        return false;
    }

    /**
     * @brief Высокопроизводительная трассировка луча сквозь структуры объектов (Ray-AABB Box Intersection).
     */
    inline RayHit traceRay(const glm::vec3& origin, const glm::vec3& direction, float maxDist) {
        RayHit closestHit{ false, maxDist, glm::vec3(0.0f), glm::vec3(0.0f) };

        for (const auto& wall : walls) {
            float tmin = (wall.min.x - origin.x) / direction.x;
            float tmax = (wall.max.x - origin.x) / direction.x;
            if (tmin > tmax) std::swap(tmin, tmax);

            float tymin = (wall.min.y - origin.y) / direction.y;
            float tymax = (wall.max.y - origin.y) / direction.y;
            if (tymin > tymax) std::swap(tymin, tymax);

            if ((tmin > tymax) || (tymin > tmax)) continue;
            if (tymin > tmin) tmin = tymin;
            if (tymax < tmax) tmax = tymax;

            float tzmin = (wall.min.z - origin.z) / direction.z;
            float tzmax = (wall.max.z - origin.z) / direction.z;
            if (tzmin > tzmax) std::swap(tzmin, tzmax);

            if ((tmin > tzmax) || (tzmin > tmax)) continue;
            if (tzmin > tmin) tmin = tzmin;
            if (tzmax < tmax) tmax = tzmax;

            // Вычисление ближайшей скалярной точки пересечения в текущем полупространстве
            if (tmin > 0.0f && tmin < closestHit.distance) {
                closestHit.hit = true;
                closestHit.distance = tmin;
                closestHit.point = origin + (direction * tmin);
                closestHit.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Базовый вектор нормали плоскости
            }
        }
        return closestHit;
    }
};
