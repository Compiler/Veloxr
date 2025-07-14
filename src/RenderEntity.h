#pragma once
#include "Common.h"
#include "DataUtils.h"
#include "VVTexture.h"
#include "Vertex.h"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
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
            void setPosition(float x, float y){ _position.x = x; _position.y = y;};
            void setPosition(glm::vec3& pos) { _position = pos; };
            void setTextureBuffer(std::unique_ptr<Veloxr::VeloxrBuffer> buffer);
            void setTextureBuffer(std::shared_ptr<Veloxr::VeloxrBuffer> buffer);
            void setTextureBuffer(Veloxr::VeloxrBuffer& buffer);
            void setDataPacket(std::shared_ptr<VVDataPacket> dataPacket) { _texture.setDataPacket(dataPacket); }

            void destroy();

            inline const glm::vec3& getPosition() const { return _position; }
            inline const std::string& getName() const { return _name; }
            inline const bool isHidden () const { return _isHidden; }

            // Use these :| 
            [[nodiscard]] inline Veloxr::VVTexture& getVVTexture() { return _texture; }
            [[nodiscard]] inline const std::shared_ptr<Veloxr::VeloxrBuffer> getBuffer() const { return _textureBuffer; }
            [[nodiscard]] inline const std::vector<Veloxr::Vertex>& getVertices (){ 
                std::cout << "Getting " << _texture.getVertices().size() << " vertices.\n";
                return _texture.getVertices(); 
            }


        private:
            static int ENTITY_COUNT;
            glm::vec3 _position;
            std::string _name{""};
            std::shared_ptr<Veloxr::VeloxrBuffer> _textureBuffer;
            bool _isHidden{false};
            std::vector<Veloxr::Vertex> _vertices;
            Veloxr::VVTexture _texture;
    };
}
