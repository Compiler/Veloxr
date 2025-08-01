#pragma once
#include "Common.h"
#include "DataUtils.h"
#include "VVTexture.h"
#include "Vertex.h"
#include "UniqueOrderedNumber.h"
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <queue>
#include <string>

namespace Veloxr {

    class RenderEntity {
        public:
            RenderEntity();
            RenderEntity(std::shared_ptr<VVDataPacket> dataPacket);

            
            void appendVertex(Veloxr::Vertex&& vertex) {
                _vertices.push_back(std::move(vertex));
            }

            void setIsHidden(bool isHidden) { _isHidden = isHidden; }
            void setName(const std::string& name);
            void setPosition(float x, float y);
            void setPosition(glm::vec3& pos);
            void setTextureBuffer(std::unique_ptr<Veloxr::VeloxrBuffer> buffer);
            void setTextureBuffer(std::shared_ptr<Veloxr::VeloxrBuffer> buffer);
            void setTextureBuffer(Veloxr::VeloxrBuffer& buffer);
            void setDataPacket(std::shared_ptr<VVDataPacket> dataPacket) { _texture.setDataPacket(dataPacket); }
            void setResolution(glm::vec2 resolution) {_resolution = resolution;}

            void destroy();

            inline const glm::vec3& getPosition() const { return _position; }
            inline const glm::vec2 getResolution() const { 
                if(_resolution.x != 0 && _resolution.y != 0) return _resolution;
                const auto& bounding = _texture.getBoundingBox();
                uint32_t width = bounding.z - bounding.x;
                uint32_t height = bounding.w - bounding.y;
                return {width, height};
            }

            inline const glm::vec2 getCenterPos() const { 
                uint32_t width = getResolution().x;
                uint32_t height = getResolution().y;
                return {_position.x + width / 2.0f, _position.y + width / 2.0f};
            }
            inline const std::string& getName() const { return _name; }
            inline const bool isHidden () const { return _isHidden; }
            inline const int getUID () const { return _entityNumber; }

            // Copy to modify position
            const std::vector<Veloxr::Vertex> getVertices ();

            // Use these :| 
            [[nodiscard]] inline Veloxr::VVTexture& getVVTexture() { return _texture; }
            [[nodiscard]] inline const std::shared_ptr<Veloxr::VeloxrBuffer> getBuffer() const { return _textureBuffer; }

        private:
            static OrderedNumberFactory _entitySlots;

            glm::vec3 _position{0, 0, 0};
            glm::vec2 _resolution{0, 0};
            std::string _name{""};
            bool _isHidden{false};
            int _entityNumber;

            std::shared_ptr<Veloxr::VeloxrBuffer> _textureBuffer;
            std::vector<Veloxr::Vertex> _vertices;
            Veloxr::VVTexture _texture;
    };
}
