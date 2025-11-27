#include "errors.h"

namespace Http {

HttpErrorCategory const& http_category()
{
    static HttpErrorCategory instance;
    return instance;
}

std::error_code make_error_code(Http::Error e)
{
    return { static_cast<int>(e), Http::http_category() };
}

}
