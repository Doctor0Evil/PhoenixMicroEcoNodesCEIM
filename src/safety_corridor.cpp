#include "safety_corridor.hpp"
#include <algorithm>

namespace {

// Helper: clamp to [0,1]
double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

}

// Very conservative, explainable rules.
// These do NOT control any machine directly, they just gate credit.
// For actual devices, you would mirror these corridors in device firmware.

SafetyAssessment evaluate_safety_corridors(const CeimNode& node) {
    SafetyAssessment result;
    result.safety_score = 1.0;
    result.within_human_corridor = true;
    result.within_wildlife_corridor = true;
    result.reason = "All safety corridors respected.";

    // Rule 1: Prevent credit for extreme physical changes in human spaces.
    // Example: heat-index nodes must not claim benefit from pushing temps
    // into unsafe zones.
    if (node.medium == "land" && node.parameter == "heat-index") {
        // Assume safe human corridor for peak surface temp roughly 35-45 C.
        if (node.cout < 30.0 || node.cout > 45.0) {
            result.safety_score *= 0.0;
            result.within_human_corridor = false;
            result.reason = "Heat-index cout outside conservative human safety corridor.";
        }
    }

    // Rule 2: Flow-based nodes near people or wildlife: cap per-node hydraulic change.
    // For micro-nodes in this shard, qavg is already very small, but we still apply a ceiling.
    if (node.medium == "water" || node.medium == "air") {
        // Example corridor: qavg for small local nodes should be below a threshold,
        // otherwise devices might induce harmful velocities or turbulence.
        const double MAX_LOCAL_Q_WATER = 0.05;   // m3/s, small bioswale-scale
        const double MAX_LOCAL_Q_AIR   = 50000;  // m3/h, small building-scale fan

        if (node.medium == "water" && node.qunit == "m3/s" && node.qavg > MAX_LOCAL_Q_WATER) {
            result.safety_score *= 0.2;
            result.within_wildlife_corridor = false;
            result.reason = "Water flow above conservative bioswale corridor.";
        }
        if (node.medium == "air" && node.qunit == "m3/h" && node.qavg > MAX_LOCAL_Q_AIR) {
            result.safety_score *= 0.5;
            result.within_human_corridor = false;
            result.reason = "Air flow above conservative indoor comfort corridor.";
        }
    }

    // Rule 3: No credit for designs that imply physical intrusion into habitats.
    // For now, we approximate this by medium + notes keyword checks.
    if (node.medium == "water" && node.notes.find("canal") != std::string::npos) {
        // Canal nodes must be explicitly designed to avoid sharp edges / debris traps;
        // until that evidence exists, we down-weight.
        result.safety_score *= 0.7;
        result.reason = "Canal-related node without explicit wildlife-safe evidence.";
        result.within_wildlife_corridor = false;
    }

    // Rule 4: Household / human-adjacent nodes must not increase acute risk.
    if (node.node_id.find("HH-") != std::string::npos) {
        // If some future version had cout > cin for a harmful parameter,
        // safety_score would go to zero.
        if (node.cout > node.cin) {
            result.safety_score = 0.0;
            result.reason = "Household node increases harmful parameter; no credit allowed.";
            result.within_human_corridor = false;
        }
    }

    result.safety_score = clamp01(result.safety_score);
    return result;
}
