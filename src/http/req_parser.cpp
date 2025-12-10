
#line 1 "src/http/req_parser.rl"
#include "req_parser.h"
#include "utils/logging.h"
#include <cstring>
#include <cassert>


#line 10 "src/http/req_parser.cpp"
static const int http_parser_start = 1;
static const int http_parser_first_final = 80;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 130 "src/http/req_parser.rl"


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
  
#line 82 "src/http/req_parser.cpp"
	{
	 this->cs = http_parser_start;
	}

#line 193 "src/http/req_parser.rl"
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

    
#line 105 "src/http/req_parser.cpp"
	{
	if ( p == pe )
		goto _test_eof;
	switch (  this->cs )
	{
case 1:
	switch( (*p) ) {
		case 68: goto tr0;
		case 71: goto tr2;
		case 72: goto tr3;
		case 79: goto tr4;
		case 80: goto tr5;
	}
	goto st0;
st0:
 this->cs = 0;
	goto _out;
tr0:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 133 "src/http/req_parser.cpp"
	if ( (*p) == 69 )
		goto st3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 76 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 69 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 84 )
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 69 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 32 )
		goto tr11;
	goto st0;
tr11:
#line 19 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.method);
}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 182 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 33: goto tr12;
		case 37: goto tr13;
		case 47: goto tr14;
		case 59: goto tr12;
		case 61: goto tr12;
		case 64: goto tr12;
		case 95: goto tr12;
		case 126: goto tr12;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 57 )
			goto tr12;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr15;
	} else
		goto tr15;
	goto st0;
tr12:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 212 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st9;
		case 35: goto tr18;
		case 37: goto st29;
		case 47: goto st31;
		case 59: goto st9;
		case 61: goto st9;
		case 63: goto tr21;
		case 95: goto st9;
		case 126: goto st9;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 57 )
			goto st9;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st9;
	} else
		goto st9;
	goto st0;
tr16:
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st10;
tr43:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 31 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.fragment);
}
	goto st10;
tr46:
#line 31 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.fragment);
}
	goto st10;
tr53:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 27 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.query);
}
	goto st10;
tr57:
#line 27 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.query);
}
	goto st10;
tr63:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st10;
tr71:
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st10;
tr92:
#line 43 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.port);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 310 "src/http/req_parser.cpp"
	if ( (*p) == 72 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 84 )
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 84 )
		goto st13;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 80 )
		goto st14;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 47 )
		goto st15;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr27;
	goto st0;
tr27:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 359 "src/http/req_parser.cpp"
	if ( (*p) == 46 )
		goto st17;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st16;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st18;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 13 )
		goto tr31;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st18;
	goto st0;
tr31:
#line 15 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.proto);
}
	goto st19;
tr40:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 47 "src/http/req_parser.rl"
	{
  store_parsed_header_value(mark, p);
}
	goto st19;
tr42:
#line 47 "src/http/req_parser.rl"
	{
  store_parsed_header_value(mark, p);
}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 407 "src/http/req_parser.cpp"
	if ( (*p) == 10 )
		goto st20;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 13: goto st21;
		case 33: goto tr34;
		case 124: goto tr34;
		case 126: goto tr34;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr34;
		} else if ( (*p) >= 35 )
			goto tr34;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr34;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto tr34;
		} else
			goto tr34;
	} else
		goto tr34;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) == 10 )
		goto tr35;
	goto st0;
tr35:
#line 55 "src/http/req_parser.rl"
	{
  LOG_DEBUG("Parsing is done");
  {p++;  this->cs = 80; goto _out;}
}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 457 "src/http/req_parser.cpp"
	goto st0;
tr34:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 469 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 33: goto st22;
		case 58: goto tr37;
		case 124: goto st22;
		case 126: goto st22;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 39 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto st22;
		} else if ( (*p) >= 35 )
			goto st22;
	} else if ( (*p) > 46 ) {
		if ( (*p) < 65 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st22;
		} else if ( (*p) > 90 ) {
			if ( 94 <= (*p) && (*p) <= 122 )
				goto st22;
		} else
			goto st22;
	} else
		goto st22;
	goto st0;
tr39:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st23;
tr37:
#line 51 "src/http/req_parser.rl"
	{
  store_parsed_header_name(mark, p);
}
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 510 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 9: goto tr39;
		case 13: goto tr40;
		case 32: goto tr39;
	}
	goto tr38;
tr38:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 527 "src/http/req_parser.cpp"
	if ( (*p) == 13 )
		goto tr42;
	goto st24;
tr18:
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st25;
tr55:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 27 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.query);
}
	goto st25;
tr59:
#line 27 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.query);
}
	goto st25;
tr65:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st25;
tr73:
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st25;
tr93:
#line 43 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.port);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 591 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr43;
		case 33: goto tr44;
		case 37: goto tr45;
		case 61: goto tr44;
		case 95: goto tr44;
		case 126: goto tr44;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr44;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr44;
	} else
		goto tr44;
	goto st0;
tr44:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 619 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr46;
		case 33: goto st26;
		case 37: goto st27;
		case 61: goto st26;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st26;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st26;
	} else
		goto st26;
	goto st0;
tr45:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 647 "src/http/req_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st28;
	} else
		goto st28;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st26;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st26;
	} else
		goto st26;
	goto st0;
tr13:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 680 "src/http/req_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st30;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st30;
	} else
		goto st30;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st9;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st9;
	} else
		goto st9;
	goto st0;
tr67:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
	goto st31;
tr75:
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st31;
tr94:
#line 43 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.port);
}
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 737 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st31;
		case 35: goto tr18;
		case 37: goto st32;
		case 61: goto st31;
		case 63: goto tr21;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st31;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st31;
	} else
		goto st31;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st33;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st33;
	} else
		goto st33;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st31;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st31;
	} else
		goto st31;
	goto st0;
tr21:
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st34;
tr69:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st34;
tr77:
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st34;
tr96:
#line 43 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.port);
}
#line 35 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.path);
}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 827 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr53;
		case 33: goto tr54;
		case 35: goto tr55;
		case 37: goto tr56;
		case 61: goto tr54;
		case 95: goto tr54;
		case 126: goto tr54;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr54;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr54;
	} else
		goto tr54;
	goto st0;
tr54:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 856 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr57;
		case 33: goto st35;
		case 35: goto tr59;
		case 37: goto st36;
		case 61: goto st35;
		case 95: goto st35;
		case 126: goto st35;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st0;
tr56:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 885 "src/http/req_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st37;
	} else
		goto st37;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st35;
	} else
		goto st35;
	goto st0;
tr14:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 918 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st31;
		case 35: goto tr18;
		case 37: goto st32;
		case 47: goto st39;
		case 61: goto st31;
		case 63: goto tr21;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st31;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st31;
	} else
		goto st31;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 32: goto tr63;
		case 33: goto tr64;
		case 35: goto tr65;
		case 37: goto tr66;
		case 47: goto tr67;
		case 58: goto tr68;
		case 61: goto tr64;
		case 63: goto tr69;
		case 64: goto st47;
		case 95: goto tr64;
		case 126: goto tr64;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr64;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr64;
	} else
		goto tr64;
	goto st0;
tr64:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 975 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr71;
		case 33: goto st40;
		case 35: goto tr73;
		case 37: goto st41;
		case 47: goto tr75;
		case 58: goto tr76;
		case 61: goto st40;
		case 63: goto tr77;
		case 64: goto st47;
		case 95: goto st40;
		case 126: goto st40;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st40;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st40;
	} else
		goto st40;
	goto st0;
tr66:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 1008 "src/http/req_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st42;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st42;
	} else
		goto st42;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st40;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st40;
	} else
		goto st40;
	goto st0;
tr68:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
	goto st43;
tr76:
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 1051 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st44;
		case 35: goto tr18;
		case 37: goto st45;
		case 47: goto st31;
		case 61: goto st44;
		case 63: goto tr21;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr81;
		} else if ( (*p) >= 36 )
			goto st44;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st44;
		} else if ( (*p) >= 65 )
			goto st44;
	} else
		goto st44;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st44;
		case 35: goto tr18;
		case 37: goto st45;
		case 47: goto st31;
		case 61: goto st44;
		case 63: goto tr21;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st44;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st44;
	} else
		goto st44;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st46;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st46;
	} else
		goto st46;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st44;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st44;
	} else
		goto st44;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	switch( (*p) ) {
		case 32: goto tr63;
		case 33: goto tr83;
		case 35: goto tr65;
		case 37: goto tr84;
		case 47: goto tr67;
		case 58: goto tr85;
		case 61: goto tr83;
		case 63: goto tr69;
		case 64: goto st31;
		case 95: goto tr83;
		case 126: goto tr83;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr83;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr83;
	} else
		goto tr83;
	goto st0;
tr83:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st48;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
#line 1166 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr71;
		case 33: goto st48;
		case 35: goto tr73;
		case 37: goto st49;
		case 47: goto tr75;
		case 58: goto tr88;
		case 61: goto st48;
		case 63: goto tr77;
		case 64: goto st31;
		case 95: goto st48;
		case 126: goto st48;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st48;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st48;
	} else
		goto st48;
	goto st0;
tr84:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 1199 "src/http/req_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st50;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st50;
	} else
		goto st50;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st48;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st48;
	} else
		goto st48;
	goto st0;
tr85:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
	goto st51;
tr88:
#line 23 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.hostname);
}
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 1242 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st31;
		case 35: goto tr18;
		case 37: goto st32;
		case 61: goto st31;
		case 63: goto tr21;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr90;
		} else if ( (*p) >= 36 )
			goto st31;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 64 )
			goto st31;
	} else
		goto st31;
	goto st0;
tr90:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1278 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st31;
		case 35: goto tr18;
		case 37: goto st32;
		case 61: goto st31;
		case 63: goto tr21;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st53;
		} else if ( (*p) >= 36 )
			goto st31;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 64 )
			goto st31;
	} else
		goto st31;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st31;
		case 35: goto tr93;
		case 37: goto st32;
		case 47: goto tr94;
		case 61: goto st31;
		case 63: goto tr96;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st54;
		} else if ( (*p) >= 36 )
			goto st31;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 64 )
			goto st31;
	} else
		goto st31;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st31;
		case 35: goto tr93;
		case 37: goto st32;
		case 47: goto tr94;
		case 61: goto st31;
		case 63: goto tr96;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st55;
		} else if ( (*p) >= 36 )
			goto st31;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 64 )
			goto st31;
	} else
		goto st31;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st31;
		case 35: goto tr93;
		case 37: goto st32;
		case 47: goto tr94;
		case 61: goto st31;
		case 63: goto tr96;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st56;
		} else if ( (*p) >= 36 )
			goto st31;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 64 )
			goto st31;
	} else
		goto st31;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st31;
		case 35: goto tr93;
		case 37: goto st32;
		case 47: goto tr94;
		case 61: goto st31;
		case 63: goto tr96;
		case 95: goto st31;
		case 126: goto st31;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st31;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st31;
	} else
		goto st31;
	goto st0;
tr81:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
#line 1428 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st44;
		case 35: goto tr18;
		case 37: goto st45;
		case 47: goto st31;
		case 61: goto st44;
		case 63: goto tr21;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st58;
		} else if ( (*p) >= 36 )
			goto st44;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st44;
		} else if ( (*p) >= 65 )
			goto st44;
	} else
		goto st44;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st44;
		case 35: goto tr93;
		case 37: goto st45;
		case 47: goto tr94;
		case 61: goto st44;
		case 63: goto tr96;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st59;
		} else if ( (*p) >= 36 )
			goto st44;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st44;
		} else if ( (*p) >= 65 )
			goto st44;
	} else
		goto st44;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st44;
		case 35: goto tr93;
		case 37: goto st45;
		case 47: goto tr94;
		case 61: goto st44;
		case 63: goto tr96;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st60;
		} else if ( (*p) >= 36 )
			goto st44;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st44;
		} else if ( (*p) >= 65 )
			goto st44;
	} else
		goto st44;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st44;
		case 35: goto tr93;
		case 37: goto st45;
		case 47: goto tr94;
		case 61: goto st44;
		case 63: goto tr96;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st61;
		} else if ( (*p) >= 36 )
			goto st44;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st44;
		} else if ( (*p) >= 65 )
			goto st44;
	} else
		goto st44;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	switch( (*p) ) {
		case 32: goto tr92;
		case 33: goto st44;
		case 35: goto tr93;
		case 37: goto st45;
		case 47: goto tr94;
		case 61: goto st44;
		case 63: goto tr96;
		case 64: goto st47;
		case 95: goto st44;
		case 126: goto st44;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st44;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st44;
	} else
		goto st44;
	goto st0;
tr15:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
#line 1584 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st9;
		case 35: goto tr18;
		case 37: goto st29;
		case 43: goto st62;
		case 47: goto st31;
		case 58: goto tr104;
		case 59: goto st9;
		case 61: goto st9;
		case 63: goto tr21;
		case 64: goto st9;
		case 95: goto st9;
		case 126: goto st9;
	}
	if ( (*p) < 45 ) {
		if ( 36 <= (*p) && (*p) <= 44 )
			goto st9;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st62;
		} else if ( (*p) >= 65 )
			goto st62;
	} else
		goto st62;
	goto st0;
tr104:
#line 39 "src/http/req_parser.rl"
	{
  store_parsed(mark, p, result.uri.scheme);
}
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
#line 1622 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 33: goto st64;
		case 37: goto st65;
		case 47: goto tr14;
		case 61: goto st64;
		case 95: goto st64;
		case 126: goto st64;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st64;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st64;
	} else
		goto st64;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	switch( (*p) ) {
		case 32: goto st10;
		case 33: goto st64;
		case 37: goto st65;
		case 61: goto st64;
		case 95: goto st64;
		case 126: goto st64;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st64;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st64;
	} else
		goto st64;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st66;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st66;
	} else
		goto st66;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st64;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st64;
	} else
		goto st64;
	goto st0;
tr2:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 1697 "src/http/req_parser.cpp"
	if ( (*p) == 69 )
		goto st68;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 84 )
		goto st7;
	goto st0;
tr3:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 1718 "src/http/req_parser.cpp"
	if ( (*p) == 69 )
		goto st70;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 65 )
		goto st71;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 68 )
		goto st7;
	goto st0;
tr4:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 1746 "src/http/req_parser.cpp"
	if ( (*p) == 80 )
		goto st73;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 84 )
		goto st74;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( (*p) == 73 )
		goto st75;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( (*p) == 79 )
		goto st76;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( (*p) == 78 )
		goto st77;
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 83 )
		goto st7;
	goto st0;
tr5:
#line 11 "src/http/req_parser.rl"
	{
  mark = p;
}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 1795 "src/http/req_parser.cpp"
	switch( (*p) ) {
		case 79: goto st79;
		case 85: goto st68;
	}
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( (*p) == 83 )
		goto st68;
	goto st0;
	}
	_test_eof2:  this->cs = 2; goto _test_eof; 
	_test_eof3:  this->cs = 3; goto _test_eof; 
	_test_eof4:  this->cs = 4; goto _test_eof; 
	_test_eof5:  this->cs = 5; goto _test_eof; 
	_test_eof6:  this->cs = 6; goto _test_eof; 
	_test_eof7:  this->cs = 7; goto _test_eof; 
	_test_eof8:  this->cs = 8; goto _test_eof; 
	_test_eof9:  this->cs = 9; goto _test_eof; 
	_test_eof10:  this->cs = 10; goto _test_eof; 
	_test_eof11:  this->cs = 11; goto _test_eof; 
	_test_eof12:  this->cs = 12; goto _test_eof; 
	_test_eof13:  this->cs = 13; goto _test_eof; 
	_test_eof14:  this->cs = 14; goto _test_eof; 
	_test_eof15:  this->cs = 15; goto _test_eof; 
	_test_eof16:  this->cs = 16; goto _test_eof; 
	_test_eof17:  this->cs = 17; goto _test_eof; 
	_test_eof18:  this->cs = 18; goto _test_eof; 
	_test_eof19:  this->cs = 19; goto _test_eof; 
	_test_eof20:  this->cs = 20; goto _test_eof; 
	_test_eof21:  this->cs = 21; goto _test_eof; 
	_test_eof80:  this->cs = 80; goto _test_eof; 
	_test_eof22:  this->cs = 22; goto _test_eof; 
	_test_eof23:  this->cs = 23; goto _test_eof; 
	_test_eof24:  this->cs = 24; goto _test_eof; 
	_test_eof25:  this->cs = 25; goto _test_eof; 
	_test_eof26:  this->cs = 26; goto _test_eof; 
	_test_eof27:  this->cs = 27; goto _test_eof; 
	_test_eof28:  this->cs = 28; goto _test_eof; 
	_test_eof29:  this->cs = 29; goto _test_eof; 
	_test_eof30:  this->cs = 30; goto _test_eof; 
	_test_eof31:  this->cs = 31; goto _test_eof; 
	_test_eof32:  this->cs = 32; goto _test_eof; 
	_test_eof33:  this->cs = 33; goto _test_eof; 
	_test_eof34:  this->cs = 34; goto _test_eof; 
	_test_eof35:  this->cs = 35; goto _test_eof; 
	_test_eof36:  this->cs = 36; goto _test_eof; 
	_test_eof37:  this->cs = 37; goto _test_eof; 
	_test_eof38:  this->cs = 38; goto _test_eof; 
	_test_eof39:  this->cs = 39; goto _test_eof; 
	_test_eof40:  this->cs = 40; goto _test_eof; 
	_test_eof41:  this->cs = 41; goto _test_eof; 
	_test_eof42:  this->cs = 42; goto _test_eof; 
	_test_eof43:  this->cs = 43; goto _test_eof; 
	_test_eof44:  this->cs = 44; goto _test_eof; 
	_test_eof45:  this->cs = 45; goto _test_eof; 
	_test_eof46:  this->cs = 46; goto _test_eof; 
	_test_eof47:  this->cs = 47; goto _test_eof; 
	_test_eof48:  this->cs = 48; goto _test_eof; 
	_test_eof49:  this->cs = 49; goto _test_eof; 
	_test_eof50:  this->cs = 50; goto _test_eof; 
	_test_eof51:  this->cs = 51; goto _test_eof; 
	_test_eof52:  this->cs = 52; goto _test_eof; 
	_test_eof53:  this->cs = 53; goto _test_eof; 
	_test_eof54:  this->cs = 54; goto _test_eof; 
	_test_eof55:  this->cs = 55; goto _test_eof; 
	_test_eof56:  this->cs = 56; goto _test_eof; 
	_test_eof57:  this->cs = 57; goto _test_eof; 
	_test_eof58:  this->cs = 58; goto _test_eof; 
	_test_eof59:  this->cs = 59; goto _test_eof; 
	_test_eof60:  this->cs = 60; goto _test_eof; 
	_test_eof61:  this->cs = 61; goto _test_eof; 
	_test_eof62:  this->cs = 62; goto _test_eof; 
	_test_eof63:  this->cs = 63; goto _test_eof; 
	_test_eof64:  this->cs = 64; goto _test_eof; 
	_test_eof65:  this->cs = 65; goto _test_eof; 
	_test_eof66:  this->cs = 66; goto _test_eof; 
	_test_eof67:  this->cs = 67; goto _test_eof; 
	_test_eof68:  this->cs = 68; goto _test_eof; 
	_test_eof69:  this->cs = 69; goto _test_eof; 
	_test_eof70:  this->cs = 70; goto _test_eof; 
	_test_eof71:  this->cs = 71; goto _test_eof; 
	_test_eof72:  this->cs = 72; goto _test_eof; 
	_test_eof73:  this->cs = 73; goto _test_eof; 
	_test_eof74:  this->cs = 74; goto _test_eof; 
	_test_eof75:  this->cs = 75; goto _test_eof; 
	_test_eof76:  this->cs = 76; goto _test_eof; 
	_test_eof77:  this->cs = 77; goto _test_eof; 
	_test_eof78:  this->cs = 78; goto _test_eof; 
	_test_eof79:  this->cs = 79; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 210 "src/http/req_parser.rl"

    if (cs >= http_parser_first_final) {
      bytes_read = static_cast<size_t>(p - data);
      return RequestParsingStatus::Finished;
    } else if (cs == http_parser_error) {
      return RequestParsingStatus::Error;
    } else {
      if (!mark) {
        LOG_ERROR("mark pointer is null");
        return RequestParsingStatus::Error;
      }
      partial_buf = std::string(mark, static_cast<size_t>(pe - mark));
      mark_active = true;
      return RequestParsingStatus::NeedMoreData;
    }   
}

};
