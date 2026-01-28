#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "WetlandTypes.hpp"
#include "WetlandKernel.hpp"

using econet::WetlandConfig;
using econet::WetlandResult;
using econet::evaluateWetland;

static bool parseHeader(const std::string& line) {
    // Simple sanity check; could be extended to enforce exact order.
    return line.find("nodeid") != std::string::npos;
}

static bool parseConfigRow(const std::string& line, WetlandConfig& cfg) {
    std::stringstream ss(line);
    std::string field;
    std::vector<std::string> cols;

    while (std::getline(ss, field, ',')) {
        cols.push_back(field);
    }
    if (cols.size() < 12) {
        return false;
    }

    try {
        cfg.nodeId         = cols[0];
        cfg.region         = cols[1];
        cfg.latitude       = std::stod(cols[2]);
        cfg.longitude      = std::stod(cols[3]);
        cfg.cin_mgL        = std::stod(cols[4]);
        cfg.cout_mgL       = std::stod(cols[5]);
        cfg.q_m3s          = std::stod(cols[6]);
        cfg.horizon_s      = std::stod(cols[7]);
        cfg.cref_mgL       = std::stod(cols[8]);
        cfg.karma_per_unit = std::stod(cols[9]);
        cfg.maxPondDepth_m = std::stod(cols[10]);
        cfg.sideSlopeHtoV  = std::stod(cols[11]);
    } catch (const std::exception&) {
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    std::string inputPath = "qpudatashards/particles/PhoenixMicroWetlandsDesign2026v1.csv";
    std::string outputPath = "qpudatashards/particles/PhoenixMicroWetlandsResults2026v1.csv";

    if (argc > 1) {
        inputPath = argv[1];
    }
    if (argc > 2) {
        outputPath = argv[2];
    }

    std::ifstream in(inputPath);
    if (!in) {
        std::cerr << "Error: cannot open input CSV: " << inputPath << "\n";
        return 1;
    }

    std::string line;
    if (!std::getline(in, line)) {
        std::cerr << "Error: empty input file.\n";
        return 1;
    }

    if (!parseHeader(line)) {
        std::cerr << "Warning: header does not match expected format.\n";
    }

    std::vector<WetlandResult> results;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        WetlandConfig cfg;
        if (!parseConfigRow(line, cfg)) {
            std::cerr << "Skipping malformed row: " << line << "\n";
            continue;
        }
        WetlandResult res = evaluateWetland(cfg);
        results.push_back(res);
    }

    in.close();

    std::ofstream out(outputPath);
    if (!out) {
        std::cerr << "Error: cannot open output CSV: " << outputPath << "\n";
        return 1;
    }

    out << "nodeid,m_avoided_kg,ecoimpactscore,safetyscore,karma\n";
    for (const auto& r : results) {
        out << r.nodeId << ","
            << r.m_avoided_kg << ","
            << r.ecoimpactscore << ","
            << r.safetyscore << ","
            << r.karma << "\n";
    }

    out.close();

    std::cout << "Processed " << results.size() << " wetland node(s).\n";
    std::cout << "Results written to " << outputPath << "\n";

    return 0;
}
