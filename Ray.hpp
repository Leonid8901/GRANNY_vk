#pragma once
#include <glm/glm.hpp>

/**
 * @brief Математическое описание математического луча в трехмерном евклидовом пространстве.
 */
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    inline Ray(const glm::vec3& orig, const glm::vec3& dir)
        : origin(orig), direction(glm::normalize(dir)) {
    }

    /**
     * @brief Вычисление параметрической точки вектора на заданной дистанции t.
     */
    inline glm::vec3 pointAt(float t) const {
        return origin + (t * direction);
    }
};

struct HitRecord {
    glm::vec3 hitPoint;
    glm::vec3 normal;
    float t;
    bool hit;
};

/**
 * @brief Утилитарный класс для базовых расчетов линейной оптики и сканирования плоскостей.
 */
class Raycaster {
public:
    /**
     * @brief Расчет вектора зеркального отскока луча (Закон отражения света).
     */
    static inline Ray reflectRay(const Ray& incomingRay, const HitRecord& hit) {
        glm::vec3 reflectedDir = glm::reflect(incomingRay.direction, hit.normal);
        return Ray(hit.hitPoint + hit.normal * 0.001f, reflectedDir); // Микросмещение против застревания в текстуре
    }

    /**
     * @brief Расчет точки пересечения луча с бесконечной горизонтальной плоскостью (Floor Plane).
     */
    static inline HitRecord testFloorIntersection(const Ray& ray) {
        HitRecord record{};
        record.hit = false;

        if (ray.direction.y >= 0.0f) return record;

        // Координатный расчет шага луча по оси ординат
        float t = -ray.origin.y / ray.direction.y;

        if (t >= 0.0f) {
            record.hit = true;
            record.t = t;
            record.hitPoint = ray.pointAt(t);
            record.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        return record;
    }
};
