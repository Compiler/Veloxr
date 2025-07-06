#ifndef ORTHOGRAPHIC_CAMERA_H
#define ORTHOGRAPHIC_CAMERA_H

#include <glm/glm.hpp>
#include "VLogger.h"
namespace Veloxr {

    class OrthographicCamera {
        public:
            OrthographicCamera()=default;
            OrthographicCamera(float left, float right, float bottom, float top, float nearPlane, float farPlane);
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
            glm::vec3 getPosition() const;

            inline glm::vec4 getROI() const {
                return {
                    _left   / _zoomLevel,
                    _right  / _zoomLevel,
                    _top    / _zoomLevel,
                    _bottom / _zoomLevel
                };
            }

            float getWidth() const { return (_right - _left) / _zoomLevel; }
            float getHeight() const { return (_top - _bottom) / _zoomLevel; }

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

            inline void resetDirty() { _dirty = false; }
            inline bool const getDirty() const { return _dirty; }


        private:
            Veloxr::LLogger console{"[Veloxr] [Camera] "};
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
            bool _dirty {true};
    };
}

#endif

