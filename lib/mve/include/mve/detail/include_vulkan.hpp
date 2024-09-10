#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <vulkan/vulkan.hpp>

#define VMA_VULKAN_VERSION 1001000
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>