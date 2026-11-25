/*

 QEMU VMware Super Video Graphics Array 2 [SVGA-II]

 Copyright (c) 2026 QEMU VMVGA (https://github.com/qemus/qemu-vmvga)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*/

static bool vmsvga3d_state_context_define(struct vmsvga_state_s *s,
                                             uint32_t cid)
{
    struct vmsvga3d_state_s *state;
    VMSVGA3DContext *context;
    uint32_t i;

    if (cid >= SVGA3D_MAX_CONTEXT_IDS) {
        return false;
    }

    context = g_try_new0(VMSVGA3DContext, 1);
    if (context == NULL) {
        return false;
    }

    state = vmsvga3d_state_ensure(s);
    if (state == NULL) {
        g_free(context);
        return false;
    }

    context->cid = cid;
    context->legacy_full_replay = true;
    context->legacy_pipeline_dirty = VMSVGA3D_LEGACY_PIPELINE_DIRTY_ALL;
    for (i = 0; i < SVGA3D_RT_MAX; i++) {
        context->render_targets[i].sid = SVGA3D_INVALID_ID;
    }

    for (i = 0; i < SVGA3D_NUM_SHADERTYPE_PREDX; i++) {
        context->bound_shader[i] = SVGA3D_INVALID_ID;
    }

    if (state->active_legacy_context_id == cid) {
        state->active_legacy_context_id = SVGA3D_INVALID_ID;
    }
    vmsvga3d_context_free(state, state->contexts[cid]);

    state->contexts[cid] = context;
    return true;
}

static bool vmsvga3d_state_context_destroy(struct vmsvga_state_s *s,
                                              uint32_t cid)
{
    struct vmsvga3d_state_s *state = s->svga3d;

    if (state == NULL || cid >= SVGA3D_MAX_CONTEXT_IDS) {
        return false;
    }

    if (state->active_legacy_context_id == cid) {
        state->active_legacy_context_id = SVGA3D_INVALID_ID;
    }
    vmsvga3d_context_free(state, state->contexts[cid]);

    state->contexts[cid] = NULL;
    return true;
}

#if defined(__GNUC__)
#define VMSVGA3D_DX_STATE_UNUSED __attribute__((unused))
#else
#define VMSVGA3D_DX_STATE_UNUSED
#endif

static void vmsvga3d_state_dx_context_init_shadow(
    SVGADXContextMobFormat *shadow)
{
    uint32_t i;
    uint32_t j;

    memset(shadow, 0xff, sizeof(*shadow));

    shadow->inputAssembly.layoutId = SVGA3D_INVALID_ID;

    for (i = 0; i < SVGA3D_DX_MAX_VERTEXBUFFERS; i++) {
        shadow->inputAssembly.vertexBuffers[i].bufferId = SVGA3D_INVALID_ID;
        shadow->inputAssembly.vertexBuffers[i].stride = 0;
        shadow->inputAssembly.vertexBuffers[i].offset = 0;
    }

    shadow->inputAssembly.indexBufferSid = SVGA3D_INVALID_ID;
    shadow->inputAssembly.indexBufferOffset = 0;
    shadow->inputAssembly.indexBufferFormat = SVGA3D_FORMAT_INVALID;
    shadow->inputAssembly.topology = SVGA3D_PRIMITIVE_INVALID;

    shadow->renderState.blendStateId = SVGA3D_INVALID_ID;

    memset(shadow->renderState.blendFactor, 0,
           sizeof(shadow->renderState.blendFactor));

    shadow->renderState.sampleMask = 0;
    shadow->renderState.depthStencilStateId = SVGA3D_INVALID_ID;
    shadow->renderState.stencilRef = 0;
    shadow->renderState.rasterizerStateId = SVGA3D_INVALID_ID;
    shadow->renderState.depthStencilViewId = SVGA3D_INVALID_ID;

    for (i = 0; i < SVGA3D_MAX_SIMULTANEOUS_RENDER_TARGETS; i++) {
        shadow->renderState.renderTargetViewIds[i] = SVGA3D_INVALID_ID;
    }

    for (i = 0; i < SVGA3D_DX_MAX_SOTARGETS; i++) {
        shadow->streamOut.targets[i] = SVGA3D_INVALID_ID;
    }

    shadow->streamOut.soid = SVGA3D_INVALID_ID;

    shadow->uavSpliceIndex = 0;
    shadow->numViewports = 0;
    shadow->numScissorRects = 0;

    memset(shadow->viewports, 0, sizeof(shadow->viewports));
    memset(shadow->scissorRects, 0, sizeof(shadow->scissorRects));

    shadow->predication.queryID = SVGA3D_INVALID_ID;
    shadow->predication.value = 0;
    shadow->shaderIfaceMobid = SVGA3D_INVALID_ID;
    shadow->shaderIfaceOffset = 0;

    for (i = 0; i < SVGA3D_NUM_SHADERTYPE; i++) {
        shadow->shaderState[i].shaderId = SVGA3D_INVALID_ID;
        for (j = 0; j < SVGA3D_DX_MAX_CONSTBUFFERS; j++) {
            shadow->shaderState[i].constantBuffers[j].sid = SVGA3D_INVALID_ID;
            shadow->shaderState[i].constantBuffers[j].offsetInBytes = 0;
            shadow->shaderState[i].constantBuffers[j].sizeInBytes = 0;
        }
        for (j = 0; j < SVGA3D_DX_MAX_SRVIEWS; j++) {
            shadow->shaderState[i].shaderResources[j] = SVGA3D_INVALID_ID;
        }
        for (j = 0; j < SVGA3D_DX_MAX_SAMPLERS; j++) {
            shadow->shaderState[i].samplers[j] = SVGA3D_INVALID_ID;
        }
    }

    for (i = 0; i < SVGA3D_MAX_QUERY; i++) {
        shadow->queryID[i] = SVGA3D_INVALID_ID;
    }

    for (i = 0; i < SVGA_COTABLE_MAX; i++) {
        shadow->cotables[i].mobid = SVGA3D_INVALID_ID;
    }

    for (i = 0; i < SVGA3D_DX11_1_MAX_UAVIEWS; i++) {
        shadow->uaViewIds[i] = SVGA3D_INVALID_ID;
        shadow->csuaViewIds[i] = SVGA3D_INVALID_ID;
    }
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_context_define(struct vmsvga_state_s *s,
                                             uint32_t cid)
{
    struct vmsvga3d_state_s *state;
    VMSVGA3DDXContext *context;

    if (cid >= SVGA3D_MAX_CONTEXT_IDS) {
        return false;
    }

    context = g_try_new0(VMSVGA3DDXContext, 1);
    if (context == NULL) {
        return false;
    }

    state = vmsvga3d_state_ensure(s);
    if (state == NULL) {
        g_free(context);
        return false;
    }

    context->cid = cid;
    vmsvga3d_state_dx_context_init_shadow(&context->shadow);
    context->renderer_dirty = VMSVGA3D_DX_CTX_F_STATE_ALL;
    context->index_buffer_size = UINT64_MAX;
    for (uint32_t slot = 0; slot < SVGA3D_DX_MAX_VERTEXBUFFERS; slot++) {
        context->vertex_buffer_size[slot] = UINT64_MAX;
    }

    for (uint32_t type = 0; type < SVGA_COTABLE_MAX; type++) {
        context->cotables[type].mobid = SVGA3D_INVALID_ID;
    }

    vmsvga3d_dx_context_free(state->dx_contexts[cid]);

    state->dx_contexts[cid] = context;
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_context_destroy(struct vmsvga_state_s *s,
                                              uint32_t cid)
{
    struct vmsvga3d_state_s *state = s->svga3d;

    if (state == NULL || cid >= SVGA3D_MAX_CONTEXT_IDS ||
        state->dx_contexts[cid] == NULL) {
        return false;
    }

    vmsvga3d_dx_context_free(state->dx_contexts[cid]);

    state->dx_contexts[cid] = NULL;
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_context_bind(
    struct vmsvga_state_s *s, uint32_t cid,
    const SVGADXContextMobFormat *valid_contents)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL) {
        return false;
    }

    if (valid_contents != NULL) {
        for (uint32_t slot = 0; slot < SVGA3D_DX_MAX_VERTEXBUFFERS; slot++) {
            SVGA3dSurfaceId new_sid =
                valid_contents->inputAssembly.vertexBuffers[slot].bufferId;

            if (new_sid == SVGA3D_INVALID_ID) {
                context->vertex_buffer_size[slot] = 0;
            } else {
                context->vertex_buffer_size[slot] = UINT64_MAX;
                if (context->vertex_buffer_max_bound <= slot) {
                    context->vertex_buffer_max_bound = slot + 1u;
                }
            }
        }

        context->index_buffer_size =
            valid_contents->inputAssembly.indexBufferSid == SVGA3D_INVALID_ID
                ? 0 : UINT64_MAX;

        memcpy(&context->shadow, valid_contents, sizeof(context->shadow));
        context->pending_so_targets_valid = false;
        context->renderer_dirty |=
            VMSVGA3D_DX_CTX_F_STATE_SOTARGETS |
            VMSVGA3D_DX_CTX_F_STATE_SHADERS |
            VMSVGA3D_DX_CTX_F_STATE_SHADER_LINKAGE;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_context_readback(
    struct vmsvga_state_s *s, uint32_t cid,
    SVGADXContextMobFormat *contents)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || contents == NULL) {
        return false;
    }

    memcpy(contents, &context->shadow, sizeof(*contents));
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_context_invalidate(struct vmsvga_state_s *s,
                                                 uint32_t cid)
{
    /* VirtualBox deliberately leaves backend context invalidation unimplemented. */
    (void)s;
    (void)cid;
    return false;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_render_target_count(
    struct vmsvga_state_s *s, uint32_t cid, uint32_t *count)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || count == NULL) {
        return false;
    }

    *count = context->render_target_count;
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_constant_buffer(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10ConstantBufferPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    SVGA3dConstantBufferBinding *binding;

    if (context == NULL || plan == NULL || !plan->shadow_update ||
        plan->stage_index >= SVGA3D_NUM_SHADERTYPE ||
        plan->slot >= SVGA3D_DX_MAX_CONSTBUFFERS) {
        return false;
    }

    binding = &context->shadow.shaderState[plan->stage_index]
                   .constantBuffers[plan->slot];
    binding->sid = plan->sid;
    binding->offsetInBytes = plan->offset_in_bytes;
    binding->sizeInBytes = plan->size_in_bytes;

    if (plan->sid != SVGA3D_INVALID_ID &&
        context->constant_buffer_max_bound[plan->stage_index] < plan->slot + 1u) {
        context->constant_buffer_max_bound[plan->stage_index] = plan->slot + 1u;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_constant_buffer_d3d11(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D11ConstantBufferPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    SVGA3dConstantBufferBinding *binding;
    VMSVGA3DD3D11Level level;
    uint32_t stage_index;

    if (context == NULL || plan == NULL || !plan->shadow_update ||
        plan->stage_index >= SVGA3D_NUM_SHADERTYPE ||
        plan->slot >= SVGA3D_DX_MAX_CONSTBUFFERS) {
        return false;
    }

    level = vmsvga3d_d3d11_shader_stage(plan->shader_type, &stage_index);
    if (level == VMSVGA3D_D3D11_LEVEL_INVALID ||
        stage_index != plan->stage_index) {
        return false;
    }

    binding = &context->shadow.shaderState[stage_index]
                   .constantBuffers[plan->slot];
    binding->sid = plan->sid;
    binding->offsetInBytes = plan->offset_in_bytes;
    binding->sizeInBytes = plan->size_in_bytes;

    if (plan->sid != SVGA3D_INVALID_ID &&
        context->constant_buffer_max_bound[stage_index] < plan->slot + 1u) {
        context->constant_buffer_max_bound[stage_index] = plan->slot + 1u;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_constant_buffer_offset(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D11ConstantBufferOffsetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    SVGA3dConstantBufferBinding *binding;
    VMSVGA3DD3D11Level level;
    uint32_t stage_index;

    if (context == NULL || plan == NULL ||
        plan->stage_index >= SVGA3D_NUM_SHADERTYPE ||
        plan->slot >= SVGA3D_DX_MAX_CONSTBUFFERS) {
        return false;
    }

    level = vmsvga3d_d3d11_shader_stage(plan->shader_type, &stage_index);
    if (level == VMSVGA3D_D3D11_LEVEL_INVALID ||
        stage_index != plan->stage_index) {
        return false;
    }

    binding = &context->shadow.shaderState[stage_index]
                   .constantBuffers[plan->slot];

    /* VirtualBox updates only offsetInBytes for SET_*_CONSTANT_BUFFER_OFFSET. */
    binding->offsetInBytes = plan->offset_in_bytes;

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_shader_resources(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10ShaderResourceSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    uint32_t c_bound;
    int32_t last_not_null = -1;
    uint32_t i;

    if (context == NULL || plan == NULL || !plan->shadow_update_atomic ||
        plan->stage_index >= SVGA3D_NUM_SHADERTYPE ||
        plan->start_view > SVGA3D_DX_MAX_SRVIEWS ||
        plan->shadow_update_count >
            SVGA3D_DX_MAX_SRVIEWS - plan->start_view) {
        return false;
    }

    for (i = 0; i < plan->shadow_update_count; i++) {
        uint32_t slot = plan->start_view + i;
        uint32_t *shadow_id =
            &context->shadow.shaderState[plan->stage_index].shaderResources[slot];

        if (*shadow_id != plan->ids[i]) {
            *shadow_id = plan->ids[i];
            context->shader_resource_modified[plan->stage_index][slot / 64u] |=
                UINT64_C(1) << (slot % 64u);
        }
        if (plan->ids[i] != SVGA3D_INVALID_ID) {
            last_not_null = (int32_t)i;
        }
    }

    c_bound = plan->start_view +
              (last_not_null >= 0 ? (uint32_t)last_not_null + 1u : 0u);

    if (context->shader_resource_max_bound[plan->stage_index] < c_bound) {
        context->shader_resource_max_bound[plan->stage_index] = c_bound;
    }

    /* SET_SHADER_RESOURCES is a native binding command, not merely a shadow
     * mutation.  D3D11 can implicitly NULL an SRV when an overlapping RTV,
     * DSV, or UAV is bound, while our authoritative guest shadow still holds
     * the original view ID.  A later guest SET of that same ID must therefore
     * replay the native binding even though the shadow value did not change.
     *
     * Keep shader_resource_modified change-driven: it controls the immediate
     * pre-draw NULL-unbind used for genuinely new guest bindings.  The stage
     * dirty bit is command-driven so repeated SETs coalesce to one full-stage
     * replay at the next pipeline setup.
     */
    if (plan->shadow_update_count != 0) {
        context->renderer_dirty |=
            VMSVGA3D_DX_CTX_F_STATE_SRV_VS << plan->stage_index;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_shader(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10ShaderSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update ||
        plan->stage_index >= SVGA3D_NUM_SHADERTYPE) {
        return false;
    }

    if (context->shadow.shaderState[plan->stage_index].shaderId !=
        plan->shader_id) {
        SVGA3dShaderType shader_type =
            (SVGA3dShaderType)(plan->stage_index + SVGA3D_SHADERTYPE_MIN);

        context->shadow.shaderState[plan->stage_index].shaderId =
            plan->shader_id;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_SHADERS;
        if (shader_type != SVGA3D_SHADERTYPE_CS) {
            context->renderer_dirty |=
                VMSVGA3D_DX_CTX_F_STATE_SHADER_LINKAGE;
        }
    }
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_samplers(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10SamplerSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    bool modified = false;
    uint32_t i;

    if (context == NULL || plan == NULL ||
        plan->stage_index >= SVGA3D_NUM_SHADERTYPE ||
        plan->start_sampler > SVGA3D_DX_MAX_SAMPLERS ||
        plan->shadow_update_count >
            SVGA3D_DX_MAX_SAMPLERS - plan->start_sampler) {
        return false;
    }

    for (i = 0; i < plan->shadow_update_count; i++) {
        uint32_t *shadow_id =
            &context->shadow.shaderState[plan->stage_index]
                 .samplers[plan->start_sampler + i];

        if (*shadow_id != plan->ids[i]) {
            *shadow_id = plan->ids[i];
            modified = true;
        }
    }

    if (modified) {
        context->renderer_dirty |=
            VMSVGA3D_DX_CTX_F_STATE_SAMPLER_VS << plan->stage_index;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_input_layout(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10InputLayoutSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.inputAssembly.layoutId != plan->layout_id) {
        context->shadow.inputAssembly.layoutId = plan->layout_id;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_INPUTLAYOUT;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_vertex_buffers(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10VertexBufferSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    uint32_t max_bound = 0;
    bool modified = false;
    uint32_t i;

    if (context == NULL || plan == NULL ||
        plan->start_buffer > SVGA3D_DX_MAX_VERTEXBUFFERS ||
        plan->shadow_update_count >
            SVGA3D_DX_MAX_VERTEXBUFFERS - plan->start_buffer) {
        return false;
    }

    for (i = 0; i < plan->shadow_update_count; i++) {
        uint32_t slot = plan->start_buffer + i;
        SVGA3dBufferBinding *binding =
            &context->shadow.inputAssembly.vertexBuffers[slot];

        if (binding->bufferId != plan->bindings[i].sid ||
            binding->stride != plan->bindings[i].stride ||
            binding->offset != plan->bindings[i].offset) {
            binding->bufferId = plan->bindings[i].sid;
            binding->stride = plan->bindings[i].stride;
            binding->offset = plan->bindings[i].offset;
            context->vertex_buffer_modified |= UINT64_C(1) << slot;
            modified = true;
        }
        if (plan->bindings[i].sid != SVGA3D_INVALID_ID) {
            max_bound = slot + 1u;
        }
    }

    if (modified) {
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_VERTEXBUFFER;
    }

    if (context->vertex_buffer_max_bound < max_bound) {
        context->vertex_buffer_max_bound = max_bound;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_index_buffer(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10IndexBufferSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.inputAssembly.indexBufferSid != plan->sid ||
        context->shadow.inputAssembly.indexBufferOffset != plan->offset ||
        context->shadow.inputAssembly.indexBufferFormat != (uint32_t)plan->format) {
        context->shadow.inputAssembly.indexBufferSid = plan->sid;
        context->shadow.inputAssembly.indexBufferOffset = plan->offset;
        context->shadow.inputAssembly.indexBufferFormat = plan->format;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_INDEXBUFFER;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_viewports(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10ViewportsSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    bool modified;
    uint32_t i;

    if (context == NULL || plan == NULL || !plan->shadow_update ||
        plan->count > SVGA3D_DX_MAX_VIEWPORTS) {
        return false;
    }

    modified = context->shadow.numViewports != (uint8_t)plan->count;
    context->shadow.numViewports = (uint8_t)plan->count;

    for (i = 0; i < plan->count; i++) {
        const SVGA3dViewport *src = &plan->viewports[i];
        SVGA3dViewport *dst = &context->shadow.viewports[i];

        if (src->x != dst->x || src->y != dst->y ||
            src->width != dst->width || src->height != dst->height ||
            src->minDepth != dst->minDepth || src->maxDepth != dst->maxDepth) {
            *dst = *src;
            modified = true;
        }
    }

    if (modified) {
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_VIEWPORT;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_topology(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10TopologySetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.inputAssembly.topology != (uint32_t)plan->topology) {
        context->shadow.inputAssembly.topology = plan->topology;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_TOPOLOGY;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_blend_state(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10BlendStateSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.renderState.blendStateId != plan->blend_id) {
        context->shadow.renderState.blendStateId = plan->blend_id;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_BLENDSTATE;
    }

    if (memcmp(context->shadow.renderState.blendFactor, plan->blend_factor,
               sizeof(context->shadow.renderState.blendFactor)) != 0) {
        memcpy(context->shadow.renderState.blendFactor, plan->blend_factor,
               sizeof(context->shadow.renderState.blendFactor));
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_BLENDSTATE;
    }

    if (context->shadow.renderState.sampleMask != plan->sample_mask) {
        context->shadow.renderState.sampleMask = plan->sample_mask;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_BLENDSTATE;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_depth_stencil_state(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10DepthStencilStateSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.renderState.depthStencilStateId !=
        plan->depth_stencil_id) {
        context->shadow.renderState.depthStencilStateId = plan->depth_stencil_id;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_DEPTHSTENCILSTATE;
    }

    if (context->shadow.renderState.stencilRef != plan->stencil_ref) {
        context->shadow.renderState.stencilRef = plan->stencil_ref;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_DEPTHSTENCILSTATE;
    }
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_rasterizer_state(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10RasterizerStateSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.renderState.rasterizerStateId != plan->rasterizer_id) {
        context->shadow.renderState.rasterizerStateId = plan->rasterizer_id;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_RASTERIZERSTATE;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_scissors(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10ScissorPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    bool modified;
    uint32_t i;

    if (context == NULL || plan == NULL || !plan->shadow_update ||
        plan->count > SVGA3D_DX_MAX_SCISSORRECTS) {
        return false;
    }

    modified = context->shadow.numScissorRects != (uint8_t)plan->count;
    context->shadow.numScissorRects = (uint8_t)plan->count;

    for (i = 0; i < plan->count; i++) {
        const SVGASignedRect *src = &plan->rects[i];
        SVGASignedRect *dst = &context->shadow.scissorRects[i];

        if (src->left != dst->left || src->top != dst->top ||
            src->right != dst->right || src->bottom != dst->bottom) {
            *dst = *src;
            modified = true;
        }
    }

    if (modified) {
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_SCISSORRECT;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_render_targets(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10RenderTargetsSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    uint32_t render_target_count = 0;
    bool modified = false;
    uint32_t i;

    if (context == NULL || plan == NULL || !plan->shadow_update_atomic ||
        plan->shadow_update_count > SVGA3D_MAX_RENDER_TARGETS) {
        return false;
    }

    if (context->shadow.renderState.depthStencilViewId !=
        plan->depth_stencil_view_id) {
        context->shadow.renderState.depthStencilViewId =
            plan->depth_stencil_view_id;
        modified = true;
    }

    for (i = 0; i < SVGA3D_MAX_RENDER_TARGETS; i++) {
        uint32_t id = i < plan->shadow_update_count
                          ? plan->ids[i]
                          : SVGA3D_INVALID_ID;

        if (context->shadow.renderState.renderTargetViewIds[i] != id) {
            context->shadow.renderState.renderTargetViewIds[i] = id;
            modified = true;
        }
        if (id != SVGA3D_INVALID_ID) {
            render_target_count = i + 1u;
        }
    }

    if (modified) {
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_RENDERTARGET;
    }

    context->render_target_count = render_target_count;
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_uavs(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D11UAVSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    int32_t last_not_null = -1;
    bool modified = false;
    uint32_t bound_count;
    uint32_t i;

    if (context == NULL || plan == NULL || !plan->shadow_update_atomic ||
        plan->uav_splice_index > SVGA3D_MAX_SIMULTANEOUS_RENDER_TARGETS ||
        plan->count > SVGA3D_DX11_1_MAX_UAVIEWS) {
        return false;
    }

    for (i = 0; i < plan->count; i++) {
        if (context->shadow.uaViewIds[i] != plan->ids[i]) {
            context->shadow.uaViewIds[i] = plan->ids[i];
            modified = true;
        }

        if (plan->ids[i] != SVGA3D_INVALID_ID) {
            last_not_null = (int32_t)i;
        }
    }

    if (context->shadow.uavSpliceIndex != plan->uav_splice_index) {
        context->shadow.uavSpliceIndex = plan->uav_splice_index;
        modified = true;
    }

    bound_count = (uint32_t)last_not_null + 1u;

    if (context->uav_max_bound < bound_count) {
        context->uav_max_bound = bound_count;
    }

    if (modified) {
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_RENDERTARGET;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_cs_uavs(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D11CSUAVSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);
    int32_t last_not_null = -1;
    bool modified = false;
    uint32_t bound_count;
    uint32_t i;

    if (context == NULL || plan == NULL || !plan->shadow_update_atomic ||
        plan->start_index >= SVGA3D_DX11_1_MAX_UAVIEWS ||
        plan->count > SVGA3D_DX11_1_MAX_UAVIEWS - plan->start_index) {
        return false;
    }

    for (i = 0; i < plan->count; i++) {
        uint32_t slot = plan->start_index + i;

        if (context->shadow.csuaViewIds[slot] != plan->ids[i]) {
            context->shadow.csuaViewIds[slot] = plan->ids[i];
            context->cs_uav_modified[slot / 64u] |=
                UINT64_C(1) << (slot % 64u);
            modified = true;
        }

        if (plan->ids[i] != SVGA3D_INVALID_ID) {
            last_not_null = (int32_t)i;
        }
    }

    bound_count = plan->start_index + (uint32_t)last_not_null + 1u;

    if (context->cs_uav_max_bound < bound_count) {
        context->cs_uav_max_bound = bound_count;
    }

    if (modified) {
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_CSTARGET;
    }

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_so_targets(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10SOTargetsPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    memcpy(context->shadow.streamOut.targets, plan->shadow_targets,
           sizeof(context->shadow.streamOut.targets));
    context->pending_so_targets = *plan;
    context->pending_so_targets_valid = true;
    context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_SOTARGETS;

    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_apply_stream_output(
    struct vmsvga_state_s *s, uint32_t cid,
    const VMSVGA3DD3D10StreamOutputSetPlan *plan)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL || plan == NULL || !plan->shadow_update) {
        return false;
    }

    if (context->shadow.streamOut.soid != plan->stream_output_id) {
        context->shadow.streamOut.soid = plan->stream_output_id;
        context->renderer_dirty |= VMSVGA3D_DX_CTX_F_STATE_SHADERS;
    }
    return true;
}

static bool VMSVGA3D_DX_STATE_UNUSED vmsvga3d_state_dx_clear_rtv_references(
    struct vmsvga_state_s *s, uint32_t cid,
    SVGA3dRenderTargetViewId destroyed_id)
{
    VMSVGA3DDXContext *context = vmsvga3d_dx_context(s, cid);

    if (context == NULL) {
        return false;
    }

    return vmsvga3d_d3d10_rtv_destroy_shadow_refs(
               destroyed_id, context->shadow.renderState.renderTargetViewIds) !=
           VMSVGA3D_D3D10_LEVEL_INVALID;
}

#undef VMSVGA3D_DX_STATE_UNUSED

static bool vmsvga3d_state_set_transform(struct vmsvga_state_s *s,
                                            uint32_t cid,
                                            SVGA3dTransformType type,
                                            const float matrix[16])
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || matrix == NULL || type < SVGA3D_TRANSFORM_MIN ||
        type >= SVGA3D_TRANSFORM_MAX) {
        return false;
    }

    if (!context->transform[type].valid ||
        memcmp(context->transform[type].matrix, matrix,
               sizeof(context->transform[type].matrix)) != 0) {
        memcpy(context->transform[type].matrix, matrix,
               sizeof(context->transform[type].matrix));
        context->transform[type].valid = true;
        context->legacy_transform_dirty |= UINT64_C(1) << type;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }
    return true;
}

static bool vmsvga3d_state_set_z_range(struct vmsvga_state_s *s,
                                          uint32_t cid,
                                          const SVGA3dZRange *z_range)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || z_range == NULL) {
        return false;
    }

    if (!context->z_range_valid ||
        memcmp(&context->z_range, z_range, sizeof(*z_range)) != 0) {
        context->z_range = *z_range;
        context->z_range_valid = true;
        context->legacy_viewport_dirty = true;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_set_render_state(
    struct vmsvga_state_s *s, uint32_t cid, uint32_t count,
    const SVGA3dRenderState *states)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    uint32_t i;

    if (context == NULL || (count != 0 && states == NULL)) {
        return false;
    }

    for (i = 0; i < count; i++) {
        if (states[i].state >= SVGA3D_RS_MAX) {
            return false;
        }
        if (!context->render_state[states[i].state].valid ||
            context->render_state[states[i].state].value != states[i].uintValue) {
            context->render_state[states[i].state].value = states[i].uintValue;
            context->render_state[states[i].state].valid = true;
            context->legacy_render_state_dirty[states[i].state / 64u] |=
                UINT64_C(1) << (states[i].state % 64u);
            context->legacy_pipeline_dirty |=
                VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;

            /* Legacy replay is defined by SVGA render-state enum order.  A few
             * VMware states translate onto native D3D9 slots that are also
             * written by a later VMware state, so preserve that later-state
             * override when only the earlier state changes incrementally. */
            if (states[i].state == SVGA3D_RS_FOGMODE &&
                context->render_state[SVGA3D_RS_RANGEFOGENABLE].valid) {
                context->legacy_render_state_dirty[
                    SVGA3D_RS_RANGEFOGENABLE / 64u] |=
                    UINT64_C(1) << (SVGA3D_RS_RANGEFOGENABLE % 64u);
            }
            if (states[i].state == SVGA3D_RS_VERTEXMATERIALENABLE &&
                context->render_state[SVGA3D_RS_INDEXEDVERTEXBLENDENABLE].valid) {
                context->legacy_render_state_dirty[
                    SVGA3D_RS_INDEXEDVERTEXBLENDENABLE / 64u] |=
                    UINT64_C(1) << (SVGA3D_RS_INDEXEDVERTEXBLENDENABLE % 64u);
            }
        }
    }

    return true;
}

static bool vmsvga3d_state_set_render_target(
    struct vmsvga_state_s *s, uint32_t cid, SVGA3dRenderTargetType type,
    const SVGA3dSurfaceImageId *target)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DSurface *surface;
    VMSVGA3DSurfaceImage *image;

    if (context == NULL || target == NULL || type < SVGA3D_RT_MIN ||
        type >= SVGA3D_RT_MAX) {
        return false;
    }

    if (target->sid == SVGA3D_INVALID_ID) {
        if (memcmp(&context->render_targets[type], target, sizeof(*target)) != 0) {
            context->render_targets[type] = *target;
            context->legacy_target_dirty |= UINT32_C(1) << type;
            context->legacy_pipeline_dirty |=
                VMSVGA3D_LEGACY_PIPELINE_DIRTY_TARGETS;
            if (type >= SVGA3D_RT_COLOR0 && type <= SVGA3D_RT_COLOR3) {
                context->legacy_viewport_dirty = true;
                context->legacy_scissor_dirty = true;
                context->legacy_pipeline_dirty |=
                    VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
            }
        }
        return true;
    }

    if (s->svga3d == NULL || target->sid >= SVGA3D_MAX_SURFACE_IDS) {
        return false;
    }

    surface = s->svga3d->surfaces[target->sid];

    if (!vmsvga3d_surface_image(surface, target, &image)) {
        return false;
    }

    /* VBox's D3D9 backend only accepts the base image for depth/stencil
     * render targets. */
    if ((type == SVGA3D_RT_DEPTH || type == SVGA3D_RT_STENCIL) &&
        (target->face != 0 || target->mipmap != 0)) {
        return false;
    }

    if (memcmp(&context->render_targets[type], target, sizeof(*target)) != 0) {
        context->render_targets[type] = *target;
        context->legacy_target_dirty |= UINT32_C(1) << type;
        context->legacy_pipeline_dirty |=
            VMSVGA3D_LEGACY_PIPELINE_DIRTY_TARGETS;
        if (type >= SVGA3D_RT_COLOR0 && type <= SVGA3D_RT_COLOR3) {
            context->legacy_viewport_dirty = true;
            context->legacy_scissor_dirty = true;
            context->legacy_pipeline_dirty |=
                VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
        }
    }

    return true;
}

static bool vmsvga3d_state_set_texture_state(
    struct vmsvga_state_s *s, uint32_t cid, uint32_t count,
    const SVGA3dTextureState *states)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    uint32_t i;

    if (context == NULL || (count != 0 && states == NULL)) {
        return false;
    }

    for (i = 0; i < count; i++) {
        if (states[i].name == SVGA3D_TS_BIND_TEXTURE) {
            if (states[i].stage >= VMSVGA3D_MAX_SAMPLERS) {
                continue;
            }
            if (states[i].value != SVGA3D_INVALID_ID &&
                (s->svga3d == NULL || states[i].value >= SVGA3D_MAX_SURFACE_IDS ||
                 s->svga3d->surfaces[states[i].value] == NULL)) {
                return false;
            }
        } else if (states[i].stage >= VMSVGA3D_MAX_SAMPLERS ||
                   states[i].name < SVGA3D_TS_MIN ||
                   states[i].name >= SVGA3D_TS_MAX) {
            continue;
        }
        if (!context->texture_state[states[i].stage][states[i].name].valid ||
            context->texture_state[states[i].stage][states[i].name].value !=
                states[i].value) {
            context->texture_state[states[i].stage][states[i].name].value =
                states[i].value;
            context->texture_state[states[i].stage][states[i].name].valid = true;
            context->legacy_texture_state_dirty[states[i].stage] |=
                UINT64_C(1) << states[i].name;
            context->legacy_pipeline_dirty |=
                VMSVGA3D_LEGACY_PIPELINE_DIRTY_TEXTURES;
        }
    }

    return true;
}

static bool vmsvga3d_state_set_material(struct vmsvga_state_s *s,
                                           uint32_t cid, SVGA3dFace face,
                                           const SVGA3dMaterial *material)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || material == NULL || (uint32_t)face >= SVGA3D_FACE_MAX) {
        return false;
    }

    if (!context->material[face].valid ||
        memcmp(&context->material[face].material, material,
               sizeof(*material)) != 0) {
        context->material[face].material = *material;
        context->material[face].valid = true;
        context->legacy_material_dirty |= UINT32_C(1) << face;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_set_light_data(struct vmsvga_state_s *s,
                                             uint32_t cid, uint32_t index,
                                             const SVGA3dLightData *data)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || data == NULL || index >= SVGA3D_NUM_LIGHTS ||
        (data->type != SVGA3D_LIGHTTYPE_POINT &&
         data->type != SVGA3D_LIGHTTYPE_SPOT1 &&
         data->type != SVGA3D_LIGHTTYPE_DIRECTIONAL)) {
        return false;
    }

    if (!context->light[index].data_valid ||
        memcmp(&context->light[index].data, data, sizeof(*data)) != 0) {
        context->light[index].data = *data;
        context->light[index].data_valid = true;
        context->legacy_light_data_dirty |= UINT32_C(1) << index;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_set_light_enabled(struct vmsvga_state_s *s,
                                                uint32_t cid,
                                                uint32_t index,
                                                uint32_t enabled)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || index >= SVGA3D_NUM_LIGHTS) {
        return false;
    }

    if (!context->light[index].enabled_valid ||
        context->light[index].enabled != enabled) {
        context->light[index].enabled = enabled;
        context->light[index].enabled_valid = true;
        context->legacy_light_enable_dirty |= UINT32_C(1) << index;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_set_viewport(struct vmsvga_state_s *s,
                                           uint32_t cid,
                                           const SVGA3dRect *rect)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || rect == NULL) {
        return false;
    }

    if (!context->viewport_valid ||
        memcmp(&context->viewport, rect, sizeof(*rect)) != 0) {
        context->viewport = *rect;
        context->viewport_valid = true;
        context->legacy_viewport_dirty = true;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_set_clip_plane(struct vmsvga_state_s *s,
                                             uint32_t cid, uint32_t index,
                                             const float plane[4])
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || plane == NULL || index >= VMSVGA3D_MAX_CLIP_PLANES) {
        return false;
    }

    if (!context->clip_plane[index].valid ||
        memcmp(context->clip_plane[index].plane, plane,
               sizeof(context->clip_plane[index].plane)) != 0) {
        memcpy(context->clip_plane[index].plane, plane,
               sizeof(context->clip_plane[index].plane));
        context->clip_plane[index].valid = true;
        context->legacy_clip_plane_dirty |= UINT32_C(1) << index;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_clear(struct vmsvga_state_s *s, uint32_t cid,
                                    SVGA3dClearFlag clear_flags,
                                    uint32_t color, float depth,
                                    uint32_t stencil, uint32_t rect_count,
                                    const SVGA3dRect *rects)
{
    VMSVGA3DContext *context;
    uint32_t type;
    uint32_t flags = clear_flags;
    bool valid = true;

    if (rect_count != 0 && rects == NULL) {
        return false;
    }

    context = vmsvga3d_context(s, cid);

    if (context == NULL) {
        return false;
    }

    if (flags & SVGA3D_CLEAR_COLOR) {
        for (type = SVGA3D_RT_COLOR0; type <= SVGA3D_RT_COLOR7 && valid; type++) {
            valid = vmsvga3d_clear_target(s, context, type, color, depth, stencil,
                                          rects, rect_count, false);
        }
    }

    if (valid && (flags & SVGA3D_CLEAR_DEPTH)) {
        valid = vmsvga3d_clear_target(s, context, SVGA3D_RT_DEPTH, color, depth,
                                      stencil, rects, rect_count, false);
    }

    if (valid && (flags & SVGA3D_CLEAR_STENCIL)) {
        valid = vmsvga3d_clear_target(s, context, SVGA3D_RT_STENCIL, color, depth,
                                      stencil, rects, rect_count, false);
    }

    if (!valid) {
        return false;
    }

    if (flags & SVGA3D_CLEAR_COLOR) {
        for (type = SVGA3D_RT_COLOR0; type <= SVGA3D_RT_COLOR7 && valid; type++) {
            valid = vmsvga3d_clear_target(s, context, type, color, depth, stencil,
                                          rects, rect_count, true);
        }
    }

    if (valid && (flags & SVGA3D_CLEAR_DEPTH)) {
        valid = vmsvga3d_clear_target(s, context, SVGA3D_RT_DEPTH, color, depth,
                                      stencil, rects, rect_count, true);
    }

    if (valid && (flags & SVGA3D_CLEAR_STENCIL)) {
        valid = vmsvga3d_clear_target(s, context, SVGA3D_RT_STENCIL, color, depth,
                                      stencil, rects, rect_count, true);
    }

    return valid;
}

static uint64_t vmsvga3d_state_trace_hash(const uint8_t *data,
                                                uint32_t size)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    uint32_t i;

    for (i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void vmsvga3d_state_trace_matrix(
    uint64_t frame, uint64_t seq, uint32_t cid, const char *name,
    const VMSVGA3DTransformState *transform)
{
    if (transform == NULL || !transform->valid) {
        fprintf(stderr,
                "VMVGA-D3D9-TEXGEN-MATRIX frame=%" PRIu64
                " seq=%" PRIu64 " cid=%u name=%s valid=0\n",
                frame, seq, cid, name);
        return;
    }

    fprintf(stderr,
            "VMVGA-D3D9-TEXGEN-MATRIX frame=%" PRIu64
            " seq=%" PRIu64 " cid=%u name=%s valid=1 m=",
            frame, seq, cid, name);
    for (uint32_t i = 0; i < 16; i++) {
        fprintf(stderr, "%s%.9g", i == 0 ? "" : ",",
                (double)transform->matrix[i]);
    }
    fprintf(stderr, "\n");
}

static const char *vmsvga3d_state_trace_cube_avg_rgb(
    const VMSVGA3DSurfaceImage *image, SVGA3dSurfaceFormat format,
    uint32_t *red, uint32_t *green, uint32_t *blue)
{
    uint64_t red_sum = 0;
    uint64_t green_sum = 0;
    uint64_t blue_sum = 0;
    uint64_t pixel_count;
    uint64_t required;
    uint32_t x;
    uint32_t y;

    /* These formats store R, G, B in the first three bytes on little-endian
     * hosts.  Do not guess channel order for other surface formats. */
    if (format != SVGA3D_X8R8G8B8 && format != SVGA3D_A8R8G8B8) {
        return "unsupported-format";
    }
    if (image == NULL || image->data == NULL ||
        image->size.width == 0 || image->size.height == 0 ||
        image->pitch < (uint64_t)image->size.width * 4u) {
        return "invalid-layout";
    }

    required = (uint64_t)image->pitch * image->size.height;
    if (required > image->data_size) {
        return "invalid-layout";
    }

    pixel_count = (uint64_t)image->size.width * image->size.height;
    for (y = 0; y < image->size.height; y++) {
        const uint8_t *row = image->data + (size_t)y * image->pitch;

        for (x = 0; x < image->size.width; x++) {
            const uint8_t *pixel = row + (size_t)x * 4u;

            red_sum += pixel[0];
            green_sum += pixel[1];
            blue_sum += pixel[2];
        }
    }

    *red = red_sum / pixel_count;
    *green = green_sum / pixel_count;
    *blue = blue_sum / pixel_count;
    return "ok";
}

static void vmsvga3d_state_trace_cube_shadow(
    uint64_t frame, uint64_t seq, uint32_t cid, uint32_t stage,
    uint32_t sid, const VMSVGA3DSurface *surface)
{
    uint32_t levels;
    uint32_t face;
    static const char *const face_names[SVGA3D_MAX_SURFACE_FACES] = {
        "+X", "-X", "+Y", "-Y", "+Z", "-Z"
    };
    uint64_t budget = UINT64_C(2) * 1024 * 1024;

    if (surface == NULL ||
        (surface->surface_flags & SVGA3D_SURFACE_CUBEMAP) == 0) {
        return;
    }

    levels = surface->face[0].numMipLevels;
    fprintf(stderr,
            "VMVGA-CUBEMAP-SHADOW frame=%" PRIu64 " seq=%" PRIu64
            " cid=%u stage=%u sid=%u levels=%u faces=%u source=cpu-shadow"
            " format=%u",
            frame, seq, cid, stage, sid, levels,
            SVGA3D_MAX_SURFACE_FACES, (uint32_t)surface->format);

    for (face = 0; face < SVGA3D_MAX_SURFACE_FACES; face++) {
        uint64_t index = (uint64_t)face * levels;
        const VMSVGA3DSurfaceImage *image = NULL;

        if (levels != 0 && index < surface->mip_count &&
            surface->mips != NULL) {
            image = &surface->mips[index];
        }

        if (image == NULL || image->data == NULL) {
            fprintf(stderr, " f%u=missing", face);
        } else if (image->data_size > budget) {
            fprintf(stderr,
                    " f%u(%s)=%ux%u/p%u/b%u/hash=budget/avg=budget",
                    face, face_names[face], image->size.width,
                    image->size.height, image->pitch, image->data_size);
        } else {
            uint32_t red, green, blue;
            const char *avg_status =
                vmsvga3d_state_trace_cube_avg_rgb(image, surface->format,
                                                  &red, &green, &blue);
            uint64_t hash = vmsvga3d_state_trace_hash(image->data,
                                                       image->data_size);

            budget -= image->data_size;
            if (strcmp(avg_status, "ok") == 0) {
                fprintf(stderr,
                        " f%u(%s)=%ux%u/p%u/b%u/avg_rgb=%u,%u,%u"
                        "/hash=%016" PRIx64,
                        face, face_names[face], image->size.width,
                        image->size.height, image->pitch, image->data_size,
                        red, green, blue, hash);
            } else {
                fprintf(stderr,
                        " f%u(%s)=%ux%u/p%u/b%u/avg=%s/hash=%016" PRIx64,
                        face, face_names[face], image->size.width,
                        image->size.height, image->pitch, image->data_size,
                        avg_status, hash);
            }
        }
    }
    fprintf(stderr, "\n");
}

/*
 * Full-mode only: read each level-0 cube face back from the DXVK/D3D9 texture
 * into a private scratch image (never the CPU shadow) and log average bytes.
 * CPU-shadow hashes cannot show GPU-only content (render-target writes,
 * GPU copies), so this separates "the guest never wrote the face" from "the
 * host texture lost it".  Bytes 0..2 of X8R8G8B8/A8R8G8B8 are B,G,R on
 * little-endian hosts.  Limited to two records per capture.
 */
static void vmsvga3d_state_trace_cube_host(
    struct vmsvga_state_s *s, uint64_t frame, uint64_t seq, uint32_t cid,
    uint32_t stage, uint32_t sid, VMSVGA3DSurface *surface)
{
    static uint64_t last_seq;
    static uint32_t emitted;
    uint32_t levels;
    uint32_t face;

    if (seq < last_seq) {
        emitted = 0;
    }
    last_seq = seq;

    if (emitted >= 2 || s == NULL || surface == NULL ||
        (surface->surface_flags & SVGA3D_SURFACE_CUBEMAP) == 0 ||
        surface->dxvk_surface == NULL || surface->mips == NULL ||
        !vmsvga3d_dxvk_ready(s->dxvk) ||
        (surface->format != SVGA3D_X8R8G8B8 &&
         surface->format != SVGA3D_A8R8G8B8)) {
        return;
    }
    levels = surface->face[0].numMipLevels;
    if (levels == 0) {
        return;
    }
    emitted++;

    fprintf(stderr,
            "VMVGA-CUBEMAP-HOST frame=%" PRIu64 " seq=%" PRIu64
            " cid=%u stage=%u sid=%u levels=%u source=dxvk-readback"
            " bytes012=B,G,R",
            frame, seq, cid, stage, sid, levels);

    for (face = 0; face < SVGA3D_MAX_SURFACE_FACES; face++) {
        uint32_t index = face * levels;
        VMSVGA3DSurfaceImage scratch;
        uint32_t red = 0;
        uint32_t green = 0;
        uint32_t blue = 0;
        const char *status;

        if (index >= surface->mip_count ||
            surface->mips[index].data_size == 0 ||
            surface->mips[index].data_size > UINT32_C(1024) * 1024) {
            fprintf(stderr, " f%u=skipped", face);
            continue;
        }
        scratch = surface->mips[index];
        scratch.data = g_try_malloc0(scratch.data_size);
        if (scratch.data == NULL) {
            fprintf(stderr, " f%u=alloc-failed", face);
            continue;
        }
        if (!vmsvga3d_dxvk_readback_image(s->dxvk, surface->dxvk_surface,
                                          surface->format, index, &scratch)) {
            fprintf(stderr, " f%u=readback-failed", face);
        } else {
            status = vmsvga3d_state_trace_cube_avg_rgb(&scratch,
                                                       surface->format,
                                                       &red, &green, &blue);
            if (strcmp(status, "ok") == 0) {
                fprintf(stderr, " f%u=avg=%u,%u,%u", face, red, green, blue);
            } else {
                fprintf(stderr, " f%u=%s", face, status);
            }
        }
        g_free(scratch.data);
    }
    fprintf(stderr, "\n");
}

static void vmsvga3d_state_trace_texgen(
    struct vmsvga_state_s *s, const VMSVGA3DContext *context, uint32_t cid)
{
    uint32_t stage;
    uint64_t frame;
    uint64_t seq;
    bool trace = false;

    if (!vmsvga_trace_detail_enabled(s)) {
        return;
    }
    frame = vmsvga_trace_frame_id(s);
    seq = vmsvga_trace_event_sequence(s);

    for (stage = 0; stage < VMSVGA3D_MAX_SAMPLERS; stage++) {
        const VMSVGA3DStateValue *flags =
            &context->texture_state[stage][SVGA3D_TS_TEXTURETRANSFORMFLAGS];
        const VMSVGA3DStateValue *gen =
            &context->texture_state[stage][SVGA3D_TS_TEXCOORDGEN];

        if ((flags->valid &&
             (flags->value & (SVGA3D_TEX_TRANSFORM_R |
                              SVGA3D_TEX_TRANSFORM_Q)) != 0) ||
            (gen->valid && gen->value != 0)) {
            trace = true;
            break;
        }
    }
    if (!trace) {
        return;
    }

    vmsvga3d_state_trace_matrix(frame, seq, cid, "WORLD",
        &context->transform[SVGA3D_TRANSFORM_WORLD]);
    vmsvga3d_state_trace_matrix(frame, seq, cid, "VIEW",
        &context->transform[SVGA3D_TRANSFORM_VIEW]);
    vmsvga3d_state_trace_matrix(frame, seq, cid, "TEXTURE0",
        &context->transform[SVGA3D_TRANSFORM_TEXTURE0]);

    for (uint32_t face = 0; face < SVGA3D_FACE_MAX; face++) {
        const VMSVGA3DMaterialState *material = &context->material[face];
        if (!material->valid) {
            continue;
        }
        fprintf(stderr,
                "VMVGA-D3D9-TEXGEN-MATERIAL frame=%" PRIu64
                " seq=%" PRIu64 " cid=%u face=%u valid=1 "
                "diffuse=%.7g,%.7g,%.7g,%.7g ambient=%.7g,%.7g,%.7g,%.7g "
                "specular=%.7g,%.7g,%.7g,%.7g emissive=%.7g,%.7g,%.7g,%.7g "
                "power=%.7g\n",
                frame, seq, cid, face,
                (double)material->material.diffuse[0],
                (double)material->material.diffuse[1],
                (double)material->material.diffuse[2],
                (double)material->material.diffuse[3],
                (double)material->material.ambient[0],
                (double)material->material.ambient[1],
                (double)material->material.ambient[2],
                (double)material->material.ambient[3],
                (double)material->material.specular[0],
                (double)material->material.specular[1],
                (double)material->material.specular[2],
                (double)material->material.specular[3],
                (double)material->material.emissive[0],
                (double)material->material.emissive[1],
                (double)material->material.emissive[2],
                (double)material->material.emissive[3],
                (double)material->material.shininess);
    }

    for (uint32_t light = 0; light < SVGA3D_NUM_LIGHTS; light++) {
        const VMSVGA3DLightState *state = &context->light[light];
        const SVGA3dLightData *data = &state->data;
        if (!state->enabled_valid || state->enabled == 0) {
            continue;
        }
        fprintf(stderr,
                "VMVGA-D3D9-TEXGEN-LIGHT frame=%" PRIu64
                " seq=%" PRIu64 " cid=%u index=%u enabled=1 "
                "data_valid=%u type=%u pos=%.7g,%.7g,%.7g "
                "dir=%.7g,%.7g,%.7g diffuse=%.7g,%.7g,%.7g,%.7g "
                "ambient=%.7g,%.7g,%.7g,%.7g specular=%.7g,%.7g,%.7g,%.7g\n",
                frame, seq, cid, light, (unsigned)state->data_valid,
                (unsigned)data->type,
                (double)data->position[0], (double)data->position[1],
                (double)data->position[2], (double)data->direction[0],
                (double)data->direction[1], (double)data->direction[2],
                (double)data->diffuse[0], (double)data->diffuse[1],
                (double)data->diffuse[2], (double)data->diffuse[3],
                (double)data->ambient[0], (double)data->ambient[1],
                (double)data->ambient[2], (double)data->ambient[3],
                (double)data->specular[0], (double)data->specular[1],
                (double)data->specular[2], (double)data->specular[3]);
    }

    for (stage = 0; stage < VMSVGA3D_MAX_SAMPLERS; stage++) {
        const VMSVGA3DStateValue *flags =
            &context->texture_state[stage][SVGA3D_TS_TEXTURETRANSFORMFLAGS];
        const VMSVGA3DStateValue *gen =
            &context->texture_state[stage][SVGA3D_TS_TEXCOORDGEN];
        const VMSVGA3DStateValue *binding =
            &context->texture_state[stage][SVGA3D_TS_BIND_TEXTURE];
        VMSVGA3DSurface *surface = NULL;

        if (!((flags->valid &&
               (flags->value & (SVGA3D_TEX_TRANSFORM_R |
                                SVGA3D_TEX_TRANSFORM_Q)) != 0) ||
              (gen->valid && gen->value != 0))) {
            continue;
        }
        fprintf(stderr,
                "VMVGA-D3D9-TEXGEN-STATE frame=%" PRIu64
                " seq=%" PRIu64 " cid=%u stage=%u sid=%u "
                "transformflags=%s%08x texcoordgen=%s%08x\n",
                frame, seq, cid, stage,
                binding->valid ? binding->value : SVGA3D_INVALID_ID,
                flags->valid ? "" : "invalid/", flags->value,
                gen->valid ? "" : "invalid/", gen->value);
        if (binding->valid && binding->value != SVGA3D_INVALID_ID &&
            s->svga3d != NULL && binding->value < SVGA3D_MAX_SURFACE_IDS) {
            surface = s->svga3d->surfaces[binding->value];
        }
        vmsvga3d_state_trace_cube_shadow(frame, seq, cid, stage,
            binding->valid ? binding->value : SVGA3D_INVALID_ID, surface);
        vmsvga3d_state_trace_cube_host(s, frame, seq, cid, stage,
            binding->valid ? binding->value : SVGA3D_INVALID_ID, surface);
    }
}

static void vmsvga3d_state_trace_draw_snapshot(
    struct vmsvga_state_s *s, const VMSVGA3DContext *context, uint32_t cid)
{
    uint32_t i;
    uint32_t stage;
    uint32_t name;
    bool first;

    if (!vmsvga_trace_detail_enabled(s) || context == NULL) {
        return;
    }

    fprintf(stderr,
            "VMVGA-D3D9-STATE-SNAPSHOT frame=%" PRIu64
            " seq=%" PRIu64 " cid=%u viewport_valid=%u "
            "viewport=%u,%u/%ux%u scissor_valid=%u "
            "scissor=%u,%u/%ux%u targets=",
            vmsvga_trace_frame_id(s), vmsvga_trace_event_sequence(s), cid,
            (unsigned)context->viewport_valid, context->viewport.x,
            context->viewport.y, context->viewport.w, context->viewport.h,
            (unsigned)context->scissor_valid,
            context->scissor.x, context->scissor.y, context->scissor.w,
            context->scissor.h);

    first = true;
    for (i = 0; i < SVGA3D_RT_MAX; i++) {
        const SVGA3dSurfaceImageId *target = &context->render_targets[i];
        VMSVGA3DSurface *surface = NULL;

        if (target->sid == SVGA3D_INVALID_ID) {
            continue;
        }
        if (s->svga3d != NULL && target->sid < SVGA3D_MAX_SURFACE_IDS) {
            surface = s->svga3d->surfaces[target->sid];
        }
        if (surface != NULL && surface->mip_count != 0 &&
            surface->mips != NULL) {
            fprintf(stderr, "%s%u:%u/%u/%u@%ux%u/f%u",
                    first ? "" : ",", i, target->sid, target->face,
                    target->mipmap, surface->mips[0].size.width,
                    surface->mips[0].size.height, (uint32_t)surface->format);
        } else {
            fprintf(stderr, "%s%u:%u/%u/%u@missing", first ? "" : ",", i,
                    target->sid, target->face, target->mipmap);
        }
        first = false;
    }

    fprintf(stderr, " shaders=");
    first = true;
    for (i = 0; i < SVGA3D_NUM_SHADERTYPE_PREDX; i++) {
        if (context->bound_shader[i] == SVGA3D_INVALID_ID) {
            continue;
        }
        fprintf(stderr, "%s%u:%u", first ? "" : ",", i,
                context->bound_shader[i]);
        first = false;
    }

    fprintf(stderr, " textures=");
    first = true;
    for (stage = 0; stage < VMSVGA3D_MAX_SAMPLERS; stage++) {
        const VMSVGA3DStateValue *binding =
            &context->texture_state[stage][SVGA3D_TS_BIND_TEXTURE];
        VMSVGA3DSurface *surface = NULL;

        if (!binding->valid) {
            continue;
        }
        if (binding->value == SVGA3D_INVALID_ID) {
            fprintf(stderr, "%s%u:unbound", first ? "" : ",", stage);
            first = false;
            continue;
        }
        if (s->svga3d != NULL &&
            binding->value < SVGA3D_MAX_SURFACE_IDS) {
            surface = s->svga3d->surfaces[binding->value];
        }
        if (surface != NULL && surface->mip_count != 0 &&
            surface->mips != NULL) {
            fprintf(stderr, "%s%u:%u@%ux%u/f%u/m%u", first ? "" : ",",
                    stage, binding->value, surface->mips[0].size.width,
                    surface->mips[0].size.height, (uint32_t)surface->format,
                    surface->mip_count);
        } else {
            fprintf(stderr, "%s%u:%u@missing", first ? "" : ",",
                    stage, binding->value);
        }
        first = false;
    }

    fprintf(stderr, " texture_states=");
    first = true;
    for (stage = 0; stage < VMSVGA3D_MAX_SAMPLERS; stage++) {
        for (name = SVGA3D_TS_MIN; name < SVGA3D_TS_MAX; name++) {
            const VMSVGA3DStateValue *value =
                &context->texture_state[stage][name];

            if (!value->valid || name == SVGA3D_TS_BIND_TEXTURE) {
                continue;
            }
            fprintf(stderr, "%s%u/%u=%08x", first ? "" : ",",
                    stage, name, value->value);
            first = false;
        }
    }

    fprintf(stderr, " render_states=");
    first = true;
    for (i = 0; i < SVGA3D_RS_MAX; i++) {
        if (!context->render_state[i].valid) {
            continue;
        }
        fprintf(stderr, "%s%u=%08x", first ? "" : ",", i,
                context->render_state[i].value);
        first = false;
    }
    fprintf(stderr, "\n");
}

static bool vmsvga3d_state_draw_primitives(
    struct vmsvga_state_s *s, uint32_t cid, uint32_t vertex_decl_count,
    const SVGA3dVertexDecl *vertex_decls, uint32_t range_count,
    const SVGA3dPrimitiveRange *ranges, uint32_t divisor_count,
    const SVGA3dVertexDivisor *divisors)
{
    struct vmsvga3d_state_s *state = s->svga3d;
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    uint32_t i;

    if (state == NULL || context == NULL || vertex_decl_count == 0 ||
        vertex_decl_count > SVGA3D_MAX_VERTEX_ARRAYS || range_count == 0 ||
        range_count > SVGA3D_MAX_DRAW_PRIMITIVE_RANGES || vertex_decls == NULL ||
        ranges == NULL || (divisor_count != 0 && divisors == NULL) ||
        (divisor_count != 0 && divisor_count != vertex_decl_count) ||
        !vmsvga3d_draw_decls_valid(vertex_decls, vertex_decl_count) ||
        !vmsvga3d_draw_vertex_surfaces_valid(state, vertex_decls,
                                             vertex_decl_count) ||
        !vmsvga3d_draw_ranges_valid(state, ranges, range_count)) {
        return false;
    }

    for (i = 0; i < divisor_count; i++) {
        if (divisors[i].indexedData && divisors[i].instanceData) {
            return false;
        }
    }

    vmsvga3d_state_trace_texgen(s, context, cid);
    vmsvga3d_state_trace_draw_snapshot(s, context, cid);

    /* Protocol validation completes here; the FIFO handler executes D3D9. */
    return true;
}

static bool vmsvga3d_state_set_scissor(struct vmsvga_state_s *s,
                                          uint32_t cid,
                                          const SVGA3dRect *rect)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);

    if (context == NULL || rect == NULL) {
        return false;
    }

    if (!context->scissor_valid ||
        memcmp(&context->scissor, rect, sizeof(*rect)) != 0) {
        context->scissor = *rect;
        context->scissor_valid = true;
        context->legacy_scissor_dirty = true;
        context->legacy_pipeline_dirty |= VMSVGA3D_LEGACY_PIPELINE_DIRTY_FIXED;
    }

    return true;
}

static bool vmsvga3d_state_generate_mipmaps(struct vmsvga_state_s *s,
                                               uint32_t sid,
                                               SVGA3dTextureFilter filter)
{
    VMSVGA3DSurface *surface;

    if (s->svga3d == NULL || sid >= SVGA3D_MAX_SURFACE_IDS ||
        filter < SVGA3D_TEX_FILTER_MIN || filter >= SVGA3D_TEX_FILTER_MAX) {
        return false;
    }

    surface = s->svga3d->surfaces[sid];
    if (surface == NULL) {
        return false;
    }

    /* Keep the guest-visible autogen choice in the surface shadow.  The FIFO
     * execution layer applies it to the D3D9 texture and generates the mips. */
    surface->autogen_filter = filter;

    return true;
}

static bool vmsvga3d_state_query_begin(struct vmsvga_state_s *s,
                                       uint32_t cid,
                                       SVGA3dQueryType type)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DQuery *query;

    if (context == NULL || type != SVGA3D_QUERYTYPE_OCCLUSION) {
        return false;
    }

    query = &context->occlusion;
    query->defined = true;
    query->state = VMSVGA3D_QUERY_BUILDING;
    query->result = 0;

    return true;
}

static bool vmsvga3d_state_query_end(struct vmsvga_state_s *s,
                                     uint32_t cid,
                                     SVGA3dQueryType type)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DQuery *query;

    if (context == NULL || type != SVGA3D_QUERYTYPE_OCCLUSION) {
        return false;
    }

    query = &context->occlusion;
    if (!query->defined) {
        return false;
    }

    query->state = VMSVGA3D_QUERY_ISSUED;
    return true;
}

bool vmsvga3d_state_query_complete(struct vmsvga_state_s *s,
                                    uint32_t cid, SVGA3dQueryType type,
                                    uint32_t result)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DQuery *query;

    if (context == NULL || type != SVGA3D_QUERYTYPE_OCCLUSION) {
        return false;
    }

    query = &context->occlusion;
    if (!query->defined || query->state != VMSVGA3D_QUERY_ISSUED) {
        return false;
    }

    query->state = VMSVGA3D_QUERY_SIGNALED;
    query->result += result;

    return true;
}

static VMSVGA3DQueryWaitStatus vmsvga3d_state_query_wait(
    struct vmsvga_state_s *s, uint32_t cid, SVGA3dQueryType type,
    uint32_t *result)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DQuery *query;

    if (result != NULL) {
        *result = 0;
    }

    if (context == NULL || type != SVGA3D_QUERYTYPE_OCCLUSION) {
        return VMSVGA3D_QUERY_WAIT_FAILED;
    }

    query = &context->occlusion;

    if (!query->defined) {
        return VMSVGA3D_QUERY_WAIT_FAILED;
    }

    if (query->state == VMSVGA3D_QUERY_ISSUED) {
        return VMSVGA3D_QUERY_WAIT_NEEDS_RENDERER;
    }

    if (result != NULL) {
        *result = query->result;
    }

    return VMSVGA3D_QUERY_WAIT_READY;
}

static bool vmsvga3d_state_shader_define(
    struct vmsvga_state_s *s, uint32_t cid, uint32_t shid,
    SVGA3dShaderType type, uint32_t bytecode_size, const uint32_t *bytecode)
{
    struct vmsvga3d_state_s *state;
    VMSVGA3DContext *context;
    VMSVGA3DShader *shader = NULL;
    VMSVGA3DShader *old_shader;
    uint32_t type_index;
    size_t new_shader_bytes;

    context = vmsvga3d_context(s, cid);
    if (context == NULL || !vmsvga3d_shader_type_index(type, &type_index) ||
        shid >= SVGA3D_MAX_SHADERIDS || bytecode == NULL || bytecode_size == 0 ||
        bytecode_size > SVGA3D_MAX_SHADER_MEMORY_BYTES ||
        bytecode_size % sizeof(uint32_t) != 0) {
        return false;
    }

    state = s->svga3d;
    old_shader = context->shader[type_index][shid];
    new_shader_bytes = state->shader_bytes;

    if (old_shader != NULL) {
        if (new_shader_bytes < old_shader->bytecode_size) {
            return false;
        }
        new_shader_bytes -= old_shader->bytecode_size;
    }

    if (new_shader_bytes > SVGA3D_MAX_SHADER_MEMORY_BYTES ||
        bytecode_size > SVGA3D_MAX_SHADER_MEMORY_BYTES - new_shader_bytes) {
        return false;
    }

    shader = g_try_new0(VMSVGA3DShader, 1);

    if (shader != NULL) {
        shader->bytecode = g_try_malloc(bytecode_size);
    }

    if (shader == NULL || shader->bytecode == NULL) {
        vmsvga3d_shader_free(shader);
        return false;
    }

    shader->shid = shid;
    shader->type = type;
    shader->bytecode_size = bytecode_size;

    memcpy(shader->bytecode, bytecode, bytecode_size);

    if (old_shader != NULL) {
        vmsvga3d_shader_free(old_shader);
    }

    context->shader[type_index][shid] = shader;
    state->shader_bytes = new_shader_bytes + bytecode_size;
    if (context->bound_shader[type_index] == shid) {
        context->legacy_shader_dirty |= UINT32_C(1) << type_index;
        context->legacy_pipeline_dirty |=
            VMSVGA3D_LEGACY_PIPELINE_DIRTY_SHADERS;
    }

    return true;
}

static bool vmsvga3d_state_shader_destroy(struct vmsvga_state_s *s,
                                             uint32_t cid, uint32_t shid,
                                             SVGA3dShaderType type)
{
    struct vmsvga3d_state_s *state = s->svga3d;
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DShader *shader;
    uint32_t type_index;

    if (state == NULL || context == NULL ||
        !vmsvga3d_shader_type_index(type, &type_index) ||
        shid >= SVGA3D_MAX_SHADERIDS) {
        return false;
    }

    shader = context->shader[type_index][shid];
    if (shader == NULL) {
        return true;
    }

    if (state->shader_bytes >= shader->bytecode_size) {
        state->shader_bytes -= shader->bytecode_size;
    } else {
        state->shader_bytes = 0;
    }

    vmsvga3d_shader_free(shader);
    context->shader[type_index][shid] = NULL;
    if (context->bound_shader[type_index] == shid) {
        context->legacy_shader_dirty |= UINT32_C(1) << type_index;
        context->legacy_pipeline_dirty |=
            VMSVGA3D_LEGACY_PIPELINE_DIRTY_SHADERS;
    }

    return true;
}

static bool vmsvga3d_state_set_shader(struct vmsvga_state_s *s,
                                         uint32_t cid,
                                         SVGA3dShaderType type,
                                         uint32_t shid)
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    uint32_t type_index;

    if (context == NULL || !vmsvga3d_shader_type_index(type, &type_index)) {
        return false;
    }

    if (context->bound_shader[type_index] != shid) {
        context->bound_shader[type_index] = shid;
        context->legacy_shader_dirty |= UINT32_C(1) << type_index;
        context->legacy_pipeline_dirty |=
            VMSVGA3D_LEGACY_PIPELINE_DIRTY_SHADERS;
    }
    if (shid == SVGA3D_INVALID_ID) {
        return true;
    }

    if (shid >= SVGA3D_MAX_SHADERIDS ||
        context->shader[type_index][shid] == NULL) {
        return false;
    }

    return true;
}

static bool vmsvga3d_state_set_shader_const(
    struct vmsvga_state_s *s, uint32_t cid, uint32_t reg,
    SVGA3dShaderType type, SVGA3dShaderConstType ctype, uint32_t count,
    const uint32_t (*values)[4])
{
    VMSVGA3DContext *context = vmsvga3d_context(s, cid);
    VMSVGA3DShaderConstant *constants;
    uint32_t limit;
    uint32_t type_index;
    uint32_t i;

    limit = vmsvga3d_shader_const_limit(ctype);
    if (context == NULL || limit == 0 || count == 0 || values == NULL ||
        !vmsvga3d_shader_type_index(type, &type_index) || reg >= limit ||
        count > limit - reg) {
        return false;
    }

    constants = vmsvga3d_shader_const_array(context, type_index, ctype);
    if (constants == NULL) {
        return false;
    }

    for (i = 0; i < count; i++) {
        uint32_t slot = reg + i;

        if (!constants[slot].valid ||
            memcmp(constants[slot].values, values[i],
                   sizeof(constants[slot].values)) != 0) {
            memcpy(constants[slot].values, values[i],
                   sizeof(constants[slot].values));
            constants[slot].valid = true;
            switch (ctype) {
            case SVGA3D_CONST_TYPE_FLOAT:
                context->legacy_shader_float_dirty[type_index][slot / 64u] |=
                    UINT64_C(1) << (slot % 64u);
                break;
            case SVGA3D_CONST_TYPE_INT:
                context->legacy_shader_int_dirty[type_index][slot / 64u] |=
                    UINT64_C(1) << (slot % 64u);
                break;
            case SVGA3D_CONST_TYPE_BOOL:
                context->legacy_shader_bool_dirty[type_index][slot / 64u] |=
                    UINT64_C(1) << (slot % 64u);
                break;
            default:
                return false;
            }
            context->legacy_pipeline_dirty |=
                VMSVGA3D_LEGACY_PIPELINE_DIRTY_SHADERS;
        }
    }

    return true;
}
