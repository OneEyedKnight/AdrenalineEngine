/*
    buffers.cpp
    Adrenaline Engine

    This file has the declarations of buffers.h
*/

#include "buffers.h"

void Adren::Buffers::createModelBuffers(std::vector<Model>& models, VkCommandPool& commandPool) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (auto& model : models) {
        indices.insert(indices.end(), model.indices.begin(), model.indices.end());
        vertices.insert(vertices.end(), model.vertices.begin(), model.vertices.end());
    }

    VkDeviceSize vBufferSize = sizeof(vertices[0]) * vertices.size();
    Buffer vStaging;
    createBuffer(allocator, vBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vStaging, VMA_MEMORY_USAGE_CPU_TO_GPU);

    void* vData;
    vmaMapMemory(allocator, vStaging.memory, &vData);
    memcpy(vData, vertices.data(), (size_t)vBufferSize);
    vmaUnmapMemory(allocator, vStaging.memory);

    createBuffer(allocator, vBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex, VMA_MEMORY_USAGE_GPU_ONLY);

    copyBuffer(vStaging.buffer, vertex.buffer, vBufferSize, commandPool);

    vmaDestroyBuffer(allocator, vStaging.buffer, vStaging.memory);

    VkDeviceSize iBufferSize = sizeof(indices[0]) * indices.size();

    Buffer iStaging;
    createBuffer(allocator, iBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, iStaging, VMA_MEMORY_USAGE_CPU_TO_GPU);

    void* iData;
    vmaMapMemory(allocator, iStaging.memory, &iData);
    memcpy(iData, indices.data(), (size_t)iBufferSize);
    vmaUnmapMemory(allocator, iStaging.memory);

    createBuffer(allocator, iBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index, VMA_MEMORY_USAGE_GPU_ONLY);
    copyBuffer(iStaging.buffer, index.buffer, iBufferSize, commandPool);

    vmaDestroyBuffer(allocator, iStaging.buffer, iStaging.memory);
}

void Adren::Buffers::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool) {
    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);
}

void Adren::Buffers::createBuffer(VmaAllocator& allocator, VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer, VmaMemoryUsage vmaUsage) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = vmaUsage;
    vmaAllocInfo.preferredFlags = properties;

    vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo, &buffer.buffer, &buffer.memory, nullptr);
}

void Adren::Buffers::createUniformBuffers(std::vector<VkImage>& images, std::vector<Model>& models) {
    UniformBufferObject ubo;
    VkDeviceSize uBufferSize = sizeof(ubo);

    createBuffer(allocator, uBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniform, VMA_MEMORY_USAGE_GPU_ONLY);
    vmaMapMemory(allocator, uniform.memory, &uniform.mapped);
    memcpy(uniform.mapped, &ubo, uBufferSize);

    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkDeviceSize minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    dynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    VkDeviceSize duBufferSize = models.size() * dynamicAlignment;
    uboData.model = (glm::mat4*)Adren::Tools::alignedAlloc(duBufferSize, dynamicAlignment);
    assert(uboData.model);

    dynamicUniform.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        createBuffer(allocator, duBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, dynamicUniform[i], VMA_MEMORY_USAGE_CPU_TO_GPU);
        vmaMapMemory(allocator, dynamicUniform[i].memory, &dynamicUniform[i].mapped);
        memcpy(dynamicUniform[i].mapped, uboData.model, dynamicAlignment * models.size());
    }
}

void Adren::Buffers::updateUniformBuffer(Camera& camera, VkExtent2D& extent) {
    if (camera.toggled) {
        UniformBufferObject ubo{};
        ubo.view = glm::lookAt(camera.pos, camera.pos + camera.front, camera.up);
        ubo.proj = glm::perspective(glm::radians(110.0f), (float)extent.width / (float)extent.height, 0.1f, 10000.0f);
        ubo.proj[1][1] *= -1;
        memcpy(uniform.mapped, &ubo, sizeof(ubo));
    }
}

void Adren::Buffers::updateDynamicUniformBuffer(uint32_t index, std::vector<Model>& models) {
    for (uint32_t i = 0; i < models.size(); i++) {
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboData.model + (i * dynamicAlignment)));
        *modelMat = glm::translate(glm::mat4(1.0f), models[i].position);

        if (models[i].rotationAngle != 0.0f)
            *modelMat = glm::rotate(*modelMat, glm::radians(models[i].rotationAngle), models[i].rotationAxis);

        if (models[i].scale != 0.0f)
            *modelMat = glm::scale(*modelMat, glm::vec3(models[i].scale));

    }

    memcpy(dynamicUniform[index].mapped, uboData.model, dynamicAlignment * models.size());
}

void Adren::Buffers::cleanup() {
    if (uboData.model) { Adren::Tools::alignedFree(uboData.model); }

    vmaDestroyBuffer(allocator, vertex.buffer, vertex.memory);
    vmaDestroyBuffer(allocator, index.buffer, index.memory);

    vmaDestroyBuffer(allocator, uniform.buffer, uniform.memory);
    vmaUnmapMemory(allocator, uniform.memory);

    for (Buffer& buffer : dynamicUniform) {
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.memory);
        vmaUnmapMemory(allocator, buffer.memory);
    }
}