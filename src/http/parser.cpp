
#line 1 "src/http/parser.rl"
#include "parser.h"
#include <cstring>


#line 8 "src/http/parser.cpp"
static const int http_parser_start = 1;
static const int http_parser_first_final = 75;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 105 "src/http/parser.rl"


bool parse_http_request(char const* data, size_t len, HttpRequest* result) {
    int cs; // current state
    const char* p {data}; // current position
    const char* pe {data + len}; // end position
    const char* eof {pe}; // EOF position

    HttpRequest ctx_storage {};
    HttpRequest* ctx {&ctx_storage};

    // a mark for substring extraction
    const char* mark = nullptr;
    
    
#line 32 "src/http/parser.cpp"
	{
	cs = http_parser_start;
	}

#line 120 "src/http/parser.rl"
    
#line 39 "src/http/parser.cpp"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
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
cs = 0;
	goto _out;
tr0:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 67 "src/http/parser.cpp"
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
#line 16 "src/http/parser.rl"
	{
  ctx->method.assign(mark, p - mark);
}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 116 "src/http/parser.cpp"
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
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 146 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st9;
		case 35: goto tr18;
		case 37: goto st24;
		case 47: goto st26;
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
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st10;
tr33:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 28 "src/http/parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st10;
tr36:
#line 28 "src/http/parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st10;
tr43:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 24 "src/http/parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st10;
tr47:
#line 24 "src/http/parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st10;
tr53:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st10;
tr61:
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st10;
tr82:
#line 40 "src/http/parser.rl"
	{
  ctx->uri.port.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 244 "src/http/parser.cpp"
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
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 293 "src/http/parser.cpp"
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
#line 12 "src/http/parser.rl"
	{
  ctx->version.assign(mark, p - mark);
}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 325 "src/http/parser.cpp"
	if ( (*p) == 10 )
		goto st75;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	goto st75;
tr18:
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st20;
tr45:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 24 "src/http/parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st20;
tr49:
#line 24 "src/http/parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st20;
tr55:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st20;
tr63:
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st20;
tr83:
#line 40 "src/http/parser.rl"
	{
  ctx->uri.port.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 394 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr33;
		case 33: goto tr34;
		case 37: goto tr35;
		case 61: goto tr34;
		case 95: goto tr34;
		case 126: goto tr34;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr34;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr34;
	} else
		goto tr34;
	goto st0;
tr34:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 422 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr36;
		case 33: goto st21;
		case 37: goto st22;
		case 61: goto st21;
		case 95: goto st21;
		case 126: goto st21;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st21;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st21;
	} else
		goto st21;
	goto st0;
tr35:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 450 "src/http/parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st23;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st23;
	} else
		goto st23;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st21;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st21;
	} else
		goto st21;
	goto st0;
tr13:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 483 "src/http/parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st25;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st25;
	} else
		goto st25;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st9;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st9;
	} else
		goto st9;
	goto st0;
tr57:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st26;
tr65:
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st26;
tr84:
#line 40 "src/http/parser.rl"
	{
  ctx->uri.port.assign(mark, p - mark);
}
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 540 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st26;
		case 35: goto tr18;
		case 37: goto st27;
		case 61: goto st26;
		case 63: goto tr21;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st26;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st26;
	} else
		goto st26;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
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
tr21:
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st29;
tr59:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st29;
tr67:
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st29;
tr86:
#line 40 "src/http/parser.rl"
	{
  ctx->uri.port.assign(mark, p - mark);
}
#line 32 "src/http/parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 630 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr43;
		case 33: goto tr44;
		case 35: goto tr45;
		case 37: goto tr46;
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
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 659 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr47;
		case 33: goto st30;
		case 35: goto tr49;
		case 37: goto st31;
		case 61: goto st30;
		case 95: goto st30;
		case 126: goto st30;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st30;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st30;
	} else
		goto st30;
	goto st0;
tr46:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 688 "src/http/parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st32;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st32;
	} else
		goto st32;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st30;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st30;
	} else
		goto st30;
	goto st0;
tr14:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 721 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st26;
		case 35: goto tr18;
		case 37: goto st27;
		case 47: goto st34;
		case 61: goto st26;
		case 63: goto tr21;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st26;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st26;
	} else
		goto st26;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	switch( (*p) ) {
		case 32: goto tr53;
		case 33: goto tr54;
		case 35: goto tr55;
		case 37: goto tr56;
		case 47: goto tr57;
		case 58: goto tr58;
		case 61: goto tr54;
		case 63: goto tr59;
		case 64: goto st42;
		case 95: goto tr54;
		case 126: goto tr54;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr54;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr54;
	} else
		goto tr54;
	goto st0;
tr54:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 778 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr61;
		case 33: goto st35;
		case 35: goto tr63;
		case 37: goto st36;
		case 47: goto tr65;
		case 58: goto tr66;
		case 61: goto st35;
		case 63: goto tr67;
		case 64: goto st42;
		case 95: goto st35;
		case 126: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st0;
tr56:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 811 "src/http/parser.cpp"
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
tr58:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st38;
tr66:
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 854 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st39;
		case 35: goto tr18;
		case 37: goto st40;
		case 47: goto st26;
		case 61: goto st39;
		case 63: goto tr21;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr71;
		} else if ( (*p) >= 36 )
			goto st39;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st39;
		} else if ( (*p) >= 65 )
			goto st39;
	} else
		goto st39;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st39;
		case 35: goto tr18;
		case 37: goto st40;
		case 47: goto st26;
		case 61: goto st39;
		case 63: goto tr21;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st39;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st39;
	} else
		goto st39;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st41;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st41;
	} else
		goto st41;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st39;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st39;
	} else
		goto st39;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 32: goto tr53;
		case 33: goto tr73;
		case 35: goto tr55;
		case 37: goto tr74;
		case 47: goto tr57;
		case 58: goto tr75;
		case 61: goto tr73;
		case 63: goto tr59;
		case 64: goto st26;
		case 95: goto tr73;
		case 126: goto tr73;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr73;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr73;
	} else
		goto tr73;
	goto st0;
tr73:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 969 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr61;
		case 33: goto st43;
		case 35: goto tr63;
		case 37: goto st44;
		case 47: goto tr65;
		case 58: goto tr78;
		case 61: goto st43;
		case 63: goto tr67;
		case 64: goto st26;
		case 95: goto st43;
		case 126: goto st43;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st43;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st43;
	} else
		goto st43;
	goto st0;
tr74:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 1002 "src/http/parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st45;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st45;
	} else
		goto st45;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st43;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st43;
	} else
		goto st43;
	goto st0;
tr75:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st46;
tr78:
#line 20 "src/http/parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
#line 1045 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st26;
		case 35: goto tr18;
		case 37: goto st27;
		case 61: goto st26;
		case 63: goto tr21;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr80;
		} else if ( (*p) >= 36 )
			goto st26;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st26;
		} else if ( (*p) >= 64 )
			goto st26;
	} else
		goto st26;
	goto st0;
tr80:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st47;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
#line 1081 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st26;
		case 35: goto tr18;
		case 37: goto st27;
		case 61: goto st26;
		case 63: goto tr21;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st48;
		} else if ( (*p) >= 36 )
			goto st26;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st26;
		} else if ( (*p) >= 64 )
			goto st26;
	} else
		goto st26;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st26;
		case 35: goto tr83;
		case 37: goto st27;
		case 47: goto tr84;
		case 61: goto st26;
		case 63: goto tr86;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st49;
		} else if ( (*p) >= 36 )
			goto st26;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st26;
		} else if ( (*p) >= 64 )
			goto st26;
	} else
		goto st26;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st26;
		case 35: goto tr83;
		case 37: goto st27;
		case 47: goto tr84;
		case 61: goto st26;
		case 63: goto tr86;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st50;
		} else if ( (*p) >= 36 )
			goto st26;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st26;
		} else if ( (*p) >= 64 )
			goto st26;
	} else
		goto st26;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st26;
		case 35: goto tr83;
		case 37: goto st27;
		case 47: goto tr84;
		case 61: goto st26;
		case 63: goto tr86;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st51;
		} else if ( (*p) >= 36 )
			goto st26;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st26;
		} else if ( (*p) >= 64 )
			goto st26;
	} else
		goto st26;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st26;
		case 35: goto tr83;
		case 37: goto st27;
		case 47: goto tr84;
		case 61: goto st26;
		case 63: goto tr86;
		case 95: goto st26;
		case 126: goto st26;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st26;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st26;
	} else
		goto st26;
	goto st0;
tr71:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1231 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st39;
		case 35: goto tr18;
		case 37: goto st40;
		case 47: goto st26;
		case 61: goto st39;
		case 63: goto tr21;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st53;
		} else if ( (*p) >= 36 )
			goto st39;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st39;
		} else if ( (*p) >= 65 )
			goto st39;
	} else
		goto st39;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st39;
		case 35: goto tr83;
		case 37: goto st40;
		case 47: goto tr84;
		case 61: goto st39;
		case 63: goto tr86;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st54;
		} else if ( (*p) >= 36 )
			goto st39;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st39;
		} else if ( (*p) >= 65 )
			goto st39;
	} else
		goto st39;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st39;
		case 35: goto tr83;
		case 37: goto st40;
		case 47: goto tr84;
		case 61: goto st39;
		case 63: goto tr86;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st55;
		} else if ( (*p) >= 36 )
			goto st39;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st39;
		} else if ( (*p) >= 65 )
			goto st39;
	} else
		goto st39;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st39;
		case 35: goto tr83;
		case 37: goto st40;
		case 47: goto tr84;
		case 61: goto st39;
		case 63: goto tr86;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st56;
		} else if ( (*p) >= 36 )
			goto st39;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st39;
		} else if ( (*p) >= 65 )
			goto st39;
	} else
		goto st39;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 32: goto tr82;
		case 33: goto st39;
		case 35: goto tr83;
		case 37: goto st40;
		case 47: goto tr84;
		case 61: goto st39;
		case 63: goto tr86;
		case 64: goto st42;
		case 95: goto st39;
		case 126: goto st39;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st39;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st39;
	} else
		goto st39;
	goto st0;
tr15:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
#line 1387 "src/http/parser.cpp"
	switch( (*p) ) {
		case 32: goto tr16;
		case 33: goto st9;
		case 35: goto tr18;
		case 37: goto st24;
		case 43: goto st57;
		case 47: goto st26;
		case 58: goto tr94;
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
				goto st57;
		} else if ( (*p) >= 65 )
			goto st57;
	} else
		goto st57;
	goto st0;
tr94:
#line 36 "src/http/parser.rl"
	{
  ctx->uri.scheme.assign(mark, p - mark);
}
	goto st58;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
#line 1425 "src/http/parser.cpp"
	switch( (*p) ) {
		case 33: goto st59;
		case 37: goto st60;
		case 47: goto tr14;
		case 61: goto st59;
		case 95: goto st59;
		case 126: goto st59;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st59;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st59;
	} else
		goto st59;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 32: goto st10;
		case 33: goto st59;
		case 37: goto st60;
		case 61: goto st59;
		case 95: goto st59;
		case 126: goto st59;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st59;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st59;
	} else
		goto st59;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st61;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st61;
	} else
		goto st61;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st59;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st59;
	} else
		goto st59;
	goto st0;
tr2:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
#line 1500 "src/http/parser.cpp"
	if ( (*p) == 69 )
		goto st63;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 84 )
		goto st7;
	goto st0;
tr3:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st64;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
#line 1521 "src/http/parser.cpp"
	if ( (*p) == 69 )
		goto st65;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 65 )
		goto st66;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 68 )
		goto st7;
	goto st0;
tr4:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 1549 "src/http/parser.cpp"
	if ( (*p) == 80 )
		goto st68;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 84 )
		goto st69;
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) == 73 )
		goto st70;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 79 )
		goto st71;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 78 )
		goto st72;
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 83 )
		goto st7;
	goto st0;
tr5:
#line 8 "src/http/parser.rl"
	{
  mark = p;
}
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 1598 "src/http/parser.cpp"
	switch( (*p) ) {
		case 79: goto st74;
		case 85: goto st63;
	}
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( (*p) == 83 )
		goto st63;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 121 "src/http/parser.rl"

    if (cs >= http_parser_first_final) {
      *result = ctx_storage;
      return true;
    } else {
      return false;
    }
  }
