#include <map>
#include <optional>
#include <vector>

struct Headers {
    std::map<std::string, std::string> headers;

    std::vector<std::string> get(std::string const& field);
    void set(std::string const& field, std::string const& value);
    void override(std::string const& field, std::string const& value);
};
