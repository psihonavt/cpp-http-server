#include "catch2/catch_test_macros.hpp"
#include "http/req_reader.h"
#include <sys/fcntl.h>
#include <sys/socket.h>

void send_to_socket(Socket const& socket, std::string const& data)
{
    ssize_t n = send(socket.fd(), data.data(), data.size(), 0);
    REQUIRE(static_cast<size_t>(n) == data.size());
}

TEST_CASE("Request reader test cases", "[http_request_reader]")
{
    int fds[2];
    REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
    Socket sender { fds[0] };
    Socket receiver { fds[1] };
    REQUIRE(fcntl(receiver.fd(), F_SETFL, O_NONBLOCK) != -1);
    REQUIRE(fcntl(sender.fd(), F_SETFL, O_NONBLOCK) != -1);

    Http::RequestReader rreader {};

    SECTION("An entire request in a buffer")
    {
        send_to_socket(sender, "GET / HTTP/1.1\r\n\r\n");
        auto result { rreader.read_requests(receiver) };
        REQUIRE(result == Http::ReqReaderResult::DONE);
        REQUIRE(!rreader.requests().empty());
        REQUIRE(rreader.requests().size() == 1);
    }

    SECTION("An incomplete request in a buffer")
    {
        send_to_socket(sender, "GET / HTTP/1.1\r\n");
        auto result { rreader.read_requests(receiver) };
        REQUIRE(result == Http::ReqReaderResult::MAYBE_CAN_READ_MORE);
        REQUIRE(rreader.requests().empty());
    }

    SECTION("An invalid request in a buffer")
    {
        send_to_socket(sender, "GET / HTTPx/1.1\r\n");
        auto result { rreader.read_requests(receiver) };
        REQUIRE(result == Http::ReqReaderResult::PARSING_ERROR);
        REQUIRE(rreader.requests().empty());
    }

    SECTION("Two complete requests in a buffer")
    {
        send_to_socket(sender, "GET / HTTP/1.1\r\n\r\nPOST /take HTTP/1.1\r\n\r\n");
        auto result { rreader.read_requests(receiver) };
        REQUIRE(result == Http::ReqReaderResult::DONE);
        REQUIRE(rreader.requests().size() == 2);
    }

    SECTION("Two complete requests and one incomplete in a buffer")
    {
        send_to_socket(sender, "GET / HTTP/1.1\r\n\r\nPOST /take HTTP/1.1\r\n\r\nGET");
        auto result { rreader.read_requests(receiver) };
        REQUIRE(result == Http::ReqReaderResult::MAYBE_CAN_READ_MORE);
        REQUIRE(rreader.requests().size() == 2);

        send_to_socket(sender, " /api/1 HTTP/1.1\r\n\r\n");
        result = rreader.read_requests(receiver);
        REQUIRE(result == Http::ReqReaderResult::DONE);
        REQUIRE(rreader.requests().size() == 3);
    }

    SECTION("Client closes the write side ")
    {
        send_to_socket(sender, "GET / HTTP/1.1\r\n\r\n");

        auto result { rreader.read_requests(receiver) };
        REQUIRE(result == Http::ReqReaderResult::DONE);
        REQUIRE(rreader.requests().size() == 1);

        shutdown(sender.fd(), SHUT_WR);
        result = rreader.read_requests(receiver);
        REQUIRE(result == Http::ReqReaderResult::CLIENT_CLOSED_CONNECTION);
        REQUIRE(rreader.requests().size() == 1);
    }

    SECTION("Error reading the data")
    {
        shutdown(receiver.fd(), SHUT_RDWR);
        rreader.read_requests(receiver);
        REQUIRE(rreader.requests().size() == 0);
    }
}
