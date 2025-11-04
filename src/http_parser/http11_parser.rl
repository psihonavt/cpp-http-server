#include "http11_parser.h"
#include <cstring>

%%{

machine http_parser;

action mark {
  mark = fpc;
}

action get_http_version {
  ctx->version.assign(mark, fpc - mark);
}

action get_http_method {
  ctx->method.assign(mark, fpc - mark);
}

action get_hostname {
  ctx->uri.hostname.assign(mark, fpc - mark);
}

action get_query {
  ctx->uri.query.assign(mark, fpc - mark);
}

action get_fragment {
  ctx->uri.fragment.assign(mark, fpc - mark);
}

action get_path {
  ctx->uri.path.assign(mark, fpc - mark);
}

action get_scheme {
  ctx->uri.scheme.assign(mark, fpc - mark);
}

action get_port {
  ctx->uri.port.assign(mark, fpc - mark);
}

#### HTTP protocl grammar #####
CRLF = "\r\n";
CTL = (cntrl | 127);
separators = ("(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\\" | "\"" | "/" | "[" | "]" | "?" | "=" | "{" | "}" | " " | "\t");
ascii_space = " ";
uri_percent_encoded = ("%" xdigit xdigit);
uri_reserved = (";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ",");
mark = ("-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")");
uri_unreserved = (alpha | digit | mark);
uri_sub_delimeters = ("!" | "$" | "&" | "'" | "(" | ")" | "*" | "+" | "," | ";" | "=");
uric = (uri_reserved | uri_unreserved | uri_percent_encoded);
uric_no_slash = (uri_unreserved | uri_percent_encoded | ";" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ",");

token = (ascii -- (CTL | separators));

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

request_line = http_method ascii_space request_uri ascii_space http_version CRLF any*;

main := request_line;

write data;

}%%

bool parse_http_request(char const* data, size_t len, HttpRequest* result) {
    int cs; // current state
    const char* p {data}; // current position
    const char* pe {data + len}; // end position
    const char* eof {pe}; // EOF position

    HttpRequest ctx_storage {};
    HttpRequest* ctx {&ctx_storage};

    // a mark for substring extraction
    const char* mark = nullptr;
    
    %% write init;
    %% write exec;

    if (cs >= http_parser_first_final) {
      *result = ctx_storage;
      return true;
    } else {
      return false;
    }
  }
