#include "http/request.h"
#include "http/response.h"
#include <filesystem>

namespace Server {

class IRequestHandler {
public:
    virtual Http::Response handle_request(Http::Request const& request) const;
};

class StaticRootHandler : public IRequestHandler {
private:
    std::filesystem::path m_root;

public:
    StaticRootHandler(std::string const& static_root)
        : m_root { static_root }
    {
    }

    Http::Response handle_request(Http::Request const& request) const override;
};

}
