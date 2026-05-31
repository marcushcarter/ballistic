#include "render_graph.h"
#include "pass.h"

static VkExtent2D ComputeExtent(const TransientImageDesc& d, VkExtent2D vp)
{
    if (d.sizing == TransientImageDesc::Sizing::Fixed) return { d.fixedWidth, d.fixedHeight };
    return { (uint32_t)(vp.width * d.widthScale), (uint32_t)(vp.height * d.heightScale) };
}
 
static bool HasWrite(VkAccessFlags2 a)
{
    constexpr VkAccessFlags2 W =
        VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT |
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_2_TRANSFER_WRITE_BIT | VK_ACCESS_2_HOST_WRITE_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    return (a & W) != 0;
}


void RenderGraph::Init(VkDevice d, VmaAllocator a) { device = d; vma = a; }
void RenderGraph::Shutdown() { heap.Shutdown(); }
void RenderGraph::SetViewport(VkExtent2D e) { viewportExtent = e; }

void RenderGraph::BeginFrame(uint64_t frameIdx, uint64_t completedFrameIdx)
{
    frameIndex = frameIdx;
    heap.Recycle(completedFrameIdx);
    Reset();
}

void RenderGraph::Reset()
{
    resources.clear();
    nameMap.clear();
    nodes.clear();
    plan.clear();
    currentSetup = nullptr;
    currentPassIdx = -1;
}

int32_t RenderGraph::Resolve(const char* name)
{
    auto it = nameMap.find(name);
    if (it == nameMap.end()) { LOG_ERROR("RenderGraph: resource '%s' not found (must be Imported first)", name); return -1; }
    return (int32_t)it->second;
}

ResourceHandle RenderGraph::RecordWrite(uint32_t resIdx, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access)
{
    Resource& res = resources[resIdx];
    res.versions.push_back({ currentPassIdx, 0 });
    ResourceHandle h{ resIdx, (uint16_t)(res.versions.size() - 1) };
    if (currentSetup) currentSetup->uses.push_back({ h, layout, stage, access, true });
    return h;
}

ResourceHandle RenderGraph::RecordRead(uint32_t resIdx, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access)
{
    Resource& res = resources[resIdx];
    uint16_t ver = (uint16_t)(res.versions.size() - 1);
    res.versions[ver].readRefCount++;
    ResourceHandle h{ resIdx, ver };
    if (currentSetup) currentSetup->uses.push_back({ h, layout, stage, access, false });
    return h;
}

ResourceHandle RenderGraph::ImportImage(const char* name, Image2D* image)
{
    auto it = nameMap.find(name);
    if (it != nameMap.end()) { resources[it->second].externalImage = image; return { it->second, 0 }; }
    Resource r{};
    r.kind = Kind::ExternalImage;
    r.name = name;
    r.externalImage = image;
    r.versions.push_back({});
    uint32_t idx = (uint32_t)resources.size();
    resources.push_back(std::move(r));
    nameMap[name] = idx;
    return { idx, 0 };
}

ResourceHandle RenderGraph::ImportBuffer(const char* name, Buffer* buffer)
{
    auto it = nameMap.find(name);
    if (it != nameMap.end()) { resources[it->second].externalBuffer = buffer; return { it->second, 0 }; }
    Resource r{};
    r.kind = Kind::ExternalBuffer;
    r.name = name;
    r.externalBuffer = buffer;
    r.versions.push_back({});
    uint32_t idx = (uint32_t)resources.size();
    resources.push_back(std::move(r));
    nameMap[name] = idx;
    return { idx, 0 };
}

ResourceHandle RenderGraph::CreateImage(const char* name, const TransientImageDesc& desc, VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access)
{
    if (nameMap.count(name)) { LOG_ERROR("RenderGraph: resource '%s' already declared", name); return {}; }
    Resource r{};
    r.kind = Kind::TransientImage;
    r.name = name;
    r.imageDesc = desc;
    r.versions.push_back({});
    uint32_t idx = (uint32_t)resources.size();
    resources.push_back(std::move(r));
    nameMap[name] = idx;
    return RecordWrite(idx, layout, stage, access);    
}

ResourceHandle RenderGraph::CreateBuffer(const char* name, const TransientBufferDesc& desc, VkPipelineStageFlags2 stage, VkAccessFlags2 access)
{
    if (nameMap.count(name)) { LOG_ERROR("RenderGraph: resource '%s' already declared", name); return {}; }
    Resource r{};
    r.kind = Kind::TransientBuffer;
    r.name = name;
    r.bufferDesc = desc;
    r.versions.push_back({});
    uint32_t idx = (uint32_t)resources.size();
    resources.push_back(std::move(r));
    nameMap[name] = idx;
    return RecordWrite(idx, VK_IMAGE_LAYOUT_UNDEFINED, stage, access);
}

ResourceHandle RenderGraph::ReadImage(const char* n, VkImageLayout l, VkPipelineStageFlags2 s, VkAccessFlags2 a)
{
    int32_t r = Resolve(n);
    return r < 0 ? ResourceHandle{} : RecordRead((uint32_t)r, l, s, a);
}

ResourceHandle RenderGraph::WriteImage(const char* n, VkImageLayout l, VkPipelineStageFlags2 s, VkAccessFlags2 a)
{
    int32_t r = Resolve(n);
    return r < 0 ? ResourceHandle{} : RecordWrite((uint32_t)r, l, s, a);
}

ResourceHandle RenderGraph::ReadBuffer(const char* n, VkPipelineStageFlags2 s, VkAccessFlags2 a)
{
    int32_t r = Resolve(n);
    return r < 0 ? ResourceHandle{} : RecordRead((uint32_t)r, VK_IMAGE_LAYOUT_UNDEFINED, s, a);
}

ResourceHandle RenderGraph::WriteBuffer(const char* n, VkPipelineStageFlags2 s, VkAccessFlags2 a)
{
    int32_t r = Resolve(n);
    return r < 0 ? ResourceHandle{} : RecordWrite((uint32_t)r, VK_IMAGE_LAYOUT_UNDEFINED, s, a);
}

void RenderGraph::AddPass(std::unique_ptr<Pass> pass)
{
    nodes.emplace_back();
    currentPassIdx = (int32_t)nodes.size() - 1;
    Node& node = nodes.back();
    node.pass = std::move(pass);
    currentSetup = &node;
    node.pass->Setup(*this);
    currentSetup = nullptr;
    currentPassIdx = -1;
}

void RenderGraph::Compile()
{
    for (Resource& r : resources)
        if ((r.kind == Kind::ExternalImage || r.kind == Kind::ExternalBuffer) && r.versions.size() > 1)
            r.versions.back().readRefCount += 1;

    for (Node& n : nodes) {
        n.active = true;
        n.refCount = 0;
        for (Use& u : n.uses) if (u.isWrite) n.refCount++;
    }

    std::vector<std::pair<uint32_t, uint16_t>> dead;
    for (uint32_t ri = 0; ri < resources.size(); ++ri) {
        Resource& r = resources[ri];
        for (uint16_t v = 1; v < (uint16_t)r.versions.size(); ++v)
            if (r.versions[v].readRefCount == 0) dead.push_back({ ri, v });
    }
    
    while (!dead.empty()) {
        std::pair<uint32_t, uint16_t> d = dead.back();
        dead.pop_back();
        int32_t pp = resources[d.first].versions[d.second].producerPass;
        if (pp < 0) continue;
        Node& pn = nodes[pp];
        if (pn.refCount == 0) continue;
        if (--pn.refCount == 0) {
            pn.active = false;
            for (Use& u : pn.uses) {
                if (u.isWrite) continue;
                Resource::Version& ver = resources[u.handle.resource].versions[u.handle.version];
                if (ver.readRefCount > 0 && --ver.readRefCount == 0)
                    dead.push_back({ u.handle.resource, u.handle.version });
            }
        }
    }
    
    plan.clear();
    plan.reserve(nodes.size());
    for (Node& n : nodes)
        if (n.active) { Step s{};
        s.node = &n;
        plan.push_back(std::move(s));
    }

    for (uint32_t e = 0; e < plan.size(); e++) {
        for (Use& u : plan[e].node->uses) {
            Resource& r = resources[u.handle.resource];
            r.referenced = true;
            if ((int32_t)e < r.firstPass) r.firstPass = (int32_t)e;
            if ((int32_t)e > r.lastPass) r.lastPass = (int32_t)e;
        }
    }

    std::vector<TransientRequest> requests;
    requests.reserve(resources.size());

    for (uint32_t ri = 0; ri < resources.size(); ++ri) {
        Resource& r = resources[ri];
        if (!r.referenced) continue;

        if (r.kind == Kind::TransientImage) {
            r.resolvedExtent = ComputeExtent(r.imageDesc, viewportExtent);
            if (r.resolvedExtent.width == 0 || r.resolvedExtent.height == 0)
                LOG_ERROR("RenderGraph: transient '%s' resolved to zero extent (viewport set?)", r.name ? r.name : "?");

            TransientRequest req{};
            req.kind = TransientRequest::Kind::Image;
            req.imageDesc = r.imageDesc;
            req.imageDesc.debugName = r.name;
            req.extent = r.resolvedExtent;
            req.firstPass = (uint32_t)r.firstPass;
            req.lastPass = (uint32_t)r.lastPass;
            r.heapSlot = (int32_t)requests.size();
            requests.push_back(req);

        } else if (r.kind == Kind::TransientBuffer) {

            TransientRequest req{};
            req.kind = TransientRequest::Kind::Buffer;
            req.bufferDesc = r.bufferDesc;
            req.bufferDesc.debugName = r.name;
            req.firstPass = (uint32_t)r.firstPass;
            req.lastPass = (uint32_t)r.lastPass;
            r.heapSlot = (int32_t)requests.size();
            requests.push_back(req);

        }
    }
    heap.Realize(requests, frameIndex);

    struct Track {
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 prodStage  = 0;
        VkAccessFlags2 prodAccess = 0;
        VkPipelineStageFlags2 flushed = 0;
        VkPipelineStageFlags2 readStages = 0;
        VkAccessFlags2 readAccess = 0;
    };
    std::vector<Track> track(resources.size());

    for (uint32_t ri = 0; ri < resources.size(); ri++) {
        Resource& r = resources[ri];
        Track& t = track[ri];
        if (r.kind == Kind::ExternalImage && r.externalImage) {
            t.layout = r.externalImage->layout;
            t.prodStage = (VkPipelineStageFlags2)r.externalImage->stage;
            t.prodAccess = (VkAccessFlags2)r.externalImage->access;
        } else if (r.kind == Kind::ExternalBuffer && r.externalBuffer) {
            t.prodStage = (VkPipelineStageFlags2)r.externalBuffer->stage;
            t.prodAccess = (VkAccessFlags2)r.externalBuffer->access;
        } else {
            t.layout = VK_IMAGE_LAYOUT_UNDEFINED;
            t.prodStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            t.prodAccess = VK_ACCESS_2_MEMORY_WRITE_BIT;
        }
    }

    for (Step& step : plan) {
        for (Use& u : step.node->uses) {
            Resource& r = resources[u.handle.resource];
            Track& t = track[u.handle.resource];
            const bool isImage = (r.kind == Kind::TransientImage || r.kind == Kind::ExternalImage);

            if (u.isWrite) {

                const VkPipelineStageFlags2 src = t.prodStage | t.readStages;
                const VkAccessFlags2 srcAcc = t.prodAccess | t.readAccess;
                const bool layoutChange = isImage && (t.layout != u.layout);
                
                if (layoutChange || src != 0) {
                    Barrier b{};
                    b.handle = u.handle;
                    b.isImage = isImage;
                    b.oldLayout = t.layout;
                    b.newLayout = u.layout;
                    b.srcStage = src;
                    b.srcAccess = srcAcc;
                    b.dstStage = u.stage;
                    b.dstAccess = u.access;
                    step.barriers.push_back(b);
                }

                t.layout = u.layout;
                t.prodStage = u.stage;
                t.prodAccess = u.access;
                t.flushed = 0;
                t.readStages = 0;
                t.readAccess = 0;
            
            } else {
                
                const bool layoutChange = isImage && (t.layout != u.layout);
                const bool needVis = HasWrite(t.prodAccess) && ((u.stage & ~t.flushed) != 0);
                
                if (layoutChange || needVis) {
                    Barrier b{};
                    b.handle = u.handle;
                    b.isImage = isImage;
                    b.oldLayout = t.layout;
                    b.newLayout = u.layout;
                    b.srcStage = t.prodStage;
                    b.srcAccess = t.prodAccess;
                    b.dstStage = u.stage;
                    b.dstAccess = u.access;
                    step.barriers.push_back(b);
                    t.layout = u.layout;
                    t.flushed |= u.stage;
                }

                t.readStages |= u.stage;
                t.readAccess |= u.access;
            }
        }
    }

    for (uint32_t ri = 0; ri < resources.size(); ri++) {
        Resource& r = resources[ri];
        Track& t = track[ri];
        const VkPipelineStageFlags2 fStage = t.prodStage | t.readStages;
        const VkAccessFlags2 fAccess= t.prodAccess | t.readAccess;

        if (r.kind == Kind::ExternalImage && r.externalImage) {
            r.externalImage->layout = t.layout;
            r.externalImage->stage = (VkPipelineStageFlags)fStage;
            r.externalImage->access = (VkAccessFlags)fAccess;
        } else if (r.kind == Kind::ExternalBuffer && r.externalBuffer) {
            r.externalBuffer->stage = (VkPipelineStageFlags)fStage;
            r.externalBuffer->access = (VkAccessFlags)fAccess;
        }
    }
}

void RenderGraph::Execute(VkCommandBuffer cmd)
{
    std::vector<VkImageMemoryBarrier2> imgBarriers;
    std::vector<VkBufferMemoryBarrier2> bufBarriers;

    for (auto& step : plan) {
        if (!step.barriers.empty()) {
            imgBarriers.clear();
            bufBarriers.clear();

            for (auto& b : step.barriers) {
                if (b.isImage) {
                    VkImage img = GetVkImage(b.handle);
                    if (!img) continue;
                    VkImageMemoryBarrier2 vb{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
                    vb.srcStageMask = b.srcStage;
                    vb.dstStageMask = b.dstStage;
                    vb.srcAccessMask = b.srcAccess;
                    vb.dstAccessMask = b.dstAccess;
                    vb.oldLayout = b.oldLayout;
                    vb.newLayout = b.newLayout;
                    vb.image = img;
                    vb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    vb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    vb.subresourceRange.aspectMask = GetImageAspect(b.handle);
                    vb.subresourceRange.baseMipLevel = 0;
                    vb.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
                    vb.subresourceRange.baseArrayLayer = 0;
                    vb.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
                    imgBarriers.push_back(vb);
                } else {
                    VkBuffer buf = GetVkBuffer(b.handle);
                    if (!buf) continue;
                    VkBufferMemoryBarrier2 vb{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
                    vb.srcStageMask = b.srcStage;
                    vb.srcAccessMask = b.srcAccess;
                    vb.dstStageMask = b.dstStage;
                    vb.dstAccessMask = b.dstAccess;
                    vb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    vb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    vb.buffer = buf;
                    vb.offset = 0;
                    vb.size = VK_WHOLE_SIZE;
                    bufBarriers.push_back(vb);
                }
            }

            if (!imgBarriers.empty() || !bufBarriers.empty()) {
                VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
                dep.imageMemoryBarrierCount = (uint32_t)imgBarriers.size();
                dep.pImageMemoryBarriers = imgBarriers.data();
                dep.bufferMemoryBarrierCount = (uint32_t)bufBarriers.size();
                dep.pBufferMemoryBarriers = bufBarriers.data();
                vkCmdPipelineBarrier2(cmd, &dep);
            }
        }
        step.node->pass->Execute(cmd, *this);
    }
}

VkImageView RenderGraph::GetImageView(ResourceHandle h)
{
    Resource& r = resources[h.resource];
    if (r.kind == Kind::TransientImage) return heap.GetImage((uint32_t)r.heapSlot).view;
    if (r.kind == Kind::ExternalImage && r.externalImage) return r.externalImage->GetView();
    return VK_NULL_HANDLE;
}
 
VkImage RenderGraph::GetVkImage(ResourceHandle h)
{
    Resource& r = resources[h.resource];
    if (r.kind == Kind::TransientImage) return heap.GetImage((uint32_t)r.heapSlot).image;
    if (r.kind == Kind::ExternalImage && r.externalImage) return r.externalImage->GetImage();
    return VK_NULL_HANDLE;
}
 
VkExtent2D RenderGraph::GetImageExtent(ResourceHandle h)
{
    Resource& r = resources[h.resource];
    if (r.kind == Kind::TransientImage) return heap.GetImage((uint32_t)r.heapSlot).extent;
    if (r.kind == Kind::ExternalImage && r.externalImage) return r.externalImage->extent;
    return {};
}
 
VkImageAspectFlags RenderGraph::GetImageAspect(ResourceHandle h)
{
    Resource& r = resources[h.resource];
    if (r.kind == Kind::TransientImage) return heap.GetImage((uint32_t)r.heapSlot).aspect;
    if (r.kind == Kind::ExternalImage && r.externalImage) return r.externalImage->aspect;
    return VK_IMAGE_ASPECT_COLOR_BIT;
}
 
VkBuffer RenderGraph::GetVkBuffer(ResourceHandle h)
{
    Resource& r = resources[h.resource];
    if (r.kind == Kind::TransientBuffer) return heap.GetBuffer((uint32_t)r.heapSlot).buffer;
    if (r.kind == Kind::ExternalBuffer && r.externalBuffer) return r.externalBuffer->Get();
    return VK_NULL_HANDLE;
}
