#ifndef SAFETY_CORRIDOR_HPP
#define SAFETY_CORRIDOR_HPP

#include "ceim_node.hpp"

// Encodes simple, conservative safety corridors for humans and wildlife.
// Output is a scalar S in [0,1] that down-scales ecoimpact and Karma.
// S = 1.0 means all safety corridors are respected.
// S < 1.0 means some risk indicators are present.
// S = 0.0 can be reserved for hard violations (deployment not allowed).

struct SafetyAssessment {
    double safety_score;          // S in [0,1]
    bool   within_human_corridor;
    bool   within_wildlife_corridor;
    std::string reason;           // short explanation for logs/shards
};

SafetyAssessment evaluate_safety_corridors(const CeimNode& node);

#endif // SAFETY_CORRIDOR_HPP
