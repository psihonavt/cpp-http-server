#pragma once
#include "req_parser.h"
#include "request.h"
#include "utils/net.h"
#include <vector>

namespace Http {

using read_requests_result = std::tuple<bool, bool>;

class RequestReader {
    RequestParser m_parser;
    std::vector<Request> m_requests;

public:
    RequestReader()
    {
        m_parser = RequestParser {};
        m_parser.init();
        m_requests = {};
    }

    read_requests_result read_requests(Socket const& client);

    std::vector<Request>& requests()
    {
        return m_requests;
    }

    void erase_requests()
    {
        m_requests.erase(m_requests.begin(), m_requests.end());
    }
};

};
