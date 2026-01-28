#include "WetlandKernel.hpp"
#include <algorithm>

namespace econet {

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

// Simple safety gate: full credit only if depth and slope are in safe corridors.
static double computeSafetyScore(const WetlandConfig& cfg) {
    // Example corridors: depth <= 0.45 m, slope >= 3:1 (H:V)
    double s_depth = (cfg.maxPondDepth_m <= 0.45) ? 1.0 : 0.0;
    double s_slope = (cfg.sideSlopeHtoV >= 3.0) ? 1.0 : 0.0;
    return 0.5 * (s_depth + s_slope);
}

WetlandResult evaluateWetland(const WetlandConfig& cfg) {
    WetlandResult out{};
    out.nodeId = cfg.nodeId;

    // Mass-avoided kernel (TP), mg/L -> kg via 1e-6 factor, assuming density ~ water.
    const double deltaC_mgL = std::max(0.0, cfg.cin_mgL - cfg.cout_mgL);
    const double m_kg = deltaC_mgL * 1e-6 * cfg.q_m3s * cfg.horizon_s;

    out.m_avoided_kg = m_kg;

    // ecoimpactscore: normalized vs a notional "good" mass target (can calibrate later).
    const double B_min = 0.0;
    const double B_max = std::max(1.0, m_kg * 2.0); // placeholder, to be calibrated
    out.ecoimpactscore = clamp01((m_kg - B_min) / (B_max - B_min));

    // Safety gating
    out.safetyscore = computeSafetyScore(cfg);

    // Karma: CEIM-style scaling, gated by safety.
    const double rawKarma = cfg.karma_per_unit * m_kg;
    out.karma = rawKarma * out.safetyscore;

    return out;
}

} // namespace econet
