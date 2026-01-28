#ifndef WETLAND_TYPES_HPP
#define WETLAND_TYPES_HPP

#include <string>

namespace econet {

struct WetlandConfig {
    std::string nodeId;
    std::string region;
    double latitude;   // deg
    double longitude;  // deg
    double cin_mgL;    // inflow TP
    double cout_mgL;   // outflow TP
    double q_m3s;      // design average flow
    double horizon_s;  // design horizon (e.g., 31536000 for 1 year)
    double cref_mgL;   // reference benchmark (e.g., 0.10 mg/L)
    double karma_per_unit; // scaling factor (e.g., 2.0e5)
    // Simple safety parameters
    double maxPondDepth_m;
    double sideSlopeHtoV;
};

struct WetlandResult {
    std::string nodeId;
    double m_avoided_kg;     // annual mass avoided
    double ecoimpactscore;   // 0–1
    double karma;            // scaled Karma
    double safetyscore;      // 0–1
};

} // namespace econet

#endif
