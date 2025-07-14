#pragma once

#include "Common.h"
#include "Vertex.h"
#include <memory>
namespace Veloxr {

    class VVShaderStageData {
    
        public:
            VVShaderStageData() = delete;
            VVShaderStageData(std::shared_ptr<VVDataPacket> dataPacket);

            void setVertices(std::vector<Veloxr::Vertex>& vertices);


        private:
            inline static LLogger console{"[Veloxr][VVShaderStageData] "}; 

            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;

            void createVertexBuffer(std::vector<Veloxr::Vertex>& vertices) ;
            std::shared_ptr<VVDataPacket> _data;
    };
}
