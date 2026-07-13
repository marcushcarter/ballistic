#include <core/rendering/render_graph_profiler.h>
#include <core/log/error_macros.h>
#include <algorithm>

namespace ballistic {

/***************/
/**** SETUP ****/
/***************/

Error RenderGraphProfiler::create(drivers::DeviceDriverVulkan& r_dd, uint32_t p_frame_count)
{
    using enum Error;

    dd = &r_dd;
    period_ns = dd->physical_device_properties.limits.timestampPeriod;

    uint32_t valid_bits = dd->timestamp_valid_bits(dd->cd->graphics_queue_family);
    if (valid_bits == 0 || period_ns == 0.0) {
        supported = false;
        enabled = active = prev_active = false;
        log_write("RenderGraphProfiler: GPU timing unsupported (validBits=%u, period=%f).", valid_bits, period_ns);
        return Ok;
    }

    supported = true;
    valid_mask = (valid_bits >= 64) ? ~0ull : ((1ull << valid_bits) - 1);

    slots.resize(p_frame_count);
    for (Slot& s : slots) {
        s.pool = dd->query_pool_create_timestamp(CAPACITY);
        s.marks.reserve(256);
    }

    raw_scratch.resize(CAPACITY);
    return Ok;
}

void RenderGraphProfiler::destroy()
{
    for (Slot& s : slots) dd->query_pool_free(s.pool);
    slots.clear();
    raw_scratch.clear();
    name_table.clear();
    _clear_results();
    supported = false;
    enabled = active = prev_active = false;
}

/***************/
/**** NAMES ****/
/***************/

uint64_t RenderGraphProfiler::intern(std::string_view p_s)
{
    uint64_t h = 1469598103934665603ull;
    for (char c : p_s) { h ^= static_cast<uint8_t>(c); h *= 1099511628211ull; }
    return h;
}

uint64_t RenderGraphProfiler::intern_named(std::string_view p_s)
{
    uint64_t id = intern(p_s);
    if (!name_table.contains(id)) name_table.emplace(id, std::string(p_s));
    return id;
}

uint64_t RenderGraphProfiler::draw_key(uint64_t p_pass_name_id, uint64_t p_name_id, uint32_t p_occurrence)
{
    uint64_t h = p_pass_name_id;
    h ^= p_name_id + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
    h ^= static_cast<uint64_t>(p_occurrence) + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
    return h;
}

const char* RenderGraphProfiler::name_of(uint64_t p_id) const
{
    auto it = name_table.find(p_id);
    return (it != name_table.end()) ? it->second.c_str() : "?";
}

/*******************/
/**** RECORDING ****/
/*******************/

void RenderGraphProfiler::_clear_results()
{
    results.clear();
    smoothed.clear();
    total_ms = 0.0;
    total_draws = 0;
    truncated = false;
    last_query_count = 0;
}

bool RenderGraphProfiler::_write_boundary(VkCommandBuffer p_cmd, uint32_t& r_index)
{
    Slot& s = slots[slot];
    if (s.query_count >= s.reset_count) { s.overflowed = true; return false; }
    r_index = s.query_count++;
    dd->command_write_timestamp(p_cmd, s.pool, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, r_index);
    return true;
}

void RenderGraphProfiler::_resolve()
{
    using enum Error;
    if (!active) return;

    Slot& s = slots[slot];
    if (!s.recorded) return;
    if (s.query_count < 2 || s.marks.empty()) { results.clear(); total_ms = 0.0; total_draws = 0; return; }

    if (dd->query_pool_get_results(s.pool, 0, s.query_count, raw_scratch.data()) != Ok) return;

    last_query_count = s.query_count;
    truncated = s.overflowed;

    results.clear();
    results.reserve(s.marks.size());

    auto delta_ms = [&](uint32_t p_a, uint32_t p_b) -> double {
        if (p_a == INVALID || p_b == INVALID) return 0.0;
        uint64_t a = raw_scratch[p_a] & valid_mask;
        uint64_t b = raw_scratch[p_b] & valid_mask;
        return (b >= a ? double(b - a) : 0.0) * period_ns * 1e-6;
    };

    double sum = 0.0;
    uint32_t draws = 0;

    for (const Mark& m : s.marks) {
        const double gap = delta_ms(m.lead_query,  m.begin_query);
        const double dur = delta_ms(m.begin_query, m.end_query);

        auto [it, fresh] = smoothed.try_emplace(m.key, Smoothed{ gap, dur });
        Smoothed& acc = it->second;
        if (!fresh) {
            acc.gap += smoothing * (gap - acc.gap);
            acc.dur += smoothing * (dur - acc.dur);
        }

        if (m.kind == MarkKind::Pass) sum += acc.gap + acc.dur;
        else draws++;

        Timing t;
        t.key = m.key;
        t.name = (m.name_id != 0) ? name_of(m.name_id) : "";
        t.type = (m.type_id != 0) ? name_of(m.type_id) : "";
        t.category = (m.kind == MarkKind::Pass) ? name_of(m.cat_id) : "";
        t.gap_ms = acc.gap;
        t.gpu_ms = acc.dur;
        t.raw_gap_ms = gap;
        t.raw_ms = dur;
        t.draw_count = m.draw_count;
        t.ordinal = m.ordinal;
        t.parent = m.parent;
        t.kind = m.kind;
        results.push_back(t);
    }

    total_ms = sum;
    total_draws = draws;
}

void RenderGraphProfiler::frame_begin(VkCommandBuffer p_cmd, uint32_t p_slot)
{
    slot = p_slot;

    _resolve();

    prev_active = active;
    active = supported && enabled;

    if (prev_active && !active) {
        _clear_results();
        for (Slot& s : slots) s.recorded = false;
    }
    if (!active) return;

    Slot& s = slots[slot];
    s.marks.clear();
    s.query_count = 0;
    s.open_pass = INVALID;
    s.open_draw = INVALID;
    s.pass_ordinal = 0;
    s.recorded = false;
    s.overflowed = false;

    uint32_t want = std::max(RESET_MIN, last_query_count * 2);
    s.reset_count = std::min(CAPACITY, want);

    dd->command_reset_query_pool(p_cmd, s.pool, 0, s.reset_count);

    uint32_t t0 = 0;
    _write_boundary(p_cmd, t0);
    s.last_boundary = t0;
}

void RenderGraphProfiler::frame_end()
{
    if (!active) return;
    slots[slot].recorded = true;
}

void RenderGraphProfiler::pass_begin(VkCommandBuffer p_cmd, std::string_view p_name, std::string_view p_category)
{
    if (!active) return;
    Slot& s = slots[slot];

    Mark m;
    m.name_id = intern_named(p_name);
    m.cat_id = intern_named(p_category);
    m.key = m.name_id;
    m.lead_query = s.last_boundary;
    m.kind = MarkKind::Pass;

    uint32_t ts;
    if (_write_boundary(p_cmd, ts)) {
        m.begin_query = ts;
        s.last_boundary = ts;
    }

    s.open_pass = static_cast<uint32_t>(s.marks.size());
    s.open_draw = INVALID;
    s.pass_ordinal = 0;
    s.name_occurrence.clear();
    s.marks.push_back(m);
}

void RenderGraphProfiler::pass_end(VkCommandBuffer p_cmd, uint32_t p_draw_count)
{
    if (!active || slots[slot].open_pass == INVALID) return;
    Slot& s = slots[slot];
    Mark& m = s.marks[s.open_pass];

    m.draw_count = p_draw_count;

    uint32_t ts;
    if (_write_boundary(p_cmd, ts)) {
        m.end_query = ts;
        s.last_boundary = ts;
    }
    s.open_pass = INVALID;
    s.open_draw = INVALID;
}

void RenderGraphProfiler::draw_begin(VkCommandBuffer p_cmd, std::string_view p_name, std::string_view p_type)
{
    if (!active) return;
    Slot& s = slots[slot];
    if (s.open_pass == INVALID) return;

    const uint64_t pass_name_id = s.marks[s.open_pass].name_id;

    uint32_t occurrence;
    const uint64_t name_id = p_name.empty() ? 0 : intern_named(p_name);
    if (name_id != 0) occurrence = s.name_occurrence[name_id]++;
    else occurrence = s.pass_ordinal;
    
    const uint64_t type_id = p_name.empty() ? 0 : intern_named(p_type);

    Mark m;
    m.name_id = name_id;
    m.type_id = type_id;
    m.ordinal = s.pass_ordinal++;
    m.key = draw_key(pass_name_id, name_id, occurrence);
    m.lead_query = s.last_boundary;
    m.parent = s.open_pass;
    m.draw_count = 1;
    m.kind = MarkKind::Draw;

    uint32_t ts;
    if (!_write_boundary(p_cmd, ts)) {
        s.open_draw = INVALID;
        return;
    }
    m.begin_query = ts;

    s.open_draw = static_cast<uint32_t>(s.marks.size());
    s.marks.push_back(m);
}

void RenderGraphProfiler::draw_end(VkCommandBuffer p_cmd)
{
    if (!active || slots[slot].open_draw == INVALID) return;
    Slot& s = slots[slot];
    Mark& m = s.marks[s.open_draw];
    uint32_t ts;
    if (_write_boundary(p_cmd, ts)) {
        m.end_query = ts;
        s.last_boundary = ts;
    }
    s.open_draw = INVALID;
}

}