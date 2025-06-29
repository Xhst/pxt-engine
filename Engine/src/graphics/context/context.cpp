#include "graphics/context/context.hpp"

namespace PXTEngine {

    Context::Context(Window& window)
        : m_window(window),
        m_instance{ "PXT Engine" },
        m_surface{ m_window, m_instance },
        m_physicalDevice{ m_instance, m_surface },
        m_device{ m_window, m_instance, m_surface, m_physicalDevice } {

		createCommandPool();
    }

	Context::~Context() {
        vkDestroyCommandPool(m_device.getDevice(), m_commandPool, nullptr);
	}
    

    void Context::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = m_physicalDevice.findQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_device.getDevice(), &poolInfo, nullptr,
            &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    uint32_t Context::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice.getDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

	void Context::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                               VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device.getDevice(), &bufferInfo, nullptr, &buffer) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device.getDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		VkMemoryAllocateFlagsInfo flagsInfo{};
		if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
			// Chain the flagsInfo structure to the allocation info
			allocInfo.pNext = &flagsInfo;
		}

        if (vkAllocateMemory(m_device.getDevice(), &allocInfo, nullptr, &bufferMemory) !=
            VK_SUCCESS) {
            throw std::runtime_error(
                "failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_device.getDevice(), buffer, bufferMemory, 0);
    }

    VkCommandBuffer Context::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device.getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

	// TODO: add more error handling
    void Context::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device.getGraphicsQueue());

        vkFreeCommandBuffers(m_device.getDevice(), m_commandPool, 1, &commandBuffer);
    }


    void Context::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        // TODO: we can try to implement a memory barrier to avoid waiting the copy
        //       to be finished before we can start rendering again.
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Context::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);

        endSingleTimeCommands(commandBuffer);
    }

    void Context::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties,
                                      VkImage &image, VkDeviceMemory &imageMemory) {
        if (vkCreateImage(m_device.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device.getDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(m_device.getDevice(), image, imageMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

	VkImageView Context::createImageView(const VkImageViewCreateInfo& viewInfo) {
		
        VkImageView imageView;
		if (vkCreateImageView(m_device.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VkSampler Context::createSampler(const VkSamplerCreateInfo& samplerInfo) {
		VkSampler sampler;
		if (vkCreateSampler(m_device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		return sampler;
	}

    void Context::createShaderModuleFromSpirV(const std::vector<char>& code, VkShaderModule* shaderModule) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(m_device.getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
    }

    void Context::createShaderModuleFromSourceBinary(const std::vector<uint32_t>& binary, VkShaderModule* shaderModule) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = binary.size() * sizeof(uint32_t);
        createInfo.pCode = binary.data();

        if (vkCreateShaderModule(m_device.getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
    }

	VkFormat Context::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                          VkFormatFeatureFlags features) {

        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice.getDevice(), format,
                &props);

            if ((tiling == VK_IMAGE_TILING_LINEAR  && (props.linearTilingFeatures  & features) == features) ||
                (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

	VkFormat Context::findDepthFormat() {
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	bool Context::getSupportedDepthFormat(VkFormat* depthFormat)
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
		std::vector<VkFormat> formatList = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : formatList)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice.getDevice(), format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				*depthFormat = format;
				return true;
			}
		}

		return false;
	}
} 