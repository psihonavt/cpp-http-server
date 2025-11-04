#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "http11_parser.h"

struct TestCase {
    std::string input;
    bool should_pass;
    std::string method;
    std::string scheme;
    std::string hostname;
    std::string path;
    std::string query;
    std::string version;
};

void print_parsed_request(HttpRequest const& req)
{
    std::cout << "\n  Parsed HttpRequest:";
    std::cout << "\n    method   = \"" << req.method << "\"";
    std::cout << "\n    scheme   = \"" << req.uri.scheme << "\"";
    std::cout << "\n    hostname = \"" << req.uri.hostname << "\"";
    std::cout << "\n    path     = \"" << req.uri.path << "\"";
    std::cout << "\n    query    = \"" << req.uri.query << "\"";
    std::cout << "\n    version  = \"" << req.version << "\"";
}

bool run_test(TestCase const& test)
{
    HttpRequest req;
    bool result = parse_http_request(test.input.c_str(), test.input.size(), &req);

    bool test_passed = false;

    std::cout << (result == test.should_pass ? "✓" : "✗") << " ";
    std::cout << "\"" << test.input << "\"";

    // Case 1: Parse result doesn't match expectation
    if (result != test.should_pass) {
        std::cout << " [EXPECTED " << (test.should_pass ? "PASS" : "FAIL") << "]";
        print_parsed_request(req);
        std::cout << "\n";
        return test_passed;
    }

    // Case 2: Parsing succeeded, check fields
    if (result) {
        bool all_match = true;

        if (req.method != test.method) {
            std::cout << "\n  ✗ method: got '" << req.method << "', expected '" << test.method << "'";
            all_match = false;
        }
        if (req.uri.scheme != test.scheme) {
            std::cout << "\n  ✗ scheme: got '" << req.uri.scheme << "', expected '" << test.scheme << "'";
            all_match = false;
        }
        if (req.uri.hostname != test.hostname) {
            std::cout << "\n  ✗ hostname: got '" << req.uri.hostname << "', expected '" << test.hostname << "'";
            all_match = false;
        }
        if (req.uri.path != test.path) {
            std::cout << "\n  ✗ path: got '" << req.uri.path << "', expected '" << test.path << "'";
            all_match = false;
        }
        if (req.uri.query != test.query) {
            std::cout << "\n  ✗ query: got '" << req.uri.query << "', expected '" << test.query << "'";
            all_match = false;
        }
        if (req.version != test.version) {
            std::cout << "\n  ✗ version: got '" << req.version << "', expected '" << test.version << "'";
            all_match = false;
        }

        if (all_match) {
            std::cout << " -> " << req.method << " " << req.uri.path
                      << (req.uri.query.empty() ? "" : "?") << req.uri.query
                      << " HTTP/" << req.version;
            test_passed = true;
        } else {
            print_parsed_request(req);
        }
    } else {
        // Parse failed as expected
        std::cout << " -> PARSE FAILED (as expected)";
        test_passed = true;
    }

    std::cout << "\n";
    return test_passed;
}

int main()
{
    std::vector<TestCase> tests = {
        // Basic requests with relative paths
        {
            "GET / HTTP/1.1\r\n",
            true,
            "GET", "", "", "/", "", "1.1" },
        { "GET /index.html HTTP/1.1\r\n",
            true,
            "GET", "", "", "/index.html", "", "1.1" },
        { "POST /api/users HTTP/1.1\r\n",
            true,
            "POST", "", "", "/api/users", "", "1.1" },
        { "DELETE /resource/123 HTTP/1.0\r\n",
            true,
            "DELETE", "", "", "/resource/123", "", "1.0" },

        // Requests with query strings
        {
            "GET /search?q=test HTTP/1.1\r\n",
            true,
            "GET", "", "", "/search", "q=test", "1.1" },
        { "GET /api?key=123&value=abc HTTP/1.1\r\n",
            true,
            "GET", "", "", "/api", "key=123&value=abc", "1.1" },
        { "POST /form?debug=1 HTTP/1.1\r\n",
            true,
            "POST", "", "", "/form", "debug=1", "1.1" },

        // Requests with absolute URIs
        {
            "GET http://example.com/path HTTP/1.1\r\n",
            true,
            "GET", "http", "example.com", "/path", "", "1.1" },
        { "GET http://example.com:8080/api HTTP/1.1\r\n",
            true,
            "GET", "http", "example.com:8080", "/api", "", "1.1" },
        { "GET https://secure.example.com/login?redirect=/home HTTP/1.1\r\n",
            true,
            "GET", "https", "secure.example.com", "/login", "redirect=/home", "1.1" },

        // Complex paths
        {
            "GET /path/to/resource.json HTTP/1.1\r\n",
            true,
            "GET", "", "", "/path/to/resource.json", "", "1.1" },
        { "GET /api/v1/users/123/profile HTTP/1.1\r\n",
            true,
            "GET", "", "", "/api/v1/users/123/profile", "", "1.1" },
        { "GET /file%20name.txt HTTP/1.1\r\n", // percent-encoded space
            true,
            "GET", "", "", "/file%20name.txt", "", "1.1" },

        // Different HTTP methods
        {
            "HEAD /status HTTP/1.1\r\n",
            true,
            "HEAD", "", "", "/status", "", "1.1" },
        { "PUT /resource HTTP/1.1\r\n",
            true,
            "PUT", "", "", "/resource", "", "1.1" },
        { "PATCH /resource/123 HTTP/1.1\r\n",
            true,
            "PATCH", "", "", "/resource/123", "", "1.1" },
        { "OPTIONS * HTTP/1.1\r\n",
            true,
            "OPTIONS", "", "", "*", "", "1.1" },

        // HTTP/1.0 requests
        {
            "GET /legacy HTTP/1.0\r\n",
            true,
            "GET", "", "", "/legacy", "", "1.0" },

        // Edge cases - root path
        {
            "GET / HTTP/1.1\r\n",
            true,
            "GET", "", "", "/", "", "1.1" },
        { "GET https://127.0.0.2/abs HTTP/1.1\r\n",
            true,
            "GET", "https", "127.0.0.2", "/abs", "", "1.1" },

        // Invalid requests
        {
            "INVALID\r\n",
            false,
            "", "", "", "", "", "" },
        { "GET\r\n", // incomplete
            false,
            "", "", "", "", "", "" },
        { "GET /path\r\n", // missing HTTP version
            false,
            "", "", "", "", "", "" },
        { "GET /path HTTP/\r\n", // incomplete version
            false,
            "", "", "", "", "", "" },
        { "GET /path HTTP/99.99\r\n", // invalid version
            false,
            "", "", "", "", "", "" },
        { "/path HTTP/1.1\r\n", // missing method
            false,
            "", "", "", "", "", "" },
        { "GET  HTTP/1.1\r\n", // missing URI
            false,
            "", "", "", "", "", "" },
        { "get /path HTTP/1.1\r\n", // lowercase method (should fail if strict)
            false,
            "", "", "", "", "", "" },
        { "GET /path\nHTTP/1.1\r\n", // embedded newline
            false,
            "", "", "", "", "", "" },
    };

    std::cout << "=== HTTP Request Line Parser Tests ===\n\n";

    int passed = 0;
    int failed = 0;

    for (auto const& test : tests) {
        if (run_test(test)) {
            passed++;
        } else {
            failed++;
        }
    }

    std::cout << "\n=== Summary ===\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Total:  " << (passed + failed) << "\n";

    return failed == 0 ? 0 : 1;
}
