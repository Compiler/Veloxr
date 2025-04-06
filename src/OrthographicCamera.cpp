#include "OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

using namespace Veloxr;

OrthographicCamera::OrthographicCamera()
    : _windowAspectRatio(1.0f),
      _imageAspectRatio(1.0f),
      _near(-1.0f),
      _far(1.0f),
      _zoomLevel(1.0f),
      _position(0.0f, 0.0f),
      _projectionMatrix(1.0f),
      _viewMatrix(1.0f),
      _viewProjectionMatrix(1.0f)
{
}

OrthographicCamera::OrthographicCamera(float windowAspectRatio, float imageAspectRatio, float nearPlane, float farPlane, float zoomLevel)
    : _windowAspectRatio(windowAspectRatio),
      _imageAspectRatio(imageAspectRatio),
      _near(nearPlane),
      _far(farPlane),
      _zoomLevel(zoomLevel),
      _position(0.0f, 0.0f),
      _projectionMatrix(1.0f),
      _viewMatrix(1.0f),
      _viewProjectionMatrix(1.0f)
{
    recalculateProjection();
    recalculateView();
}

void OrthographicCamera::init(float windowAspectRatio, float imageAspectRatio, float nearPlane, float farPlane, float zoomLevel) {
    _windowAspectRatio = windowAspectRatio;
    _imageAspectRatio  = imageAspectRatio;
    _near = nearPlane;
    _far  = farPlane;
    _zoomLevel = zoomLevel;
    _position = glm::vec2(0.0f);
    _projectionMatrix = glm::mat4(1.0f);
    _viewMatrix = glm::mat4(1.0f);
    _viewProjectionMatrix = glm::mat4(1.0f);
    recalculateProjection();
    recalculateView();
}

void OrthographicCamera::setWindowAspectRatio(float windowAspectRatio) {
    _windowAspectRatio = windowAspectRatio;
    recalculateProjection();
}

void OrthographicCamera::setImageAspectRatio(float imageAspectRatio) {
    _imageAspectRatio = imageAspectRatio;
    recalculateProjection();
}

void OrthographicCamera::recalculateProjection() {
    if (_windowAspectRatio <= 0.0f || _imageAspectRatio <= 0.0f) {
        _projectionMatrix = glm::ortho(-1.f, 1.f, -1.f, 1.f, _near, _far);
        _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
        return;
    }
    float windowAR = _windowAspectRatio;
    float imageAR  = _imageAspectRatio;
    float halfWidth;
    float halfHeight;
    if (windowAR >= imageAR) {
        halfWidth  = 0.5f * _zoomLevel;
        halfHeight = halfWidth / imageAR;
    } else {
        halfHeight = 0.5f * _zoomLevel;
        halfWidth  = halfHeight * imageAR;
    }
    float left   = -halfWidth;
    float right  =  halfWidth;
    float bottom = -halfHeight;
    float top    =  halfHeight;
    std::cout << "[CAMERA] windowAR=" << windowAR << " imageAR=" << imageAR
        << " => left=" << left << " right=" << right
        << " top=" << top << " bottom=" << bottom << "\n";
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
    if (zoomLevel < 0.0001f) zoomLevel = 0.0001f;
    _zoomLevel = zoomLevel;
    recalculateProjection();
}

void OrthographicCamera::addToZoom(float delta) {
    setZoomLevel(_zoomLevel + delta);
}

void OrthographicCamera::setPosition(const glm::vec2& pos) {
    _position = pos;
    recalculateView();
}

void OrthographicCamera::translate(const glm::vec2& delta) {
    _position += delta;
    recalculateView();
}

