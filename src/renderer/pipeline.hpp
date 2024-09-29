#pragma once

#include <string>
#include <vector>

namespace CGEngine {

    class Pipeline {
    public:
        Pipeline(const std::string& vertFilepath, const std::string& fragFilepath);

    private:
        static std::vector<char> readFile(const std::string& filename);

        void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
    };
}