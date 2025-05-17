#ifndef ORTHOGRAPHIC_CAMERA_H
#define ORTHOGRAPHIC_CAMERA_H

#include <glm/glm.hpp>
namespace Veloxr {

    class OrthoCam {
        public:
            OrthoCam()=default;
            OrthoCam(float left, float right, float bottom, float top, float nearPlane, float farPlane);
            // Initialize the camera with certain bounds. Ignore near and far plane. Set to 0, 1.
            void init(float left, float right, float bottom, float top, float nearPlane, float farPlane);
            // Position in world coordinates. Initialized cameras from renderer will make world coordinates the pixels of the texture.
            void setPosition(const glm::vec3& pos);
            void setPosition(const glm::vec2& pos);
            // Translate as a delta in world coordinates.
            void translate(const glm::vec2& delta);
            // Rotate the viewport.
            void setRotation(float rot);
            // setProjection will reinitialize the camera with a different world space.
            void setProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane);
            // Used in renderer to tell the GPU how to interpret our camera.
            const glm::mat4& getProjectionMatrix() const;
            const glm::mat4& getViewMatrix() const;
            glm::mat4 getViewProjectionMatrix() const;

            // Set the zoom level, relative to world coordinates.
            void setZoomLevel(float zoomLevel);
            inline const float getZoomLevel() const { return _zoomLevel;}
            // Change zoom based on a delta.
            void addToZoom(float delta);
            // Center of world space.
            void zoomToCenter(float zoomDelta);
            // Center of anchor point.
            void zoomCentered(const glm::vec2& anchorPoint, float zoomDelta);
            // Fit to bounds.
            void fitViewport(float left, float right, float bottom, float top);

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

