#include "droxy.h"
#include "nlohmann/json.hpp"
#include "server/context.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <cpptrace/from_current_macros.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace Server {
using json = nlohmann::json;

std::filesystem::path json_dump_localtion(std::filesystem::path const& metadata_folder, std::string const& video_id)
{
    return metadata_folder / std::format("{}.json", video_id);
}

bool has_metadata_available(std::filesystem::path const& metadata_folder, std::string const& video_id)
{
    auto dump_location = json_dump_localtion(metadata_folder, video_id);
    if (!std::filesystem::exists(dump_location)) {
        return false;
    }
    std::ifstream file { dump_location };
    if (file.is_open()) {
        json data;
        CPPTRACE_TRY
        {
            data = json::parse(file);
        }
        CPPTRACE_CATCH(std::exception const& e)
        {
            LOG_EXCEPTION("error parsing JSON from {}: {}", dump_location.string(), e.what());
            return false;
        }

        for (auto& format : data[0]) {
            if (format["video_ext"] == "mp4") {
                auto url = format["url"];
                auto parts = str_split(url, "expire=");
                if (parts.size() != 2) {
                    LOG_WARN("No expire= in the stream url {}", str_vector_join(parts, "expire="));
                    return false;
                } else {
                    auto parts2 = str_split(parts[1], "&");
                    if (parts2.size() < 2) {
                        LOG_WARN("No & in the stream url {}", str_vector_join(parts2, "&"));
                        return false;
                    }
                    auto tstamp = str_toint(parts2[0]);
                    if (!tstamp) {
                        LOG_WARN("Couldn't extract timestamp from {}", parts2[0]);
                        return false;
                    }
                    auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                                                   .count();
                    if (seconds_since_epoch > *tstamp) {
                        LOG_DEBUG("the url has expired");
                        return false;
                    }
                    return true;
                }
            }
        }
        return false;
    } else {
        LOG_ERROR("error opening a file: {}", dump_location.string());
        return false;
    }
    return false;
}

std::string get_video_id(Http::Request const& request)
{
    auto parts = str_split(request.uri.path, "/");
    if (parts.size() < 2) {
        LOG_DEBUG("failed to extract a video id from {}", request.uri.path);
        return "";
    } else {
        LOG_DEBUG("extracted a video id {}", parts[parts.size() - 1]);
    }
    return parts[parts.size() - 1];
}

std::vector<std::string> DroxyHandler::get_ytdlp_cmd(std::string const& video_id) const
{
    auto yt_video_url = std::format("https://youtube.com/watch?v={}", video_id);
    std::vector<std::string> args = { "sh", "-c" };
    auto ytdlp_an_jq = std::format("out=$(yt-dlp -J --no-warnings --format "
                                   "\"bestvideo[height<=480][ext=mp4][vcodec^=avc1]+bestaudio[ext=m4a]\" {} 2>&1); status=$?; "
                                   "[ $status -eq 0 ] && printf \"%s\" $out| jq [.requested_formats]"
                                   " || {{ printf \"%s\" $out; exit $status; }}",
        yt_video_url);
    args.push_back(ytdlp_an_jq);
    return args;
}

bool DroxyHandler::dump_to_metadata_file(std::string const& video_id, std::string& content) const
{
    auto dump_location = json_dump_localtion(m_metadata_folder, video_id);
    std::ofstream file(dump_location, std::ios_base::trunc | std::ios_base::out);
    if (file.is_open()) {
        file << std::move(content) << std::endl;
    } else {
        LOG_ERROR("failed to write to a file {}", dump_location.string());
    }
    return file.good();
}

void DroxyHandler::handle_request(RequestContext const& ctx, Http::Request const& request) const
{
    auto video_id = get_video_id(request);
    if (video_id.empty()) {
        ctx.response_is_ready(Http::Response(Http::StatusCode::BAD_REQUEST));
        return;
    }

    if (!has_metadata_available(m_metadata_folder, video_id)) {
        auto cmd = get_ytdlp_cmd(video_id);
        auto cb = [&ctx, video_id, this](Tasks::TaskResult result) -> void {
            if (result.is_successful) {
                if (dump_to_metadata_file(video_id, result.stdout_content)) {
                    auto player = get_player_html(video_id);
                    if (player) {
                        ctx.response_is_ready(Http::Response(Http::StatusCode::OK, *player, "text/html"));
                    } else {
                        ctx.response_is_ready(Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "failed to render player"));
                    }
                } else {
                    ctx.response_is_ready(Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "error dumping to a file"));
                }
            } else {
                ctx.response_is_ready(Http::Response(
                    Http::StatusCode::INTERNAL_SERVER_ERROR,
                    std::format("ytdlp + jq\nSTDOUT={}\nSTDERR={}", result.stdout_content, result.stderr_content)));
            }
        };
        m_tasks_queue.schedule_task(cmd, cb, Tasks::task_ttl_t(10 * 1000));
    } else {
        auto player = get_player_html(video_id);
        if (player) {
            ctx.response_is_ready(Http::Response(Http::StatusCode::OK, *player, "text/html"));
        } else {
            ctx.response_is_ready(Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "failed to render player"));
        }
    }
}

std::optional<std::string> DroxyHandler::get_player_html(std::string const& video_id) const
{
    std::filesystem::path player = m_metadata_folder / m_player_file_name;
    std::ifstream file(player);
    if (!file.is_open()) {
        LOG_ERROR("Player html couldn't be read!");
        return std::nullopt;
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    file.close();
    auto content = oss.str();
    str_replace_all(content, "x-video-id", video_id);
    return content;
}

std::optional<std::string> DroxyStreamHandler::get_stream_url(std::string const& video_id) const
{
    if (!has_metadata_available(m_metadata_folder, video_id)) {
        return std::nullopt;
    }
    auto dump_location = json_dump_localtion(m_metadata_folder, video_id);
    std::ifstream file { dump_location };
    json data = json::parse(file);
    std::string ext = (m_stream_type == AUDIO) ? "m4a" : "mp4";
    std::string key_ext = (m_stream_type == AUDIO) ? "audio_ext" : "video_ext";
    for (auto& format : data[0]) {
        if (format[key_ext] == ext) {
            return format["url"];
        }
    }
    return std::nullopt;
}

void DroxyStreamHandler::handle_request(RequestContext const& ctx, Http::Request const& request) const
{
    auto video_id = get_video_id(request);
    if (video_id.empty()) {
        ctx.response_is_ready(Http::Response(Http::StatusCode::BAD_REQUEST));
    }
    auto stream_url = get_stream_url(video_id);
    if (stream_url) {

        Http::Headers h { request.headers };
        h.unset("host");

        auto cb = [&ctx](response_or_chunk_t&& response) -> void {
            ctx.response_is_ready(std::move(response));
        };
        bool success = m_requester.make_request(ctx, HttpClient::RequestMethod::GET, *stream_url, h, cb);
        if (!success) {
            ctx.response_is_ready(Http::Response(Http::StatusCode::INTERNAL_SERVER_ERROR, "Problem with a HTTP requester"));
        }
    } else {
        ctx.response_is_ready(Http::Response(Http::StatusCode::BAD_REQUEST, "coudn't get the stream URL"));
    }
}

}
