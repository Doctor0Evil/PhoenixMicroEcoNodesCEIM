#include "ceim_node.hpp"
#include "safety_corridor.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

void compute_ceim_and_karma(CeimNode& node); // from ceim_node.cpp

static std::vector<CeimNode> load_nodes(const std::string& path) {
    std::vector<CeimNode> nodes;
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Failed to open input CSV: " << path << "\n";
        return nodes;
    }

    std::string line;
    // Skip header
    if (!std::getline(in, line)) {
        return nodes;
    }

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string field;

        CeimNode node;

        std::getline(ss, node.node_id, ',');
        std::getline(ss, node.medium, ',');
        std::getline(ss, node.region, ',');
        std::getline(ss, field, ','); node.latitude_deg  = std::stod(field);
        std::getline(ss, field, ','); node.longitude_deg = std::stod(field);
        std::getline(ss, node.parameter, ',');
        std::getline(ss, field, ','); node.cin  = std::stod(field);
        std::getline(ss, field, ','); node.cout = std::stod(field);
        std::getline(ss, node.unit, ',');
        std::getline(ss, field, ','); node.qavg = std::stod(field);
        std::getline(ss, node.qunit, ',');
        std::getline(ss, field, ','); node.horizon_s = std::stod(field);
        std::getline(ss, field, ','); node.ecoimpact_score_raw = std::stod(field);
        std::getline(ss, field, ','); node.karma_per_unit = std::stod(field);
        std::getline(ss, node.notes);

        node.mass_avoided_kg = 0.0;
        node.karma_raw = 0.0;
        node.ecoimpact_score_adj = 0.0;
        node.karma_adj = 0.0;

        nodes.push_back(node);
    }
    return nodes;
}

static void write_output(const std::string& path, const std::vector<CeimNode>& nodes) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Failed to open output CSV: " << path << "\n";
        return;
    }

    out << "nodeid,medium,parameter,cin,cout,unit,qavg,qunit,horizon_s,"
        << "mass_avoided_kg,karma_raw,ecoimpact_score_raw,"
        << "safety_score,within_human_corridor,within_wildlife_corridor,"
        << "ecoimpact_score_adj,karma_adj,notes\n";

    for (const auto& node : nodes) {
        SafetyAssessment sa = evaluate_safety_corridors(node);

        out << node.node_id << ","
            << node.medium << ","
            << node.parameter << ","
            << node.cin << ","
            << node.cout << ","
            << node.unit << ","
            << node.qavg << ","
            << node.qunit << ","
            << node.horizon_s << ","
            << node.mass_avoided_kg << ","
            << node.karma_raw << ","
            << node.ecoimpact_score_raw << ","
            << sa.safety_score << ","
            << (sa.within_human_corridor ? "true" : "false") << ","
            << (sa.within_wildlife_corridor ? "true" : "false") << ","
            << node.ecoimpact_score_adj << ","
            << node.karma_adj << ","
            << "\"" << node.notes << "\""
            << "\n";
    }
}

int main() {
    const std::string input_path  = "data/PhoenixMicroEcoNodes2026v1.csv";
    const std::string output_path = "qpudatashards/CEIMKarmaPhoenixMicroNodes_2026-01-28.csv";

    auto nodes = load_nodes(input_path);
    for (auto& node : nodes) {
        compute_ceim_and_karma(node);
    }
    write_output(output_path, nodes);

    std::cout << "Processed " << nodes.size()
              << " nodes into " << output_path << "\n";
    return 0;
}
