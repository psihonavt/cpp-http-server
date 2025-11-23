#include "errors.h"

namespace Http {
HttpErrorCategory const& http_category()
{
    static HttpErrorCategory instance;
    return instance;
}
}

namespace std {

template<>
struct is_error_code_enum<Http::Error> : true_type { };

}

std::error_code make_error_code(Http::Error e)
{
    return { static_cast<int>(e), Http::http_category() };
}
