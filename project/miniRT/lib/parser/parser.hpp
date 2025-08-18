#ifndef PARSER_HPP
#define PARSER_HPP

#include "scene.hpp"
#include <charconv>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Parser {

// Splits a string_view by a delimiter
std::vector<std::string_view> split(std::string_view s, char delimiter);
void check_arg_count(const std::vector<std::string>& tokens, const std::vector<size_t>& expected_counts, std::string_view type);
Vec3 parse_vec3(std::string_view s);
Vec3 parse_color(std::string_view s);
std::pair<Vec3, double> parse_specular_params(std::string_view s);

// Helper to convert string_view to a numeric type with error checking.
template <typename T>
T convert_string(std::string_view s)
{
    T value;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc() || ptr != s.data() + s.size()) {
        throw std::invalid_argument("Invalid numeric value");
    }
    return value;
}

/**
 * @class FileParser
 * @brief A robust parser for scene description files.
 *
 * This class encapsulates the entire logic for parsing a scene file,
 * making the process re-entrant and thread-safe by managing state internally.
 * It provides detailed error reporting through the ParsingError exception.
 */
class FileParser {
public:
    /**
     * @brief Constructs a parser for a given file.
     * @param filename The path to the scene file.
     */
    explicit FileParser(const std::string& filename);

    /**
     * @brief Parses the file and constructs the Scene object.
     * @return A complete Scene object.
     * @throws ParsingError if any syntax or semantic error is found.
     */
    Scene parse();

    // Forward declaration for the exception class
    class ParsingError;

private:
    // --- State Members ---
    std::ifstream m_file;
    std::string m_filename;
    int m_line_num = 0;
    Scene m_scene;
    std::vector<std::shared_ptr<Hittable>> m_objects;

    // --- Helper Methods ---
    void process_line(const std::string& line);
    void parse_line_tokens(const std::vector<std::string>& tokens);

    // --- Element Parsing Methods ---
    void parse_ambient(const std::vector<std::string>& tokens);
    void parse_camera(const std::vector<std::string>& tokens);
    void parse_light(const std::vector<std::string>& tokens);
    void parse_sphere(const std::vector<std::string>& tokens);
    void parse_plane(const std::vector<std::string>& tokens);
    void parse_cylinder(const std::vector<std::string>& tokens);

    // --- Generic Parsing Utilities ---
    std::shared_ptr<Material> parse_material_properties(const std::vector<std::string>& tokens, size_t diffuse_color_index);
};

/**
 * @brief A convenience function to parse a scene file.
 * @param filename The path to the scene file.
 * @return A complete Scene object.
 */
Scene parse_file(const std::string& filename);

/**
 * @class ParsingError
 * @brief Custom exception for detailed parsing errors.
 *
 * Includes the file name, line number, and a descriptive message to
 * facilitate debugging.
 */
class FileParser::ParsingError : public std::runtime_error {
public:
    ParsingError(const std::string& filename, int line, const std::string& message)
        : std::runtime_error(
              "Error in '" + filename + "' on line " + std::to_string(line) + ": " + message)
    {
    }
};
} // namespace Parser

#endif // PARSER_HPP
