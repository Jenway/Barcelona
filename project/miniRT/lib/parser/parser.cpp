#include "parser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "bvh.hpp"
#include "cylinder.hpp"
#include "plane.hpp"
#include "sphere.hpp"

namespace Parser {

std::vector<std::string_view> split(std::string_view s, char delimiter)
{
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);
    while (end != std::string_view::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delimiter, start);
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

// A more robust argument count checker.
void check_arg_count(const std::vector<std::string>& tokens, const std::vector<size_t>& expected_counts, std::string_view type)
{
    for (size_t count : expected_counts) {
        if (tokens.size() == count)
            return; // Found a valid count
    }
    std::string expected_str;
    for (size_t i = 0; i < expected_counts.size(); ++i) {
        expected_str += std::to_string(expected_counts[i]) + (i < expected_counts.size() - 1 ? " or " : "");
    }
    throw std::runtime_error("Invalid number of arguments for " + std::string(type) + ". Expected " + expected_str + " arguments.");
}

// Parses a Vec3 from a "x,y,z" string_view.
Vec3 parse_vec3(std::string_view s)
{
    auto components = split(s, ',');
    if (components.size() != 3) {
        throw std::runtime_error("Vector must have 3 components (x,y,z).");
    }
    return { convert_string<double>(components[0]), convert_string<double>(components[1]), convert_string<double>(components[2]) };
}

// Parses a color from "R,G,B" and normalizes it.
Vec3 parse_color(std::string_view s)
{
    auto components = split(s, ',');
    if (components.size() != 3) {
        throw std::runtime_error("Color must have 3 components (R,G,B).");
    }
    auto r = convert_string<double>(components[0]);
    auto g = convert_string<double>(components[1]);
    auto b = convert_string<double>(components[2]);

    // If values look like 0-255 integers, normalize to 0-1.
    auto norm = [](double v) { return v > 1.0 ? v / 255.0 : v; };
    return { norm(r), norm(g), norm(b) };
}

// Parses specular parameters. Supports multiple formats:
//  - "ratio,shininess"                => gray specular (ratio in 0..1)
//  - "r,g,b,shininess"                => explicit RGB specular (r/g/b either 0..1 or 0..255)
std::pair<Vec3, double> parse_specular_params(std::string_view s)
{
    auto components = split(s, ',');
    if (components.size() == 2) {
        // "ratio,shininess" form
        auto ratio = convert_string<double>(components[0]);
        auto shininess = convert_string<double>(components[1]);
        return { Vec3(ratio, ratio, ratio), shininess };
    }
    if (components.size() == 4) {
        // "r,g,b,shininess" form
        auto r = convert_string<double>(components[0]);
        auto g = convert_string<double>(components[1]);
        auto b = convert_string<double>(components[2]);
        auto shininess = convert_string<double>(components[3]);
        auto normalize_if_needed = [](double v) { return v > 1.0 ? v / 255.0 : v; };
        return { Vec3(normalize_if_needed(r), normalize_if_needed(g), normalize_if_needed(b)), shininess };
    }
    throw std::runtime_error("Invalid specular format. Expected 'ratio,shininess' or 'r,g,b,shininess'.");
}

FileParser::FileParser(const std::string& filename)
    : m_filename(filename)
{
    m_file.open(filename);
    if (!m_file.is_open()) {
        throw std::runtime_error("Error: Could not open file: " + filename);
    }
}

Scene FileParser::parse()
{
    std::string line;
    while (std::getline(m_file, line)) {
        m_line_num++;
        try {
            process_line(line);
        } catch (const std::exception& e) {
            // Re-throw as a detailed ParsingError
            throw ParsingError(m_filename, m_line_num, e.what());
        }
    }

    if (!m_scene.has_camera) {
        throw ParsingError(m_filename, m_line_num, "Scene file must contain a camera ('C').");
    }
    if (!m_scene.has_ambient_light) {
        throw ParsingError(m_filename, m_line_num, "Scene file must contain ambient light ('A').");
    }

    // Build the BVH from the collected objects
    if (!m_objects.empty()) {
        m_scene.world = bvh_node_from_objects(m_objects, 0, m_objects.size());
    }

    return m_scene;
}

void FileParser::process_line(const std::string& line)
{
    std::string trimmed_line;
    // Trim leading whitespace
    size_t first = line.find_first_not_of(" \t\n\r");
    if (std::string::npos == first)
        return; // Line is empty or whitespace

    // Trim trailing comments and whitespace
    size_t last = line.find_last_not_of(" \t\n\r");
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos && comment_pos < last) {
        last = line.find_last_not_of(" \t\n\r", comment_pos - 1);
    }
    if (std::string::npos == last)
        return; // Only a comment on the line

    trimmed_line = line.substr(first, (last - first + 1));

    if (trimmed_line.empty())
        return;

    std::vector<std::string> tokens; // Use std::string, NOT std::string_view
    std::stringstream ss(trimmed_line);
    std::string segment;

    while (ss >> segment) {
        tokens.push_back(segment); // This is now safe, it copies the string.
    }

    if (!tokens.empty()) {
        parse_line_tokens(tokens);
    }
}

void FileParser::parse_line_tokens(const std::vector<std::string>& tokens)
{
    const auto& type = tokens[0];
    if (type == "#")
        return; // Ignore comments
    if (type == "A")
        parse_ambient(tokens);
    else if (type == "C")
        parse_camera(tokens);
    else if (type == "L")
        parse_light(tokens);
    else if (type == "sp")
        parse_sphere(tokens);
    else if (type == "pl")
        parse_plane(tokens);
    else if (type == "cy")
        parse_cylinder(tokens);
    else {
        throw std::runtime_error("Unknown element type: " + std::string(type));
    }
}

// --- Element Parsing Methods ---

void FileParser::parse_ambient(const std::vector<std::string>& tokens)
{
    check_arg_count(tokens, { 3 }, "Ambient Light (A)");
    if (m_scene.has_ambient_light) {
        throw std::runtime_error("Ambient light can only be declared once.");
    }
    m_scene.ambient_light.ratio = convert_string<double>(tokens[1]);
    m_scene.ambient_light.color = parse_color(tokens[2]);
    m_scene.has_ambient_light = true;
}

void FileParser::parse_camera(const std::vector<std::string>& tokens)
{
    check_arg_count(tokens, { 4 }, "Camera (C)");
    if (m_scene.has_camera) {
        throw std::runtime_error("Camera can only be declared once.");
    }
    m_scene.camera.origin = parse_vec3(tokens[1]);
    m_scene.camera.orientation = parse_vec3(tokens[2]).normalized();
    m_scene.camera.fov = convert_string<double>(tokens[3]);
    if (m_scene.camera.fov <= 0 || m_scene.camera.fov >= 180) {
        throw std::runtime_error("Camera FOV must be between 0 and 180 degrees.");
    }
    m_scene.has_camera = true;
}

void FileParser::parse_light(const std::vector<std::string>& tokens)
{
    check_arg_count(tokens, { 4 }, "Light (L)");
    Light light;
    light.position = parse_vec3(tokens[1]);
    light.brightness = convert_string<double>(tokens[2]);
    light.color = parse_color(tokens[3]);
    m_scene.lights.push_back(light);
}

// Parse material properties robustly. diffuse_color_index is the token index of the diffuse color.
std::shared_ptr<Material> FileParser::parse_material_properties(const std::vector<std::string>& tokens, size_t diffuse_color_index)
{
    auto mat = std::make_shared<Material>();
    mat->diffuse_color = parse_color(tokens[diffuse_color_index]);

    // Defaults for optional properties
    mat->specular_color = Vec3(0.5, 0.5, 0.5);
    mat->shininess = 32.0;
    mat->reflectivity = 0.0;

    size_t n = tokens.size();

    // If there is no token after diffuse -> done.
    if (n <= diffuse_color_index + 1)
        return mat;

    // Look at the next token to decide format.
    std::string_view next_tok = tokens[diffuse_color_index + 1];
    auto comps = split(next_tok, ',');

    if (comps.size() == 2 || comps.size() == 4) {
        // Combined token forms:
        //  - "ratio,shininess" (2 comps)
        //  - "r,g,b,shininess" (4 comps)
        auto [spec_col, shiny] = parse_specular_params(next_tok);
        mat->specular_color = spec_col;
        mat->shininess = shiny;

        // Reflectivity might be the following token (if present)
        if (n > diffuse_color_index + 2) {
            mat->reflectivity = convert_string<double>(tokens[diffuse_color_index + 2]);
        }
    } else if (comps.size() == 3) {
        // Separate specular color token:
        // tokens[diffuse_color_index + 1] == "r,g,b" (specular color)
        // tokens[diffuse_color_index + 2] == shininess (optional)
        // tokens[diffuse_color_index + 3] == reflectivity (optional)
        mat->specular_color = parse_color(next_tok);
        if (n > diffuse_color_index + 2) {
            mat->shininess = convert_string<double>(tokens[diffuse_color_index + 2]);
        }
        if (n > diffuse_color_index + 3) {
            mat->reflectivity = convert_string<double>(tokens[diffuse_color_index + 3]);
        }
    } else {
        throw std::runtime_error("Invalid material specification after diffuse color.");
    }

    return mat;
}

void FileParser::parse_sphere(const std::vector<std::string>& tokens)
{
    // tokens: sp center diameter diffuse [specular...] [reflectivity]
    check_arg_count(tokens, { 4, 5, 6, 7 }, "Sphere (sp)");
    Vec3 center = parse_vec3(tokens[1]);
    auto diameter = convert_string<double>(tokens[2]);
    auto mat = parse_material_properties(tokens, 3);
    m_objects.push_back(std::make_shared<Sphere>(center, diameter / 2.0, mat));
}

void FileParser::parse_plane(const std::vector<std::string>& tokens)
{
    // tokens: pl point normal diffuse [specular...] [reflectivity]
    check_arg_count(tokens, { 4, 5, 6, 7 }, "Plane (pl)");
    Vec3 point = parse_vec3(tokens[1]);
    Vec3 normal = parse_vec3(tokens[2]).normalized();
    auto mat = parse_material_properties(tokens, 3);
    m_objects.push_back(std::make_shared<Plane>(point, normal, mat));
}

void FileParser::parse_cylinder(const std::vector<std::string>& tokens)
{
    // tokens: cy center axis diameter height diffuse [specular...] [reflectivity]
    check_arg_count(tokens, { 6, 7, 8, 9 }, "Cylinder (cy)");
    Vec3 center = parse_vec3(tokens[1]);
    Vec3 axis = parse_vec3(tokens[2]).normalized();
    double diameter = convert_string<double>(tokens[3]);
    double height = convert_string<double>(tokens[4]);
    auto mat = parse_material_properties(tokens, 5);
    m_objects.push_back(std::make_shared<Cylinder>(center, axis, diameter / 2.0, height, mat));
}

// --- Main Parser Function ---

Scene parse_file(const std::string& filename)
{
    FileParser parser(filename);
    return parser.parse();
}

} // namespace Parser