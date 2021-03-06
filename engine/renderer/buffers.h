/*
	buffers.h
	Adrenaline Engine

	This file has declarations of the buffers used in the engine.
*/

#pragma once
#include "types.h"
#include "devices.h"
#include "camera.h"
#include "model.h"
#include "tools.h"

namespace Adren {
class Buffers {
public:
	Buffers(Devices& devices) : devices(devices) {}

	void createModelBuffers(std::vector<Model>& models, VkCommandPool& commandPool);
	void createUniformBuffers(std::vector<VkImage>& images, std::vector<Model>& models);
	void updateUniformBuffer(Camera& camera, VkExtent2D& extent);
	void updateDynamicUniformBuffer(std::vector<Model>& models);
	void createBuffer(VmaAllocator& allocator, VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer, VmaMemoryUsage vmaUsage);
	void cleanup();

	Buffer vertex;
	Buffer index;
	Buffer uniform;
	Buffer dynamicUniform;
	UboData uboData;
private:
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool);

	Devices& devices;
	VkDevice& device = devices.device;
	VkPhysicalDevice& gpu = devices.gpu;
	VkQueue& graphicsQueue = devices.graphicsQueue;
	VmaAllocator& allocator = devices.allocator;
};
}