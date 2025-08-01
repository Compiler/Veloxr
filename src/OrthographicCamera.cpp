
#include "OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <iostream>

using namespace Veloxr;

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    : _position(0.0f, 0.0f, 0.0f), rotation(0.0f), _zoomLevel(1.0f),
      _left(left), _right(right), _bottom(bottom), _top(top), _nearPlane(nearPlane), _farPlane(farPlane) {
    recalcProjection();
    recalcView();
}

void OrthographicCamera::init(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _position = glm::vec3(0.0f);
    rotation = 0.0f;
    _zoomLevel = 1.0f;
    console.logc2("init Updating camera bounds (l, r, t, b) => (", left, ", ", right, ", ", top, ", ", bottom, ")");
    recalcProjection();
    recalcView();
}

void OrthographicCamera::setPosition(const glm::vec3& pos) {
    _position = glm::vec3(pos.x, pos.y, pos.z);
    recalcView();
}

void OrthographicCamera::setPosition(const glm::vec2& pos) {
    _position = glm::vec3(pos.x, pos.y, _position.z);
    recalcView();
}

void OrthographicCamera::translate(const glm::vec2& delta) {
    _position.x += delta.x;
    _position.y += delta.y;
    recalcView();
}

void OrthographicCamera::setRotation(float rot) {
    rotation = rot;
    recalcView();
}

void OrthographicCamera::setProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    console.logc2("setProjection Updating camera bounds (l, r, t, b) => (", left, ", ", right, ", ", top, ", ", bottom, ")");
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    recalcProjection();
}

const glm::mat4& OrthographicCamera::getProjectionMatrix() const {
    return projectionMatrix;
}

glm::vec3 OrthographicCamera::getPosition() const {
    return _position;
}

const glm::mat4& OrthographicCamera::getViewMatrix() const {
    return viewMatrix;
}

glm::mat4 OrthographicCamera::getViewProjectionMatrix() const {
    return projectionMatrix * viewMatrix;
}

void OrthographicCamera::setZoomLevel(float zoomLevel) {
    if (zoomLevel < 0.00001f) {
        zoomLevel = 0.00001f;
    }
    _zoomLevel = zoomLevel;
    _dirty = true;
    recalcProjection();
}

void OrthographicCamera::addToZoom(float delta) {
    setZoomLevel(_zoomLevel + delta);
}

void OrthographicCamera::zoomToCenter(float zoomDelta) {
    // NDC center
    glm::vec4 centerClip = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // clip -> world
    glm::mat4 invVP = glm::inverse(getViewProjectionMatrix());
    glm::vec4 centerWorld = invVP * centerClip;

    // perspective divide 
    centerWorld /= centerWorld.w;

    zoomCentered(glm::vec2(centerWorld.x, centerWorld.y), zoomDelta);
}

glm::vec3 OrthographicCamera::worldToCamera(const glm::vec3& worldCoord) {
    glm::mat4 invViewProj = glm::inverse(this->getViewProjectionMatrix());
    glm::vec4 pos = invViewProj * glm::vec4(0, 0, 0, 1);
    return glm::vec3(pos) / pos.w; // perspective-divide this bish

}

void OrthographicCamera::zoomCentered(const glm::vec2& anchorPoint, float zoomDelta) {
    console.debug(__func__, ": ", anchorPoint.x, ", ", anchorPoint.y);
    float oldZoom = _zoomLevel;
    float newZoom = oldZoom + zoomDelta;
    if (newZoom < 0.00001f) {
        newZoom = 0.00001f;  
    }

    glm::mat4 oldVP = getViewProjectionMatrix();

    glm::vec4 oldClipPos = oldVP * glm::vec4(anchorPoint, 0.0f, 1.0f);

    setZoomLevel(newZoom);

    glm::mat4 newVP = getViewProjectionMatrix();
    glm::mat4 invNewVP = glm::inverse(newVP);

    glm::vec4 newWorldPos = invNewVP * oldClipPos;
    newWorldPos /= newWorldPos.w;

    glm::vec3 diff = glm::vec3(anchorPoint, 0.0f) - glm::vec3(newWorldPos);

    setPosition(_position + diff);
}

// Fit a viewport, ie a texture. Only do the math on zoom to maintain AR of camera.
void OrthographicCamera::fitViewport(float left, float right, float bottom, float top) {
    //if(left > right || top > bottom) return;
    float boxWidth  = right - left;
    float boxHeight = top - bottom;
    float camWidth  = _right - _left;
    float camHeight = _top - _bottom;

    float neededZoomW = camWidth  / boxWidth;
    float neededZoomH = camHeight / boxHeight;
    // Will maintain portrai / landscape
    float newZoom = glm::min(neededZoomW, neededZoomH);

    if (boxWidth <= 0.0f || boxHeight <= 0.0f) {
        return;
    }
    if (newZoom < 0.00001f) {
        newZoom = 0.00001f; 
    }

    setZoomLevel(newZoom);
}



void OrthographicCamera::recalcView() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), _position)
        * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1));
    viewMatrix = glm::inverse(transform);
    _dirty = true;
    console.log("[CAMERA] Position: ", _position.x, ", ", _position.y);
}

void OrthographicCamera::recalcProjection() {
    float l = _left / _zoomLevel;
    float r = _right / _zoomLevel;
    float b = _bottom / _zoomLevel;
    float t = _top / _zoomLevel;
    projectionMatrix = glm::ortho(l, r, b, t, _nearPlane, _farPlane);
    _dirty = true;
}

