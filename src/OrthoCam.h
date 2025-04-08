#ifndef ORTHOGRAPHIC_CAMERA_H
#define ORTHOGRAPHIC_CAMERA_H

#include <glm/glm.hpp>
namespace Veloxr {
class OrthoCam {
public:
    OrthoCam()=default;
    OrthoCam(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void init(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void setPosition(const glm::vec3& pos);
    void setPosition(const glm::vec2& pos);
    void translate(const glm::vec2& delta);
    void setRotation(float rot);
    void setProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    const glm::mat4& getProjectionMatrix() const;
    const glm::mat4& getViewMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;
    void setZoomLevel(float zoomLevel);
    inline const float getZoomLevel() const { return _zoomLevel;}
    void addToZoom(float delta);
private:
    void recalcView();
    void recalcProjection();
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 _position;
    float rotation;
    float _zoomLevel = 1.0f;
    float _left;
    float _right;
    float _bottom;
    float _top;
    float _nearPlane;
    float _farPlane;
};
}

#endif

