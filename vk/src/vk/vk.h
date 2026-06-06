#pragma once

#include <vk/core/instance.h>
#include <vk/core/debug_messenger.h>
#include <vk/core/surface.h>
#include <vk/core/physical_device.h>
#include <vk/core/queue.h>
#include <vk/core/device.h>
#include <vk/core/swapchain.h>

#include <vk/sync/semaphore.h>
#include <vk/sync/command_pool.h>
#include <vk/sync/command_buffer.h>
#include <vk/sync/fence.h>

#include <vk/descriptor/descriptor_pool.h>
#include <vk/descriptor/descriptor_set_layout.h>
#include <vk/descriptor/descriptor_set.h>

#include <vk/pipeline/shader.h>
#include <vk/pipeline/pipeline_layout.h>
#include <vk/pipeline/pipeline_cache.h>
#include <vk/pipeline/pipeline.h>
#include <vk/pipeline/compute_pipeline.h>
#include <vk/pipeline/graphics_pipeline.h>

#include <vk/buffer/buffer.h>
#include <vk/buffer/frame_ring.h>

#include <vk/image/image_2d.h>
#include <vk/image/image_view.h>
#include <vk/image/sampler.h>

#include <vk/heap/transient_resource.h>
#include <vk/heap/transient_heap.h>
#include <vk/heap/bindless_heap.h>

#include <vk/allocator/allocator.h>
#include <vk/allocator/allocator_pool.h>

#include <vk/pass/render_pass.h>
#include <vk/pass/framebuffer.h>
#include <vk/pass/transition_set.h>

#include <vk/misc/input_layout.h>
#include <vk/misc/utils.h>