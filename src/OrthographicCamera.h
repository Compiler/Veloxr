#pragma once
#include <glm/glm.hpp>
namespace Veloxr {

class OrthographicCamera
{
public:
    OrthographicCamera(){}
    OrthographicCamera(float aspectRatio, float nearPlane = -1.0f, float farPlane = 1.0f, float zoomLevel = 1.0f);

    void init(float aspectRatio, float nearPlane = -1.0f, float farPlane = 1.0f, float zoomLevel = 1.0f);

    void recalculateProjection();
    void recalculateView();

    void zoom(float zoomDelta);
    void pan(const glm::vec2& panDelta);

    const glm::mat4& getProjectionMatrix() const;
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getViewProjectionMatrix() const;

    float getZoomLevel() const;
    glm::vec2 getPosition() const;

    void setZoomLevel(float zoomLevel);
    void setPosition(const glm::vec2& pos);

private:
    float _aspectRatio;
    float _near;
    float _far;
    float _zoomLevel;
    glm::vec2 _position;
    glm::mat4 _projectionMatrix;
    glm::mat4 _viewMatrix;
    glm::mat4 _viewProjectionMatrix;
};
}

