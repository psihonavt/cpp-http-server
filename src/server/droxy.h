#include "handlers.h"
#include "server/tasks.h"
#include <filesystem>
#include <string>

namespace Server {

std::string get_video_id(Http::Request const& request);
bool has_metadata_available(std::filesystem::path const&, std::string const&);

class DroxyHandler : public IRequestHandler {
private:
    Tasks::Queue& m_tasks_queue;
    std::filesystem::path m_metadata_folder;
    std::string m_player_file_name;

    std::vector<std::string> get_ytdlp_cmd(std::string const& video_id) const;

    bool dump_to_metadata_file(std::string const& video_id, std::string& content) const;
    std::optional<std::string> get_player_html(std::string const& video_id) const;

public:
    DroxyHandler(Tasks::Queue& tasks_queue, std::filesystem::path const& metadata_folder, std::string const& player_file_name)
        : m_tasks_queue { tasks_queue }
        , m_metadata_folder { metadata_folder }
        , m_player_file_name { player_file_name }
    {
        if (!std::filesystem::is_directory(m_metadata_folder)) {
            throw std::runtime_error(std::format("can't access {}; does it exist?", m_metadata_folder.string()));
        }

        if (!std::filesystem::exists(m_metadata_folder / m_player_file_name)) {
            throw std::runtime_error(std::format("can't find player file {} in {}", m_player_file_name, m_metadata_folder.string()));
        }
    }

    void handle_request(RequestContext const& ctx, Http::Request const& request) const override;
};

class DroxyStreamHandler : public IRequestHandler {
public:
    enum StreamType {
        AUDIO,
        VIDEO,
    };

    DroxyStreamHandler(HttpClient::Requester& requester, StreamType stream_type, std::filesystem::path const& metadata_folder)
        : m_requester { requester }
        , m_stream_type { stream_type }
        , m_metadata_folder { metadata_folder }
    {
        if (!std::filesystem::is_directory(m_metadata_folder)) {
            throw std::runtime_error(std::format("can't access {}; does it exist?", m_metadata_folder.string()));
        }
    }

    void handle_request(RequestContext const& ctx, Http::Request const& request) const override;

private:
    HttpClient::Requester& m_requester;
    StreamType m_stream_type;
    std::filesystem::path m_metadata_folder;
    std::optional<std::string> get_stream_url(std::string const& video_id) const;
};

}
