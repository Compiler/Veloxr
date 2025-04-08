#pragma once
#include <glm/glm.hpp>

namespace Veloxr {

class OrthographicCamera {
public:
    OrthographicCamera();
    OrthographicCamera(float texWidth, float texHeight,
                       float windowWidth, float windowHeight,
                       float nearPlane = -1.0f, float farPlane = 1.0f,
                       float zoomLevel = 1.0f);

    void init(float texWidth, float texHeight,
              float windowWidth, float windowHeight,
              float nearPlane = -1.0f, float farPlane = 1.0f,
              float zoomLevel = 1.0f);

    void setTextureSize(float texWidth, float texHeight);

    void setWindowSize(float windowWidth, float windowHeight);

    void setZoomLevel(float zoomLevel);
    void addToZoom(float delta);

    void setPosition(const glm::vec2& pos);
    void translate(const glm::vec2& delta);

    void incrementTexHeight(){_texHeight += 500; recalculateProjection(); recalculateView();}
    void incrementTexWidth(){_texWidth += 500; recalculateProjection(); recalculateView();}

    float getZoomLevel() const;
    glm::vec2 getPosition() const;

    const glm::mat4& getProjectionMatrix() const;
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getViewProjectionMatrix() const;

private:
    void recalculateProjection();
    void recalculateView();

private:
    float _texWidth, _texHeight;
    float _windowWidth, _windowHeight;
    float _near, _far;
    float _zoomLevel;

    glm::vec2 _position; 

    glm::mat4 _projectionMatrix;
    glm::mat4 _viewMatrix;
    glm::mat4 _viewProjectionMatrix;
};

} 

