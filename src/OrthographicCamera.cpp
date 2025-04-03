#include "OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Veloxr;

OrthographicCamera::OrthographicCamera(float aspectRatio, float nearPlane, float farPlane, float zoomLevel)
    : _aspectRatio(aspectRatio), _near(nearPlane), _far(farPlane), _zoomLevel(zoomLevel),
      _position(glm::vec2(0.0f)), _projectionMatrix(glm::mat4(1.0f)), 
      _viewMatrix(glm::mat4(1.0f)), _viewProjectionMatrix(glm::mat4(1.0f))
{
    recalculateProjection();
    recalculateView();
}

void OrthographicCamera::init(float aspectRatio, float nearPlane, float farPlane, float zoomLevel) {
    _aspectRatio = aspectRatio;
    _near = nearPlane;
    _far = farPlane;
    _projectionMatrix = glm::mat4(1.0f);
    _viewMatrix = glm::mat4(1.0f);
    _viewProjectionMatrix = glm::mat4(1.0f);
    _zoomLevel = zoomLevel;

    recalculateProjection();
    recalculateView();

}
#include <iostream>
void OrthographicCamera::recalculateProjection() {
    float halfWidth = _aspectRatio * 0.5f * _zoomLevel;
    float halfHeight = 0.5f * _zoomLevel;
    float left = -halfWidth;
    float right = halfWidth;
    float bottom = -halfHeight;
    float top = halfHeight;
    std::cout << "[CAMERA] left, right, top, bottom: [" << left << ", " << right << ", " << top << ", " << bottom << "]\n";
    std::cout << "[CAMERA] aspectRatio, _zoomLevel: [" << _aspectRatio << ", " << _zoomLevel << "]\n";
    _projectionMatrix = glm::ortho(left, right, bottom, top, _near, _far);
    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
}

void OrthographicCamera::recalculateView() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(-_position, 0.0f));
    _viewMatrix = transform;
    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
}

void OrthographicCamera::zoom(float zoomDelta) {
    _zoomLevel += zoomDelta;
    if (_zoomLevel < 0.01f) _zoomLevel = 0.01f;
    recalculateProjection();
}

void OrthographicCamera::pan(const glm::vec2& panDelta) {
    _position += panDelta;
    recalculateView();
}

const glm::mat4& OrthographicCamera::getProjectionMatrix() const {
    return _projectionMatrix;
}

const glm::mat4& OrthographicCamera::getViewMatrix() const {
    return _viewMatrix;
}

const glm::mat4& OrthographicCamera::getViewProjectionMatrix() const {
    return _viewProjectionMatrix;
}

float OrthographicCamera::getZoomLevel() const {
    return _zoomLevel;
}

glm::vec2 OrthographicCamera::getPosition() const {
    return _position;
}

void OrthographicCamera::setZoomLevel(float zoomLevel) {
    _zoomLevel = zoomLevel;
    recalculateProjection();
}

void OrthographicCamera::addToZoom(float delta) {
    _zoomLevel += delta;
    _zoomLevel = std::min(_zoomLevel, static_cast<decltype(_zoomLevel)>(10));
    _zoomLevel = std::max(_zoomLevel, static_cast<decltype(_zoomLevel)>(0.0001));
    recalculateProjection();
}
void OrthographicCamera::setPosition(const glm::vec2& pos) {
    _position = pos;
    recalculateView();
}

void OrthographicCamera::translate(const glm::vec2& delta){
    _position.x += delta.x;
    _position.y += delta.y;
    recalculateView();
}

