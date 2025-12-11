#include "req_parser.h"
#include "config.h"
#include "utils/logging.h"
#include "utils/helpers.h"
#include <cstring>
#include <cassert>

%%{

machine http_parser;
access this->;

action mark {
  mark = fpc;
}

action get_http_version {
  store_parsed(mark, fpc, result.proto);
  mark = nullptr;
}

action get_http_method {
  store_parsed(mark, fpc, result.method);
  mark = nullptr;
}

action get_hostname {
  store_parsed(mark, fpc, result.uri.hostname);
  mark = nullptr;
}

action get_query {
  store_parsed(mark, fpc, result.uri.query);
  mark = nullptr;
}

action get_fragment {
  store_parsed(mark, fpc, result.uri.fragment);
  mark = nullptr;
}

action get_path {
  store_parsed(mark, fpc, result.uri.path);
  mark = nullptr;
}

action get_scheme {
  store_parsed(mark, fpc, result.uri.scheme);
  mark = nullptr;
}

action get_port {
  store_parsed(mark, fpc, result.uri.port);
  mark = nullptr;
}

action get_field_value {
  store_parsed_header_value(mark, fpc);
  mark = nullptr;
}

action get_field_name {
  store_parsed_header_name(mark, fpc);
  mark = nullptr;
}

action done {
  LOG_DEBUG("Parsing is done");
  fbreak;
}

#### HTTP protocl grammar #####
CRLF = "\r\n";
CTL = (cntrl | 127);
separators = ("(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\\" | "\"" | "/" | "[" | "]" | "?" | "=" | "{" | "}" | " " | "\t");
ascii_space = " ";
ws = ascii_space | "\t";
ows = ws*;
rws = ws+;
uri_percent_encoded = ("%" xdigit xdigit);
uri_reserved = (";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ",");
mark = ("-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")");
uri_unreserved = (alpha | digit | mark);
uri_sub_delimeters = ("!" | "$" | "&" | "'" | "(" | ")" | "*" | "+" | "," | ";" | "=");
uric = (uri_reserved | uri_unreserved | uri_percent_encoded);
uric_no_slash = (uri_unreserved | uri_percent_encoded | ";" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ",");
octet = any;
text = any - CTL;
quote = "\"";
quoted_pair = "\\" ascii;
qdtext = text - quote;
quoted_string = (quote (qdtext | quoted_pair)* quote);

token = (ascii -- (CTL | separators))+;

http_method = ("GET" | "POST" | "PUT" | "DELETE" | "OPTIONS" | "HEAD") >mark %get_http_method;

# https://www.ietf.org/rfc/rfc2396.txt
scheme = (alpha (alpha | digit | "+" | "-" | ".")* ) >mark %get_scheme;
ipv4_dec_octet = ("25"[0-5] | "2"[0-4]digit | "1"digit{2} | [1-9]digit | digit );
ipv4_literal = (ipv4_dec_octet "." ipv4_dec_octet "." ipv4_dec_octet "." ipv4_dec_octet);
reg_name = (uri_unreserved | uri_percent_encoded | uri_sub_delimeters)*;
host = (ipv4_literal | reg_name) >mark %get_hostname;
userinfo = (uri_unreserved | uri_percent_encoded | uri_sub_delimeters | ":")*;
port = digit{2,5} >mark %get_port;
authority = ( (userinfo "@")? host (":" port)? );
pchar = (uri_unreserved | uri_percent_encoded | uri_sub_delimeters | ":" | "@");
param = pchar*;
segment = pchar* (";" param)*;
path_segments = segment ("/" segment)*;
opaque_part = (uric_no_slash uric*);
abs_path = "/" path_segments;
rel_segment = (uri_unreserved | uri_percent_encoded | ";" | "@" | "&" | "=" | "+" | "$" | "," )+;
rel_path = (rel_segment abs_path?) >mark %get_path;
net_path = "//" authority (abs_path >mark %get_path)?;

query = uric* >mark %get_query;
fragment = uric* >mark %get_fragment;

hier_part = (net_path | abs_path >mark %get_path) ("?"query)?("#"fragment)?;

absolute_uri = (scheme ":" (hier_part | opaque_part));
relative_uri = ( net_path | abs_path >mark %get_path | rel_path ) ("?"query)? ("#"fragment)?;

request_uri = ("*" | absolute_uri | relative_uri );

http_version_number = (digit+ "." digit+);
http_version = "HTTP/" http_version_number >mark %get_http_version; 

request_line = http_method ascii_space request_uri ascii_space http_version CRLF;

field_value = any* >mark %get_field_value;
field_name = (token -- ":")+ >mark %get_field_name;
message_header = field_name ":" field_value :> CRLF;

request = request_line message_header* (CRLF @done);

main := request;

write data;

}%%

namespace Http {

void RequestParser::store_parsed(char const* start, char const* end, std::string& target){
  if (!start || !end){
    LOG_ERROR("invalid start/end pointers");
    cs = http_parser_error;
    return;
  }
  target = partial_buf + std::string(start, static_cast<size_t>(end - start));
  mark_active = false;
  partial_buf = "";
}

void RequestParser::store_parsed_header_value(char const* start, char const* end){
  if (!start || !end){
    LOG_ERROR("invalid start/end pointers");
    cs = http_parser_error;
    return;
  }
  if (cur_header_name == ""){
    LOG_ERROR("a header name must not be empty");
    cs = http_parser_error;
    return;
  }
  auto value = partial_buf + std::string(start, static_cast<size_t>(end - start));

  result.headers.set(cur_header_name, str_trim(value));

  mark_active = false;
  partial_buf = "";
  cur_header_name = "";
}

void RequestParser::store_parsed_header_name(char const* start, char const* end){
  if (!start || !end){
    LOG_ERROR("invalid start/end pointers");
    cs = http_parser_error;
    return;
  }
  if (cur_header_name != ""){
    LOG_ERROR("a header must be empty");
    cs = http_parser_error;
    return;
  }
  cur_header_name = partial_buf + std::string(start, static_cast<size_t>(end - start));
  mark_active = false;
  partial_buf = "";
}

void RequestParser::init(){
  cur_header_name = "";
  partial_buf = "";
  mark_active = false;
  result = Request{};
  bytes_read = 0;
  %% write init;
}

RequestParsingStatus RequestParser::parse_request(char const* data, size_t len, size_t offset) {
    if (offset > len) {
      LOG_ERROR("offset must be smaller than the buffer size");
      return RequestParsingStatus::Error;
    }

    const char* p {data + offset}; 
    const char* pe {data + len}; 

    char const* mark {nullptr};
    if (mark_active) {
      mark = p;
    }

    %% write exec;

    if (cs >= http_parser_first_final) {
      bytes_read = static_cast<size_t>(p - data);
      return RequestParsingStatus::Finished;
    } else if (cs == http_parser_error) {
      return RequestParsingStatus::Error;
    } else {
      if (mark) {
        partial_buf = partial_buf + std::string(mark, static_cast<size_t>(pe - mark));
        if (partial_buf.size() > PARSER_MAX_PARTIAL_BUF_LEN) {
          LOG_WARN("partial_buf is getting too large, aborting");
          return RequestParsingStatus::Error;
        }
        mark_active = true;
      }
      return RequestParsingStatus::NeedMoreData;
    }   
}

};
