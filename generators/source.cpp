
// ------------------------------------------------

#include "parameters.hpp"

// ------------------------------------------------

void generateParameters(std::vector<std::string_view>& args) {

    // ------------------------------------------------

    if (args.size() != 5) return;

    // ------------------------------------------------

    std::filesystem::path inputPath{ args[2] };
    std::filesystem::path parameterPath{ args[3] };
    std::filesystem::path assignersPath{ args[4] };

    // ------------------------------------------------

    if (!std::filesystem::exists(parameterPath.parent_path())) {
        std::filesystem::create_directory(parameterPath.parent_path());
    }

    if (!std::filesystem::exists(assignersPath.parent_path())) {
        std::filesystem::create_directory(assignersPath.parent_path());
    }

    // ------------------------------------------------

    std::ifstream ifile{ inputPath };
    std::ofstream pfile{ parameterPath };
    std::ofstream afile{ assignersPath };

    // ------------------------------------------------

    using namespace Kaixo::Generator;

    // ------------------------------------------------

    if (auto xml = basic_xml::parse(file_to_string(ifile))) {
        ParameterGenerator generator;

        generator.generate(xml.value());

        pfile << generator.parametersAsString();
        afile << generator.assignersAsString();
    }
    else {
        std::cerr << "Failed to parse XML file [" << inputPath << "]\n";
    }

    // ------------------------------------------------

}

// ------------------------------------------------

int main(const int argc, char const* const* const argv) {

    // ------------------------------------------------

    std::vector<std::string_view> args{ argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)) };

    if (args.size() < 2) return 1;

    // Generate parameters
    if (args[1] == "parameters") {
        generateParameters(args);
    }


    return 0;
}