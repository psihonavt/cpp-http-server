#pragma once

#include <cstring>
#include <format>
#include <stdexcept>
#include <sys/fcntl.h>
#include <unistd.h>

namespace Server {

class SignalPipe {
private:
    std::string m_name;
    int m_read_end;
    int m_write_end;

public:
    SignalPipe(std::string const& name, bool non_blocking = true)
        : m_name { name }
    {
        int pends[2];
        if (pipe(pends) != 0) {
            throw std::runtime_error(std::format("pipe: ", strerror(errno)));
        }
        m_read_end = pends[0];
        m_write_end = pends[1];

        if (non_blocking) {
            for (int fd : std::vector { m_read_end, m_write_end }) {
                auto flags = fcntl(fd, F_GETFL);
                if ((fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1) {
                    throw std::runtime_error(std::format("fcntl F_SETFL O_NONBLOCK: {}", strerror(errno)));
                }
            }
        }
    }

    ~SignalPipe()
    {
        if (m_read_end != -1) {
            close(m_read_end);
        }
        if (m_write_end != -1) {
            close(m_write_end);
        }
    }

    int write_end() { return m_write_end; }

    int read_end() { return m_read_end; }
};

void setup_signal_handling();
void reset_all_signals_to_default();
}
