#ifndef CEIM_NODE_HPP
#define CEIM_NODE_HPP

#include <string>

struct CeimNode {
    // Identification and location
    std::string node_id;
    std::string medium;     // water, air, solid, land
    std::string region;
    double latitude_deg;
    double longitude_deg;

    // Parameter and concentrations
    std::string parameter;  // TP, CO2, plastics, etc.
    double cin;             // inlet concentration or baseline level
    double cout;            // outlet concentration or mitigated level
    std::string unit;       // mg/L, ppm, kg/day, etc.

    // Flow / throughput and horizon
    double qavg;            // average flow or rate
    std::string qunit;      // m3/s, m3/h, kg/s, etc.
    double horizon_s;       // integration horizon in seconds

    // Pre-assigned eco-impact score and karma scaling
    double ecoimpact_score_raw;   // 0-1, from shard
    double karma_per_unit;        // scaling, e.g., 0.67 * tons avoided

    // Free-text notes
    std::string notes;

    // Derived quantities (computed by CEIM kernel)
    double mass_avoided_kg;       // pollutant mass avoided over horizon
    double karma_raw;             // raw Karma before safety scaling

    // Safety-aware outputs (filled after safety checks)
    double ecoimpact_score_adj;   // adjusted by safety factor
    double karma_adj;             // Karma scaled by safety factor
};

#endif // CEIM_NODE_HPP
