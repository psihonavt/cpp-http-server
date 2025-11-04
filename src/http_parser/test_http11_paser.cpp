#include "http11_parser.h"
#include <iostream>

int main()
{
    std::string test_cases[] = {
        "OPTIONS HTTP/1.0\r\nBODY",
        "OPTIONS   HTTP/1.0\r\nBODY",
        "HTTP/1.1",
        "HTTP/2.0",
        "HTTP/3.0",
        "INVALID",
        "HTTP/1",    // missing minor
        "HTTP/1.1.1" // extra digit
    };

    for (auto const& test : test_cases) {
        HttpRequest request;
        bool success = parse_http_request(test.c_str(), test.size(), &request);

        if (success) {
            std::cout << "✓ \"" << test << "\" -> " << request.version << "\n";
        } else {
            std::cout << "✗ \"" << test << "\" -> PARSE FAILED\n";
        }
    }

    return 0;
}
