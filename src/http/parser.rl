#include "parser.h"
#include "utils/logging.h"
#include <cstring>
#include <cstring>

%%{

machine http_parser;
access this->;

action mark {
  mark = fpc;
}

action get_http_version {
  store_parsed(mark, fpc, result.proto);
}

action get_http_method {
  store_parsed(mark, fpc, result.method);
}

action get_hostname {
  store_parsed(mark, fpc, result.uri.hostname);
}

action get_query {
  store_parsed(mark, fpc, result.uri.query);
}

action get_fragment {
  store_parsed(mark, fpc, result.uri.fragment);
}

action get_path {
  store_parsed(mark, fpc, result.uri.path);
}

action get_scheme {
  store_parsed(mark, fpc, result.uri.scheme);
}

action get_port {
  store_parsed(mark, fpc, result.uri.port);
}

action get_field_value {
  store_parsed_header_value(mark, fpc);
}

action get_field_name {
  store_parsed_header_name(mark, fpc);
}

action done {
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
message_header = field_name ":" ows field_value:> CRLF;

request = request_line message_header* (CRLF @done);

main := request;

write data;

}%%

namespace Http {

void RequestParser::store_parsed(char const* start, char const* end, std::string& target){
  if (!start || !end){
    throw std::runtime_error("invalid start/end pointers");
  }
  target = partial_buf + std::string(start, static_cast<size_t>(end - start));
  mark_active = false;
  partial_buf = "";
}

void RequestParser::store_parsed_header_value(char const* start, char const* end){
  if (!start || !end){
    throw std::runtime_error("invalid start/end pointers");
  }
  if (cur_header_name == ""){
    throw std::runtime_error("a header name must not be empty");
  }
  auto value = partial_buf + std::string(start, static_cast<size_t>(end - start));

  size_t ws_pos {value.find_last_not_of(" \t")};
  if (ws_pos != std::string::npos){
    value.erase(ws_pos + 1);
  }

  result.headers.set(cur_header_name, value);

  mark_active = false;
  partial_buf = "";
  cur_header_name = "";
}

void RequestParser::store_parsed_header_name(char const* start, char const* end){
  if (!start || !end){
    throw std::runtime_error("invalid start/end pointers");
  }
  if (cur_header_name != ""){
    throw std::runtime_error("a header must be empty");
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
  %% write init;
}

RequestParsingStatus RequestParser::parse_request(char const* data, size_t len, char const* eof) {
    const char* p {data}; 
    const char* pe {data + len}; 

    char const* mark {nullptr};
    if (mark_active){
      mark = p;
    }

    %% write exec;

    if (cs >= http_parser_first_final) {
      return RequestParsingStatus::Finished;
    } else if (cs == http_parser_error) {
      return RequestParsingStatus::Error;
    } else if (!eof) {
      if (!mark){
        throw std::runtime_error("mark can not be null");
      }
      partial_buf = std::string(mark, static_cast<size_t>(pe - mark));
      mark_active = true;
      return RequestParsingStatus::NeedContinue;
    } else {
      return RequestParsingStatus::Error;
    }
  }
};
