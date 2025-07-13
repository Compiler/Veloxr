#pragma once
#include "DataUtils.h"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <string>

namespace Veloxr {

    class RenderEntity {
        public:
            RenderEntity();
            RenderEntity(glm::vec3 position);
            RenderEntity(glm::vec2 position);
            RenderEntity(float x, float y);


            void setIsHidden(bool isHidden) { _isHidden = isHidden; }
            void setName(const std::string& name);
            void setPosition(float x, float y){ _position.x = x; _position.y = y;};
            void setPosition(glm::vec3& pos) { _position = pos; };
            void setTextureBuffer(std::unique_ptr<Veloxr::VeloxrBuffer> buffer);
            void setTextureBuffer(std::shared_ptr<Veloxr::VeloxrBuffer> buffer);
            void setTextureFilepath(std::string& filepath);


            void destroy();


            [[nodiscard]] inline const glm::vec3& getPosition() const { return _position; }
            [[nodiscard]] inline const std::string& getName() const { return _name; }
            [[nodiscard]] inline const std::shared_ptr<Veloxr::VeloxrBuffer> getBuffer() const { return _textureBuffer; }
            [[nodiscard]] inline const bool isHidden () const { return _isHidden; }




        private:
            static int ENTITY_COUNT;
            glm::vec3 _position;
            std::string _name{""};
            std::string _filepath{""};
            std::shared_ptr<Veloxr::VeloxrBuffer> _textureBuffer;
            bool _isHidden{false};

    };
}
