
#line 1 "src/http_parser/http11_parser.rl"
#include "http11_parser.h"
#include <cstring>


#line 8 "src/http_parser/http11_parser.cpp"
static const int http_parser_start = 1;
static const int http_parser_first_final = 107;
static const int http_parser_error = 0;

static const int http_parser_en_main = 1;


#line 94 "src/http_parser/http11_parser.rl"


bool parse_http_request(char const* data, size_t len, HttpRequest* result) {
    int cs; // current state
    const char* p {data}; // current position
    const char* pe {data + len}; // end position
    const char* eof {pe}; // EOF position

    HttpRequest ctx_storage {};
    HttpRequest* ctx {&ctx_storage};

    // a mark for substring extraction
    const char* mark = nullptr;
    
    
#line 32 "src/http_parser/http11_parser.cpp"
	{
	cs = http_parser_start;
	}

#line 109 "src/http_parser/http11_parser.rl"
    
#line 39 "src/http_parser/http11_parser.cpp"
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
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 67 "src/http_parser/http11_parser.cpp"
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
#line 16 "src/http_parser/http11_parser.rl"
	{
  ctx->method.assign(mark, p - mark);
}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 116 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr12;
		case 33: goto tr13;
		case 37: goto tr14;
		case 47: goto tr15;
		case 58: goto tr16;
		case 61: goto tr13;
		case 64: goto st26;
		case 95: goto tr13;
		case 126: goto tr13;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr18;
	} else
		goto tr18;
	goto st0;
tr12:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st9;
tr30:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st9;
tr56:
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st9;
tr64:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st9;
tr72:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st9;
tr80:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 28 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st9;
tr83:
#line 28 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st9;
tr133:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 217 "src/http_parser/http11_parser.cpp"
	if ( (*p) == 72 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 84 )
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
	if ( (*p) == 80 )
		goto st13;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 47 )
		goto st14;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr24;
	goto st0;
tr24:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 266 "src/http_parser/http11_parser.cpp"
	if ( (*p) == 46 )
		goto st16;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st15;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st17;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 13 )
		goto tr28;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st17;
	goto st0;
tr28:
#line 12 "src/http_parser/http11_parser.rl"
	{
  ctx->version.assign(mark, p - mark);
}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 298 "src/http_parser/http11_parser.cpp"
	if ( (*p) == 10 )
		goto st107;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	goto st107;
tr13:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 317 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr30;
		case 33: goto st19;
		case 37: goto st20;
		case 58: goto tr33;
		case 61: goto st19;
		case 64: goto st26;
		case 95: goto st19;
		case 126: goto st19;
	}
	if ( (*p) < 48 ) {
		if ( 36 <= (*p) && (*p) <= 46 )
			goto st19;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st19;
		} else if ( (*p) >= 65 )
			goto st19;
	} else
		goto st19;
	goto st0;
tr14:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 350 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st21;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st21;
	} else
		goto st21;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st19;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st19;
	} else
		goto st19;
	goto st0;
tr16:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st22;
tr33:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 393 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st36;
		} else if ( (*p) >= 36 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 48 ) {
		if ( 36 <= (*p) && (*p) <= 46 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
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
			goto st23;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st23;
	} else
		goto st23;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 32: goto tr12;
		case 33: goto tr39;
		case 37: goto tr40;
		case 58: goto tr41;
		case 61: goto tr39;
		case 95: goto tr39;
		case 126: goto tr39;
	}
	if ( (*p) < 48 ) {
		if ( 36 <= (*p) && (*p) <= 46 )
			goto tr39;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr39;
		} else if ( (*p) >= 65 )
			goto tr39;
	} else
		goto tr39;
	goto st0;
tr39:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 502 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr30;
		case 33: goto st27;
		case 37: goto st28;
		case 58: goto tr44;
		case 61: goto st27;
		case 95: goto st27;
		case 126: goto st27;
	}
	if ( (*p) < 48 ) {
		if ( 36 <= (*p) && (*p) <= 46 )
			goto st27;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st27;
		} else if ( (*p) >= 65 )
			goto st27;
	} else
		goto st27;
	goto st0;
tr40:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 534 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st29;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st29;
	} else
		goto st29;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st27;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st27;
	} else
		goto st27;
	goto st0;
tr41:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st30;
tr44:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 577 "src/http_parser/http11_parser.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st32;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( (*p) == 32 )
		goto st9;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st33;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 32 )
		goto st9;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st34;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 32 )
		goto st9;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st35;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 32 )
		goto st9;
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	switch( (*p) ) {
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st37;
		} else if ( (*p) >= 36 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	switch( (*p) ) {
		case 32: goto st9;
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st38;
		} else if ( (*p) >= 36 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	switch( (*p) ) {
		case 32: goto st9;
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st39;
		} else if ( (*p) >= 36 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 32: goto st9;
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st40;
		} else if ( (*p) >= 36 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 32: goto st9;
		case 33: goto st23;
		case 37: goto st24;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 48 ) {
		if ( 36 <= (*p) && (*p) <= 46 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
tr15:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 768 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st42;
		case 37: goto st43;
		case 61: goto st42;
		case 95: goto st42;
		case 126: goto st42;
	}
	if ( (*p) < 48 ) {
		if ( 36 <= (*p) && (*p) <= 46 )
			goto st42;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st42;
		} else if ( (*p) >= 64 )
			goto st42;
	} else
		goto st42;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st42;
		case 37: goto st43;
		case 61: goto st42;
		case 95: goto st42;
		case 126: goto st42;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st42;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st42;
	} else
		goto st42;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st44;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st44;
	} else
		goto st44;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st42;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st42;
	} else
		goto st42;
	goto st0;
tr18:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 846 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr30;
		case 33: goto st19;
		case 37: goto st20;
		case 43: goto st45;
		case 58: goto tr61;
		case 59: goto st19;
		case 61: goto st19;
		case 64: goto st26;
		case 95: goto st19;
		case 126: goto st19;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 44 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st45;
		} else if ( (*p) >= 36 )
			goto st19;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st45;
		} else if ( (*p) >= 65 )
			goto st45;
	} else
		goto st45;
	goto st0;
tr61:
#line 36 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.scheme.assign(mark, p - mark);
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
#line 888 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 33: goto st23;
		case 37: goto st24;
		case 47: goto st47;
		case 61: goto st23;
		case 64: goto st26;
		case 95: goto st23;
		case 126: goto st23;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st36;
		} else if ( (*p) >= 36 )
			goto st23;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 47 )
		goto st48;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 32: goto tr64;
		case 33: goto tr65;
		case 35: goto tr66;
		case 37: goto tr67;
		case 47: goto tr68;
		case 58: goto tr69;
		case 61: goto tr65;
		case 63: goto tr70;
		case 64: goto tr71;
		case 95: goto tr65;
		case 126: goto tr65;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr65;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr65;
	} else
		goto tr65;
	goto st0;
tr65:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st49;
tr73:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 970 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr72;
		case 33: goto tr73;
		case 35: goto tr74;
		case 37: goto tr75;
		case 47: goto tr76;
		case 58: goto tr77;
		case 61: goto tr73;
		case 63: goto tr78;
		case 64: goto tr79;
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
tr90:
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st50;
tr66:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st50;
tr74:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st50;
tr135:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 1041 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr80;
		case 33: goto tr81;
		case 37: goto tr82;
		case 61: goto tr81;
		case 95: goto tr81;
		case 126: goto tr81;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr81;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr81;
	} else
		goto tr81;
	goto st0;
tr81:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 1069 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr83;
		case 33: goto st51;
		case 37: goto st52;
		case 61: goto st51;
		case 95: goto st51;
		case 126: goto st51;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st51;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st51;
	} else
		goto st51;
	goto st0;
tr82:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1097 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st53;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st53;
	} else
		goto st53;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st51;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st51;
	} else
		goto st51;
	goto st0;
tr67:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st54;
tr75:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
#line 1144 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st55;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st55;
	} else
		goto st55;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st49;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st49;
	} else
		goto st49;
	goto st0;
tr134:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st56;
tr68:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st56;
tr76:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st56;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
#line 1197 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st56;
		case 35: goto tr90;
		case 37: goto st57;
		case 61: goto st56;
		case 63: goto tr92;
		case 95: goto st56;
		case 126: goto st56;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st56;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st56;
	} else
		goto st56;
	goto st0;
tr136:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
#line 1227 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st58;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st58;
	} else
		goto st58;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st56;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st56;
	} else
		goto st56;
	goto st0;
tr92:
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st59;
tr70:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st59;
tr78:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st59;
tr138:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 32 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.path.assign(mark, p - mark);
}
	goto st59;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
#line 1298 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr95;
		case 35: goto tr96;
	}
	goto tr94;
tr94:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st60;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
#line 1314 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
	}
	goto st60;
tr95:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st61;
tr98:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st61;
tr101:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 28 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st61;
tr104:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
#line 28 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 1364 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
		case 72: goto st66;
	}
	goto st60;
tr96:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st62;
tr99:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
#line 1391 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr101;
		case 33: goto tr102;
		case 35: goto tr99;
		case 37: goto tr103;
		case 61: goto tr102;
		case 95: goto tr102;
		case 126: goto tr102;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr102;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr102;
	} else
		goto tr102;
	goto st60;
tr102:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
#line 1420 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr104;
		case 33: goto st63;
		case 35: goto tr99;
		case 37: goto st64;
		case 61: goto st63;
		case 95: goto st63;
		case 126: goto st63;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st63;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st63;
	} else
		goto st63;
	goto st60;
tr103:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st64;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
#line 1449 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st65;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st65;
	} else
		goto st65;
	goto st60;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st63;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st63;
	} else
		goto st63;
	goto st60;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
		case 84: goto st67;
	}
	goto st60;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
		case 84: goto st68;
	}
	goto st60;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
		case 80: goto st69;
	}
	goto st60;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
		case 47: goto st70;
	}
	goto st60;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr112;
	goto st60;
tr112:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 1541 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
		case 46: goto st72;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st71;
	goto st60;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	switch( (*p) ) {
		case 32: goto tr98;
		case 35: goto tr99;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st73;
	goto st60;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	switch( (*p) ) {
		case 13: goto tr116;
		case 32: goto tr98;
		case 35: goto tr99;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st73;
	goto st60;
tr116:
#line 12 "src/http_parser/http11_parser.rl"
	{
  ctx->version.assign(mark, p - mark);
}
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 1583 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 10: goto st108;
		case 32: goto tr98;
		case 35: goto tr99;
	}
	goto st60;
tr175:
#line 12 "src/http_parser/http11_parser.rl"
	{
  ctx->version.assign(mark, p - mark);
}
	goto st108;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
#line 1600 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
	}
	goto st108;
tr157:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st109;
tr160:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 28 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st109;
tr163:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
#line 28 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.fragment.assign(mark, p - mark);
}
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1640 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
		case 72: goto st114;
	}
	goto st108;
tr158:
#line 24 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.query.assign(mark, p - mark);
}
	goto st110;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
#line 1657 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr160;
		case 33: goto tr161;
		case 35: goto tr158;
		case 37: goto tr162;
		case 61: goto tr161;
		case 95: goto tr161;
		case 126: goto tr161;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr161;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr161;
	} else
		goto tr161;
	goto st108;
tr161:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
#line 1686 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr163;
		case 33: goto st111;
		case 35: goto tr158;
		case 37: goto st112;
		case 61: goto st111;
		case 95: goto st111;
		case 126: goto st111;
	}
	if ( (*p) < 63 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st111;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st111;
	} else
		goto st111;
	goto st108;
tr162:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1715 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st113;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st113;
	} else
		goto st113;
	goto st108;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st111;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st111;
	} else
		goto st111;
	goto st108;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
		case 84: goto st115;
	}
	goto st108;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
		case 84: goto st116;
	}
	goto st108;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
		case 80: goto st117;
	}
	goto st108;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
		case 47: goto st118;
	}
	goto st108;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr171;
	goto st108;
tr171:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st119;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
#line 1807 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
		case 46: goto st120;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st119;
	goto st108;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	switch( (*p) ) {
		case 32: goto tr157;
		case 35: goto tr158;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st121;
	goto st108;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	switch( (*p) ) {
		case 13: goto tr175;
		case 32: goto tr157;
		case 35: goto tr158;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st121;
	goto st108;
tr69:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st75;
tr77:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
#line 1863 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st76;
		case 35: goto tr90;
		case 37: goto st77;
		case 47: goto st56;
		case 61: goto st76;
		case 63: goto tr92;
		case 64: goto st79;
		case 95: goto st76;
		case 126: goto st76;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st89;
		} else if ( (*p) >= 36 )
			goto st76;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st76;
		} else if ( (*p) >= 65 )
			goto st76;
	} else
		goto st76;
	goto st0;
tr142:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
#line 1901 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st76;
		case 35: goto tr90;
		case 37: goto st77;
		case 47: goto st56;
		case 61: goto st76;
		case 63: goto tr92;
		case 64: goto st79;
		case 95: goto st76;
		case 126: goto st76;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto st76;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st76;
	} else
		goto st76;
	goto st0;
tr143:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
#line 1933 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st78;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st78;
	} else
		goto st78;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st76;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st76;
	} else
		goto st76;
	goto st0;
tr145:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st79;
tr71:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st79;
tr79:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st79;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
#line 1986 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr64;
		case 33: goto tr123;
		case 35: goto tr66;
		case 37: goto tr124;
		case 47: goto tr68;
		case 58: goto tr125;
		case 61: goto tr123;
		case 63: goto tr70;
		case 64: goto tr68;
		case 95: goto tr123;
		case 126: goto tr123;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr123;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr123;
	} else
		goto tr123;
	goto st0;
tr123:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st80;
tr126:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 2033 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr72;
		case 33: goto tr126;
		case 35: goto tr74;
		case 37: goto tr127;
		case 47: goto tr76;
		case 58: goto tr128;
		case 61: goto tr126;
		case 63: goto tr78;
		case 64: goto tr76;
		case 95: goto tr126;
		case 126: goto tr126;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr126;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr126;
	} else
		goto tr126;
	goto st0;
tr124:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st81;
tr127:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 2080 "src/http_parser/http11_parser.cpp"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st82;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st82;
	} else
		goto st82;
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st80;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st80;
	} else
		goto st80;
	goto st0;
tr125:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
	goto st83;
tr128:
#line 20 "src/http_parser/http11_parser.rl"
	{
  ctx->uri.hostname.assign(mark, p - mark);
}
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st83;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
#line 2127 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st56;
		case 35: goto tr90;
		case 37: goto st57;
		case 61: goto st56;
		case 63: goto tr92;
		case 95: goto st56;
		case 126: goto st56;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st84;
		} else if ( (*p) >= 36 )
			goto st56;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st56;
		} else if ( (*p) >= 64 )
			goto st56;
	} else
		goto st56;
	goto st0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st56;
		case 35: goto tr90;
		case 37: goto st57;
		case 61: goto st56;
		case 63: goto tr92;
		case 95: goto st56;
		case 126: goto st56;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st85;
		} else if ( (*p) >= 36 )
			goto st56;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st56;
		} else if ( (*p) >= 64 )
			goto st56;
	} else
		goto st56;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr134;
		case 35: goto tr135;
		case 37: goto tr136;
		case 61: goto tr134;
		case 63: goto tr138;
		case 95: goto tr134;
		case 126: goto tr134;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr137;
		} else if ( (*p) >= 36 )
			goto tr134;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr134;
		} else if ( (*p) >= 64 )
			goto tr134;
	} else
		goto tr134;
	goto st0;
tr137:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 2221 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr134;
		case 35: goto tr135;
		case 37: goto tr136;
		case 61: goto tr134;
		case 63: goto tr138;
		case 95: goto tr134;
		case 126: goto tr134;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr139;
		} else if ( (*p) >= 36 )
			goto tr134;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr134;
		} else if ( (*p) >= 64 )
			goto tr134;
	} else
		goto tr134;
	goto st0;
tr139:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st87;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
#line 2257 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr134;
		case 35: goto tr135;
		case 37: goto tr136;
		case 61: goto tr134;
		case 63: goto tr138;
		case 95: goto tr134;
		case 126: goto tr134;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr140;
		} else if ( (*p) >= 36 )
			goto tr134;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr134;
		} else if ( (*p) >= 64 )
			goto tr134;
	} else
		goto tr134;
	goto st0;
tr140:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st88;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
#line 2293 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr134;
		case 35: goto tr135;
		case 37: goto tr136;
		case 61: goto tr134;
		case 63: goto tr138;
		case 95: goto tr134;
		case 126: goto tr134;
	}
	if ( (*p) < 64 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr134;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr134;
	} else
		goto tr134;
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	switch( (*p) ) {
		case 32: goto tr56;
		case 33: goto st76;
		case 35: goto tr90;
		case 37: goto st77;
		case 47: goto st56;
		case 61: goto st76;
		case 63: goto tr92;
		case 64: goto st79;
		case 95: goto st76;
		case 126: goto st76;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto st90;
		} else if ( (*p) >= 36 )
			goto st76;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st76;
		} else if ( (*p) >= 65 )
			goto st76;
	} else
		goto st76;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr142;
		case 35: goto tr135;
		case 37: goto tr143;
		case 47: goto tr134;
		case 61: goto tr142;
		case 63: goto tr138;
		case 64: goto tr145;
		case 95: goto tr142;
		case 126: goto tr142;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr144;
		} else if ( (*p) >= 36 )
			goto tr142;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr142;
		} else if ( (*p) >= 65 )
			goto tr142;
	} else
		goto tr142;
	goto st0;
tr144:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st91;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
#line 2385 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr142;
		case 35: goto tr135;
		case 37: goto tr143;
		case 47: goto tr134;
		case 61: goto tr142;
		case 63: goto tr138;
		case 64: goto tr145;
		case 95: goto tr142;
		case 126: goto tr142;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr146;
		} else if ( (*p) >= 36 )
			goto tr142;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr142;
		} else if ( (*p) >= 65 )
			goto tr142;
	} else
		goto tr142;
	goto st0;
tr146:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
#line 2423 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr142;
		case 35: goto tr135;
		case 37: goto tr143;
		case 47: goto tr134;
		case 61: goto tr142;
		case 63: goto tr138;
		case 64: goto tr145;
		case 95: goto tr142;
		case 126: goto tr142;
	}
	if ( (*p) < 58 ) {
		if ( (*p) > 46 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr147;
		} else if ( (*p) >= 36 )
			goto tr142;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr142;
		} else if ( (*p) >= 65 )
			goto tr142;
	} else
		goto tr142;
	goto st0;
tr147:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 2461 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 32: goto tr133;
		case 33: goto tr142;
		case 35: goto tr135;
		case 37: goto tr143;
		case 47: goto tr134;
		case 61: goto tr142;
		case 63: goto tr138;
		case 64: goto tr145;
		case 95: goto tr142;
		case 126: goto tr142;
	}
	if ( (*p) < 65 ) {
		if ( 36 <= (*p) && (*p) <= 59 )
			goto tr142;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr142;
	} else
		goto tr142;
	goto st0;
tr2:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 2493 "src/http_parser/http11_parser.cpp"
	if ( (*p) == 69 )
		goto st95;
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( (*p) == 84 )
		goto st7;
	goto st0;
tr3:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
#line 2514 "src/http_parser/http11_parser.cpp"
	if ( (*p) == 69 )
		goto st97;
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( (*p) == 65 )
		goto st98;
	goto st0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( (*p) == 68 )
		goto st7;
	goto st0;
tr4:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
#line 2542 "src/http_parser/http11_parser.cpp"
	if ( (*p) == 80 )
		goto st100;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( (*p) == 84 )
		goto st101;
	goto st0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	if ( (*p) == 73 )
		goto st102;
	goto st0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( (*p) == 79 )
		goto st103;
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( (*p) == 78 )
		goto st104;
	goto st0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( (*p) == 83 )
		goto st7;
	goto st0;
tr5:
#line 8 "src/http_parser/http11_parser.rl"
	{
  mark = p;
}
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 2591 "src/http_parser/http11_parser.cpp"
	switch( (*p) ) {
		case 79: goto st106;
		case 85: goto st95;
	}
	goto st0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( (*p) == 83 )
		goto st95;
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
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
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
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 110 "src/http_parser/http11_parser.rl"

    if (cs >= http_parser_first_final) {
      *result = ctx_storage;
      return true;
    } else {
      return false;
    }
  }
