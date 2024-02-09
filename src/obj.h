// Split string by delimiter
std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> result;

    size_t index;
    while((index = str.find(delimiter)) != std::string::npos) {
        result.push_back(str.substr(0, index));

        str = str.substr(index + 1);
    }

    if(str.size()) result.push_back(str);

    return result;
}

// Load vertex specification
void load_vertex(std::vector<point3d>& vertices, double x, double y, double z) {
    vertices.push_back(point3d(x, y, z));
}

// Load face specification
void load_face(std::vector<triangle>& result, std::vector<point3d>& vertices,
               int i, int j, int k) {
    result.push_back(triangle(vertices[i - 1], vertices[j - 1], vertices[k - 1]));
}

// Load obj file
std::vector<triangle> load_obj(const std::string& filename) {
    std::vector<triangle> result;
    std::vector<point3d> vertices;

    std::fstream file(filename);
    if(file) {
        std::string line;

        while(getline(file, line)) {
            auto parameters = split(line, ' ');
            if(parameters.size()) {
                if(parameters[0] == "v" && parameters.size() == 4) {
                    load_vertex(vertices,
                                std::stod(parameters[1]), std::stod(parameters[2]), std::stod(parameters[3]));
                }

                if(parameters[0] == "f" && parameters.size() == 4) {
                    load_face(result, vertices,
                              std::stoi(parameters[1]), std::stoi(parameters[2]), std::stoi(parameters[3]));
                }
            }
        }
    }

    return result;
}
