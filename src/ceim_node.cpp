#include "ceim_node.hpp"
#include "safety_corridor.hpp"
#include <cmath>

// Convert various units into kg avoided over the horizon.
// This is deliberately simple and conservative. For Phoenix water/air nodes
// you can align coefficients with your existing CEIM/qpudatashards later.

static double mass_avoided_kg_from_node(const CeimNode& node) {
    const double delta = node.cin - node.cout;
    if (delta <= 0.0) {
        return 0.0;
    }

    // Case 1: concentration * flow * time (e.g., mg/L * m3/s * s)
    if (node.unit == "mg/L" && node.qunit == "m3/s") {
        double mass_mg = delta * node.qavg * node.horizon_s; // mg/s * s
        return mass_mg * 1e-6; // mg -> kg
    }

    // Case 2: ppm in air with volumetric flow (very coarse, ideal-gas-equivalent).
    if (node.unit == "ppm" && (node.qunit == "m3/h" || node.qunit == "m3/s")) {
        // Use a fixed coefficient representing kg CO2 per ppm*m3 at ~25 C, 1 atm.
        // This is approximate but consistent with DAC/CEIM practices you already use.
        const double kg_per_ppm_m3 = 1.9e-6; // kg / (ppm * m3)
        double q_m3 = node.qavg * (node.qunit == "m3/h" ? node.horizon_s / 3600.0 : node.horizon_s);
        return delta * q_m3 * kg_per_ppm_m3;
    }

    // Case 3: direct mass-rate parameters (kg/day or kg/s)
    if (node.unit == "kg/day") {
        double mass_kg = delta * (node.horizon_s / 86400.0);
        return mass_kg;
    }
    if (node.unit == "kg/s") {
        double mass_kg = delta * node.horizon_s;
        return mass_kg;
    }

    // Case 4: non-mass parameters like heat-index: treat as 0 mass directly,
    // but they can still carry ecoimpact via your existing ecoimpact_score_raw.
    return 0.0;
}

// Compute raw Karma and then apply safety scaling.
void compute_ceim_and_karma(CeimNode& node) {
    node.mass_avoided_kg = mass_avoided_kg_from_node(node);

    // Map mass to raw Karma using your 0.67 factor (per metric ton) where applicable.
    const double tons_avoided = node.mass_avoided_kg / 1000.0;
    node.karma_raw = tons_avoided * 0.67;

    // Combine with node's existing ecoimpact scaling, if any.
    // This keeps your previous ecoimpact_score_raw in the loop.
    const double eco_scale = (node.ecoimpact_score_raw <= 0.0) ? 1.0 : node.ecoimpact_score_raw;
    node.karma_raw *= eco_scale;

    // Safety gating
    SafetyAssessment sa = evaluate_safety_corridors(node);
    node.ecoimpact_score_adj = node.ecoimpact_score_raw * sa.safety_score;
    node.karma_adj           = node.karma_raw           * sa.safety_score;
}
