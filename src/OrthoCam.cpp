
#include "OrthoCam.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Veloxr;

OrthoCam::OrthoCam(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    : _position(0.0f, 0.0f, 0.0f), rotation(0.0f), _zoomLevel(1.0f),
      _left(left), _right(right), _bottom(bottom), _top(top), _nearPlane(nearPlane), _farPlane(farPlane) {
    recalcProjection();
    recalcView();
}

void OrthoCam::init(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _position = glm::vec3(0.0f);
    rotation = 0.0f;
    _zoomLevel = 1.0f;
    recalcProjection();
    recalcView();
}

void OrthoCam::setPosition(const glm::vec3& pos) {
    _position = pos;
    recalcView();
}

void OrthoCam::setPosition(const glm::vec2& pos) {
    _position = glm::vec3(pos.x, pos.y, _position.z);
    recalcView();
}

void OrthoCam::translate(const glm::vec2& delta) {
    _position.x += delta.x;
    _position.y += delta.y;
    recalcView();
}

void OrthoCam::setRotation(float rot) {
    rotation = rot;
    recalcView();
}

void OrthoCam::setProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    recalcProjection();
}

const glm::mat4& OrthoCam::getProjectionMatrix() const {
    return projectionMatrix;
}

const glm::mat4& OrthoCam::getViewMatrix() const {
    return viewMatrix;
}

glm::mat4 OrthoCam::getViewProjectionMatrix() const {
    return projectionMatrix * viewMatrix;
}

void OrthoCam::setZoomLevel(float zoomLevel) {
    if (zoomLevel < 0.00001f) {
        zoomLevel = 0.00001f;
    }
    _zoomLevel = zoomLevel;
    recalcProjection();
}

void OrthoCam::addToZoom(float delta) {
    setZoomLevel(_zoomLevel + delta);
}

void OrthoCam::recalcView() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), _position)
        * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1));
    viewMatrix = glm::inverse(transform);
}

void OrthoCam::recalcProjection() {
    float l = _left / _zoomLevel;
    float r = _right / _zoomLevel;
    float b = _bottom / _zoomLevel;
    float t = _top / _zoomLevel;
    projectionMatrix = glm::ortho(l, r, b, t, _nearPlane, _farPlane);
}

