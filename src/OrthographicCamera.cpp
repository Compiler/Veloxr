#include "OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

namespace Veloxr {

OrthographicCamera::OrthographicCamera()
  : _texWidth(1.0f), _texHeight(1.0f),
    _windowWidth(1.0f), _windowHeight(1.0f),
    _near(-1.0f), _far(1.0f),
    _zoomLevel(1.0f),
    _position(0.0f),
    _projectionMatrix(1.0f),
    _viewMatrix(1.0f),
    _viewProjectionMatrix(1.0f) {
    recalculateProjection();
    recalculateView();
}

OrthographicCamera::OrthographicCamera(float texWidth, float texHeight,
                                       float windowWidth, float windowHeight,
                                       float nearPlane, float farPlane,
                                       float zoomLevel)
  : _texWidth(texWidth), _texHeight(texHeight),
    _windowWidth(windowWidth), _windowHeight(windowHeight),
    _near(nearPlane), _far(farPlane),
    _zoomLevel(zoomLevel),
    _position(0.0f),
    _projectionMatrix(1.0f),
    _viewMatrix(1.0f),
    _viewProjectionMatrix(1.0f) {
    recalculateProjection();
    recalculateView();
}

void OrthographicCamera::init(float texWidth, float texHeight,
                              float windowWidth, float windowHeight,
                              float nearPlane, float farPlane,
                              float zoomLevel) {
    _texWidth = texWidth;
    _texHeight = texHeight;
    _windowWidth = windowWidth;
    _windowHeight = windowHeight;
    _near = nearPlane;
    _far = farPlane;
    _zoomLevel = zoomLevel;
    _position = glm::vec2(0.0f);

    _projectionMatrix = glm::mat4(1.0f);
    _viewMatrix = glm::mat4(1.0f);
    _viewProjectionMatrix = glm::mat4(1.0f);

    recalculateProjection();
    recalculateView();
}

void OrthographicCamera::setTextureSize(float texWidth, float texHeight) {
    _texWidth = texWidth;
    _texHeight = texHeight;
    recalculateProjection();
}

void OrthographicCamera::setWindowSize(float windowWidth, float windowHeight) {
    _windowWidth = windowWidth;
    _windowHeight = windowHeight;
    recalculateProjection();
}

void OrthographicCamera::setZoomLevel(float zoomLevel) {
    if (zoomLevel < 0.00001f) {
        zoomLevel = 0.00001f;
    }
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

void OrthographicCamera::recalculateProjection() {
    if (_windowWidth <= 0.0f)  _windowWidth  = 1.0f;
    if (_windowHeight <= 0.0f) _windowHeight = 1.0f;

    float texAspect = _texWidth / _texHeight;
    float windowAspect = _windowWidth / _windowHeight;

    float halfWidth = 0.0f;
    float halfHeight = 0.0f;

    if (texAspect > windowAspect) {
        halfWidth  = (_texWidth * 0.5f) / _zoomLevel;
        halfHeight = halfWidth / texAspect;
    } else {
        halfHeight = (_texHeight * 0.5f) / _zoomLevel;
        halfWidth  = halfHeight * texAspect;
    }

    float left   = -halfWidth;
    float right  = +halfWidth;
    float bottom = -halfHeight;
    float top    = +halfHeight;

    _projectionMatrix = glm::ortho(left, right, bottom, top, _near, _far);

    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;

    std::cout << "[OrthographicCamera] texW=" << _texWidth
              << " texH=" << _texHeight
              << " windowW=" << _windowWidth
              << " windowH=" << _windowHeight
              << " zoom=" << _zoomLevel
              << " => left=" << left
              << " right=" << right
              << " bottom=" << bottom
              << " top=" << top << "\n";


}

void OrthographicCamera::recalculateView() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f),
                                         glm::vec3(-_position, 0.0f));

    _viewMatrix = transform;
    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
    std::cout
        << "Camera position= (" << _position.x 
        << ", " << _position.y << ")\n";

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

} 

