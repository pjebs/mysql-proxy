/* $%BEGINLICENSE%$
 Copyright (c) 2008, 2009, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; version 2 of the
 License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA

 $%ENDLICENSE%$ */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "network-mysqld-proto.h"
#include "network-mysqld-packet.h"
#include "network_mysqld_type.h"
#include "glib-ext.h"

#if GLIB_CHECK_VERSION(2, 16, 0)
#define C(x) x, sizeof(x) - 1
#define S(x) x->str, x->len

/**
 * Tests for the MySQL Protocol Codec functions
 * @ingroup proto
 */

/*@{*/
void t_ok_packet_new(void) {
	network_mysqld_ok_packet_t *ok_packet;

	ok_packet = network_mysqld_ok_packet_new();
	g_assert(ok_packet);

	network_mysqld_ok_packet_free(ok_packet);
}

void t_ok_packet_append(void) {
	network_mysqld_ok_packet_t *ok_packet;
	network_packet *packet;

	ok_packet = network_mysqld_ok_packet_new();
	packet = network_packet_new();
	packet->data = g_string_new(NULL);

	/* check if a empty ok-packet is encoded correctly */
	g_assert_cmpint(0, ==, network_mysqld_proto_append_ok_packet(packet->data, ok_packet));
	g_assert_cmpint(7, ==, packet->data->len);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet->data), C("\x00\x00\x00\x00\x00\x00\x00")));

	g_assert_cmpint(0, ==, network_mysqld_proto_get_ok_packet(packet, ok_packet));

	/* check if encoding and decoding works */
	ok_packet->warnings = 1;
	ok_packet->server_status = 2;
	ok_packet->insert_id = 3;
	ok_packet->affected_rows = 4;

	g_string_truncate(packet->data, 0);
	packet->offset = 0;

	g_assert_cmpint(0, ==, network_mysqld_proto_append_ok_packet(packet->data, ok_packet));
	g_assert_cmpint(7, ==, packet->data->len);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet->data), C("\x00\x04\x03\x02\x00\x01\x00")));
	
	network_mysqld_ok_packet_free(ok_packet);

	ok_packet = network_mysqld_ok_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_get_ok_packet(packet, ok_packet));
	g_assert_cmpint(1, ==, ok_packet->warnings);
	g_assert_cmpint(2, ==, ok_packet->server_status);
	g_assert_cmpint(3, ==, ok_packet->insert_id);
	g_assert_cmpint(4, ==, ok_packet->affected_rows);

	network_mysqld_ok_packet_free(ok_packet);

	/* check if too-short packet is denied */
	ok_packet = network_mysqld_ok_packet_new();
	g_string_truncate(packet->data, 0);
	packet->offset = 0;
	g_assert_cmpint(-1, ==, network_mysqld_proto_get_ok_packet(packet, ok_packet));

	network_mysqld_ok_packet_free(ok_packet);

	g_string_free(packet->data, TRUE);
	network_packet_free(packet);
}

void t_err_packet_new(void) {
	network_mysqld_err_packet_t *err_packet;

	err_packet = network_mysqld_err_packet_new();
	g_assert(err_packet);

	network_mysqld_err_packet_free(err_packet);
}

void t_err_packet_append(void) {
	network_mysqld_err_packet_t *err_packet;
	network_packet *packet;

	err_packet = network_mysqld_err_packet_new();
	packet = network_packet_new();
	packet->data = g_string_new(NULL);

	/* check if a empty ok-packet is encoded correctly */
	g_assert_cmpint(0, ==, network_mysqld_proto_append_err_packet(packet->data, err_packet));
	g_assert_cmpint(9, ==, packet->data->len);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet->data), C("\xff\x00\x00#07000")));

	g_assert_cmpint(0, ==, network_mysqld_proto_get_err_packet(packet, err_packet));

	/* check if encoding and decoding works */
	err_packet->errcode = 3;
	g_string_assign_len(err_packet->errmsg, C("test"));
	g_string_assign_len(err_packet->sqlstate, C("01234"));

	g_string_truncate(packet->data, 0);
	packet->offset = 0;

	g_assert_cmpint(0, ==, network_mysqld_proto_append_err_packet(packet->data, err_packet));
	g_assert_cmpint(13, ==, packet->data->len);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet->data), C("\xff\x03\x00#01234test")));
	
	network_mysqld_err_packet_free(err_packet);

	err_packet = network_mysqld_err_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_get_err_packet(packet, err_packet));
	g_assert_cmpint(3, ==, err_packet->errcode);
	g_assert_cmpstr("01234", ==, err_packet->sqlstate->str);
	g_assert_cmpstr("test", ==, err_packet->errmsg->str);

	network_mysqld_err_packet_free(err_packet);

	/* check if too-short packet is denied */
	err_packet = network_mysqld_err_packet_new();
	g_string_truncate(packet->data, 0);
	packet->offset = 0;
	g_assert_cmpint(-1, ==, network_mysqld_proto_get_err_packet(packet, err_packet));

	network_mysqld_err_packet_free(err_packet);

	g_string_free(packet->data, TRUE);
	network_packet_free(packet);
}

void t_eof_packet_new(void) {
	network_mysqld_eof_packet_t *eof_packet;

	eof_packet = network_mysqld_eof_packet_new();
	g_assert(eof_packet);

	network_mysqld_eof_packet_free(eof_packet);
}

void t_eof_packet_append(void) {
	network_mysqld_eof_packet_t *eof_packet;
	network_packet *packet;

	eof_packet = network_mysqld_eof_packet_new();
	packet = network_packet_new();
	packet->data = g_string_new(NULL);

	/* check if a empty ok-packet is encoded correctly */
	g_assert_cmpint(0, ==, network_mysqld_proto_append_eof_packet(packet->data, eof_packet));
	g_assert_cmpint(5, ==, packet->data->len);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet->data), C("\xfe\x00\x00\x00\x00")));

	g_assert_cmpint(0, ==, network_mysqld_proto_get_eof_packet(packet, eof_packet));

	/* check if encoding and decoding works */
	eof_packet->warnings = 1;
	eof_packet->server_status = 2;

	g_string_truncate(packet->data, 0);
	packet->offset = 0;

	g_assert_cmpint(0, ==, network_mysqld_proto_append_eof_packet(packet->data, eof_packet));
	g_assert_cmpint(5, ==, packet->data->len);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet->data), C("\xfe\x01\x00\x02\x00")));
	
	network_mysqld_eof_packet_free(eof_packet);

	eof_packet = network_mysqld_eof_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_get_eof_packet(packet, eof_packet));
	g_assert_cmpint(1, ==, eof_packet->warnings);
	g_assert_cmpint(2, ==, eof_packet->server_status);

	network_mysqld_eof_packet_free(eof_packet);

	/* check if too-short packet is denied */
	eof_packet = network_mysqld_eof_packet_new();
	g_string_truncate(packet->data, 0);
	packet->offset = 0;
	g_assert_cmpint(-1, ==, network_mysqld_proto_get_eof_packet(packet, eof_packet));

	network_mysqld_eof_packet_free(eof_packet);

	g_string_free(packet->data, TRUE);
	network_packet_free(packet);
}

void test_mysqld_handshake(void) {
	const char raw_packet[] = "J\0\0\0"
		"\n"
		"5.0.45-Debian_1ubuntu3.3-log\0"
		"w\0\0\0"
		"\"L;!3|8@"
		"\0"
		",\242" /* 0x2c 0xa2 */
		"\10"
		"\2\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"vV,s#PLjSA+Q"
		"\0";
	network_mysqld_auth_challenge *shake;
	network_packet packet;

	shake = network_mysqld_auth_challenge_new();
	
	packet.data = g_string_new(NULL);
	packet.offset = 0;
	g_string_append_len(packet.data, C(raw_packet));

	g_assert_cmpint(packet.data->len, ==, 78);

	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_auth_challenge(&packet, shake));

	g_assert(shake->server_version == 50045);
	g_assert(shake->thread_id == 119);
	g_assert(shake->server_status == 
			SERVER_STATUS_AUTOCOMMIT);
	g_assert(shake->charset == 8);
	g_assert(shake->capabilities ==
			(CLIENT_CONNECT_WITH_DB |
			CLIENT_LONG_FLAG |

			CLIENT_COMPRESS |

			CLIENT_PROTOCOL_41 |

			CLIENT_TRANSACTIONS |
			CLIENT_SECURE_CONNECTION));

	g_assert(shake->challenge->len == 20);
	g_assert(0 == memcmp(shake->challenge->str, "\"L;!3|8@vV,s#PLjSA+Q", shake->challenge->len));

	/* ... and back */
	g_string_truncate(packet.data, 0);
	g_string_append_len(packet.data, C("J\0\0\0"));
	network_mysqld_proto_append_auth_challenge(packet.data, shake);

	g_assert_cmpint(packet.data->len, ==, sizeof(raw_packet) - 1);

	g_assert(0 == memcmp(packet.data->str, raw_packet, packet.data->len));

	network_mysqld_auth_challenge_free(shake);
	g_string_free(packet.data, TRUE);
}

void test_mysqld_auth_empty_pw(void) {
	const char raw_packet[] = 
		"&\0\0\1\205\246\3\0\0\0\0\1\10\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0root\0\0"
		;
	GString *packet;
	network_mysqld_auth_response *auth;

	auth = network_mysqld_auth_response_new();
	g_string_assign(auth->username, "root");
	auth->capabilities    = 
		(CLIENT_LONG_PASSWORD |
	       	CLIENT_LONG_FLAG |
		CLIENT_LOCAL_FILES | 
		CLIENT_PROTOCOL_41 |
		CLIENT_INTERACTIVE |
		CLIENT_TRANSACTIONS |
		CLIENT_SECURE_CONNECTION |
		CLIENT_MULTI_STATEMENTS |
		CLIENT_MULTI_RESULTS); 
	auth->max_packet_size = 1 << 24;
	auth->charset         = 8;
	
	packet = g_string_new(NULL);

	network_mysqld_proto_append_int8(packet, 38);
	network_mysqld_proto_append_int8(packet, 0);
	network_mysqld_proto_append_int8(packet, 0);
	network_mysqld_proto_append_int8(packet, 1);

	g_assert(0 == network_mysqld_proto_append_auth_response(packet, auth));

#if 0
	g_message("%s: packet->len = %d, packet is: %d", G_STRLOC, packet->len, sizeof(raw_packet) - 1);
#endif

	g_assert(packet->len == sizeof(raw_packet) - 1);

#if 0
	for (i = 0; i < packet->len; i++) {
		g_message("%s: [%d] %02x %c= %02x", G_STRLOC, i, packet->str[i], packet->str[i] == raw_packet[i] ? '=' : '!', raw_packet[i]);
	}
#endif

	g_assert(0 == memcmp(packet->str, raw_packet, sizeof(raw_packet) - 1));

	network_mysqld_auth_response_free(auth);

	g_string_free(packet, TRUE);
}

void test_mysqld_auth_with_pw(void) {
	const char raw_packet[] = 
		":\0\0\1"
		"\205\246\3\0"
		"\0\0\0\1"
		"\10"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"root\0"
		"\24\241\304\260>\255\1:F,\256\337K\323\340\4\273\354I\256\204"
		;
	const char raw_challenge[] = 
			"%@R[SoWC"      /* part 1 */
			"+L|LG_+R={tV"; /* part 2 */

	GString *packet, *challenge, *hashed_password;
	network_mysqld_auth_response *auth;

	auth = network_mysqld_auth_response_new();
	g_string_assign(auth->username, "root");
	auth->capabilities    = 
		CLIENT_LONG_PASSWORD |
	       	CLIENT_LONG_FLAG |
		CLIENT_LOCAL_FILES | 
		CLIENT_PROTOCOL_41 |
		CLIENT_INTERACTIVE |
		CLIENT_TRANSACTIONS |
		CLIENT_SECURE_CONNECTION |
		CLIENT_MULTI_STATEMENTS |
		CLIENT_MULTI_RESULTS; 
	auth->max_packet_size = 1 << 24;
	auth->charset         = 8;

	challenge = g_string_new(NULL);
	hashed_password = g_string_new(NULL);
	g_string_append_len(challenge, raw_challenge, sizeof(raw_challenge) - 1);

	network_mysqld_proto_password_hash(hashed_password, C("123"));
	network_mysqld_proto_password_scramble(auth->response, S(challenge), S(hashed_password));

	g_string_free(hashed_password, TRUE);
	
	packet = g_string_new(NULL);

	network_mysqld_proto_append_int8(packet, 58);
	network_mysqld_proto_append_int8(packet, 0);
	network_mysqld_proto_append_int8(packet, 0);
	network_mysqld_proto_append_int8(packet, 1);

	g_assert(0 == network_mysqld_proto_append_auth_response(packet, auth));
	g_assert(packet->len == sizeof(raw_packet) - 1);

#if 0
	for (i = 0; i < packet->len; i++) {
		g_message("%s: [%d] %02x %c= %02x", G_STRLOC, i, packet->str[i], packet->str[i] == raw_packet[i] ? '=' : '!', raw_packet[i]);
	}
#endif

	g_assert(0 == memcmp(packet->str, raw_packet, sizeof(raw_packet) - 1));

	network_mysqld_auth_response_free(auth);

	g_string_free(packet, TRUE);
	g_string_free(challenge, TRUE);
}

/**
 * @test
 *   network_mysqld_auth_response_new() and network_mysqld_auth_response_free() 
 *   don't cause a crash 
 */
void t_auth_response_new() {
	network_mysqld_auth_response *shake;

	shake = network_mysqld_auth_response_new();
	g_assert(shake);

	network_mysqld_auth_response_free(shake);
}

/**
 * @test
 *   network_mysqld_proto_get_auth_response() can decode a string
 *   network_mysqld_proto_append_auth_response() can encode the result
 *     of the network_mysqld_proto_get_auth_response() 
 */
void t_mysqld_get_auth_response(void) {
	const char raw_packet[] = 
		"\205\246\3\0"
		"\0\0\0\1"
		"\10"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"root\0"
		"\24\241\304\260>\255\1:F,\256\337K\323\340\4\273\354I\256\204"
		;

	network_mysqld_auth_response *auth;
	network_packet packet;
	int err = 0;

	auth = network_mysqld_auth_response_new();
	packet.data = g_string_new_len(C(raw_packet));
	packet.offset = 0;
	
	err = err || network_mysqld_proto_get_auth_response(&packet, auth);

	g_assert_cmpint(err, ==, 0);

	g_assert(auth->username);
	g_assert_cmpint(auth->username->len, ==, 4);
	g_assert_cmpstr(auth->username->str, ==, "root");

	g_assert_cmpuint(auth->capabilities, ==,
		CLIENT_LONG_PASSWORD |
	       	CLIENT_LONG_FLAG |
		CLIENT_LOCAL_FILES | 
		CLIENT_PROTOCOL_41 |
		CLIENT_INTERACTIVE |
		CLIENT_TRANSACTIONS |
		CLIENT_SECURE_CONNECTION |
		CLIENT_MULTI_STATEMENTS |
		CLIENT_MULTI_RESULTS); 
	g_assert_cmpuint(auth->max_packet_size, ==, 1 << 24);
	g_assert_cmpuint(auth->charset        , ==, 8);

	g_string_truncate(packet.data, 0);
	packet.offset = 0;

	err = err || network_mysqld_proto_append_auth_response(packet.data, auth);
	g_assert_cmpint(err, ==, 0);

	g_assert_cmpint(packet.data->len, ==, sizeof(raw_packet) - 1);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet.data), raw_packet, packet.data->len));

	network_mysqld_auth_response_free(auth);

	/* empty auth struct */
	g_string_truncate(packet.data, 0);
	packet.offset = 0;

	auth = network_mysqld_auth_response_new();
	err = err || network_mysqld_proto_append_auth_response(packet.data, auth);
	g_assert_cmpint(err, ==, 0);
	network_mysqld_auth_response_free(auth);

	g_string_free(packet.data, TRUE);
}

/**
 * @test
 *   network_mysqld_proto_get_auth_response() can decode a pre-4.0 packet
 *   network_mysqld_proto_append_auth_response() can encode the result
 *     of the network_mysqld_proto_get_auth_response() 
 */
void t_mysqld_get_auth_response_pre_41(void) {
	const char raw_packet[] = 
		"\205$"
		"\0\0\0"
		"root\0"
		;

	network_mysqld_auth_response *auth;
	network_packet packet;
	int err = 0;

	auth = network_mysqld_auth_response_new();
	packet.data = g_string_new_len(C(raw_packet));
	packet.offset = 0;

	err = err || network_mysqld_proto_get_auth_response(&packet, auth);

	g_assert_cmpint(err, ==, 0);

	g_assert(auth->username);
	g_assert_cmpint(auth->username->len, ==, 4);
	g_assert_cmpstr(auth->username->str, ==, "root");

	g_assert_cmpuint(auth->capabilities, ==,
		CLIENT_LONG_PASSWORD |
	       	CLIENT_LONG_FLAG |
		CLIENT_LOCAL_FILES | 
		CLIENT_INTERACTIVE |
		CLIENT_TRANSACTIONS 
		); 
	g_assert_cmpuint(auth->max_packet_size, ==, 0);

	g_string_truncate(packet.data, 0);
	packet.offset = 0;

	err = err || network_mysqld_proto_append_auth_response(packet.data, auth);
	g_assert_cmpint(err, ==, 0);

	g_assert_cmpint(packet.data->len, ==, sizeof(raw_packet) - 1);
	g_assert_cmpint(TRUE, ==, g_memeq(S(packet.data), raw_packet, packet.data->len));

	network_mysqld_auth_response_free(auth);

	/* empty auth struct */
	g_string_truncate(packet.data, 0);
	packet.offset = 0;

	auth = network_mysqld_auth_response_new();
	err = err || network_mysqld_proto_append_auth_response(packet.data, auth);
	g_assert_cmpint(err, ==, 0);
	network_mysqld_auth_response_free(auth);

	g_string_free(packet.data, TRUE);
}

/**
 * @test
 *   network_mysqld_proto_get_auth_response() can decode a broken pre-4.0 packet
 */
void t_mysqld_get_auth_response_no_term(void) {
	const char raw_packet[] = 
		"\205$"
		"\0\0\0"
		"root\0" /* missing trailing \0 */
		"foo"
		;

	network_mysqld_auth_response *auth;
	network_packet packet;
	int err = 0;

	auth = network_mysqld_auth_response_new();
	packet.data = g_string_new_len(C(raw_packet));
	packet.offset = 0;

	err = err || network_mysqld_proto_get_auth_response(&packet, auth);

	g_assert_cmpint(err, !=, 0);

	network_mysqld_auth_response_free(auth);

	g_string_free(packet.data, TRUE);
}


typedef struct {
	const char *s;
	size_t s_len;
} strings;

void t_resultset_fields_works(void) {
	strings packets[] = {
		{ C("\1\0\0\1\2") }, /* 2 fields */
		{ C("6\0\0\2\3def\0\6STATUS\0\rVariable_name\rVariable_name\f\10\0P\0\0\0\375\1\0\0\0\0") },
		{ C("&\0\0\3\3def\0\6STATUS\0\5Value\5Value\f\10\0\0\2\0\0\375\1\0\0\0\0") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ C("\23\0\0\5\17Aborted_clients\00298") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ NULL, 0 }
	};
	int i;
	network_queue *q;
	GPtrArray *fields;

	q = network_queue_new();

	for (i = 0; packets[i].s; i++) {
		network_queue_append(q, g_string_new_len(packets[i].s, packets[i].s_len));
	}

	fields = g_ptr_array_new();
	g_assert(NULL != network_mysqld_proto_get_fielddefs(q->chunks->head, fields));

	network_queue_free(q);
}

void t_resultset_fields_parse_err(void) {
	strings packets[] = {
		{ C("\1\0\0\1\377") }, /* err-packet */
		{ C("6\0\0\2\3def\0\6STATUS\0\rVariable_name\rVariable_name\f\10\0P\0\0\0\375\1\0\0\0\0") },
		{ C("&\0\0\3\3def\0\6STATUS\0\5Value\5Value\f\10\0\0\2\0\0\375\1\0\0\0\0") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ C("\23\0\0\5\17Aborted_clients\00298") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ NULL, 0 }
	};
	int i;
	network_queue *q;
	GPtrArray *fields;

	q = network_queue_new();

	for (i = 0; packets[i].s; i++) {
		network_queue_append(q, g_string_new_len(packets[i].s, packets[i].s_len));
	}

	fields = g_ptr_array_new();
	g_assert(NULL == network_mysqld_proto_get_fielddefs(q->chunks->head, fields));

	network_queue_free(q);
}

void t_resultset_fields_parse_null(void) {
	strings packets[] = {
		{ C("\1\0\0\1\373") }, /* NULL */
		{ C("6\0\0\2\3def\0\6STATUS\0\rVariable_name\rVariable_name\f\10\0P\0\0\0\375\1\0\0\0\0") },
		{ C("&\0\0\3\3def\0\6STATUS\0\5Value\5Value\f\10\0\0\2\0\0\375\1\0\0\0\0") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ C("\23\0\0\5\17Aborted_clients\00298") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ NULL, 0 }
	};
	int i;
	network_queue *q;
	GPtrArray *fields;

	q = network_queue_new();

	for (i = 0; packets[i].s; i++) {
		network_queue_append(q, g_string_new_len(packets[i].s, packets[i].s_len));
	}

	fields = g_ptr_array_new();
	g_assert(NULL == network_mysqld_proto_get_fielddefs(q->chunks->head, fields));

	network_queue_free(q);
}

void t_resultset_fields_parse_low(void) {
	strings packets[] = {
		{ C("\1\0\0\1\2") }, /* 2 fields */
		{ C("6\0\0\2\3def\0\6STATUS\0\rVariable_name\rVariable_name\f\10\0P\0\0\0\375\1\0\0\0\0") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ C("\23\0\0\5\17Aborted_clients\00298") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ NULL, 0 }
	};
	int i;
	network_queue *q;
	GPtrArray *fields;

	q = network_queue_new();

	for (i = 0; packets[i].s; i++) {
		network_queue_append(q, g_string_new_len(packets[i].s, packets[i].s_len));
	}

	fields = g_ptr_array_new();
	g_assert(NULL == network_mysqld_proto_get_fielddefs(q->chunks->head, fields));

	network_queue_free(q);
}

void t_resultset_fields_parse_high(void) {
	strings packets[] = {
		{ C("\1\0\0\1\2") }, /* 2 fields */
		{ C("6\0\0\2\3def\0\6STATUS\0\rVariable_name\rVariable_name\f\10\0P\0\0\0\375\1\0\0\0\0") },
		{ C("&\0\0\3\3def\0\6STATUS\0\5Value\5Value\f\10\0\0\2\0\0\375\1\0\0\0\0") },
		{ C("&\0\0\3\3def\0\6STATUS\0\5Value\5Value\f\10\0\0\2\0\0\375\1\0\0\0\0") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ C("\23\0\0\5\17Aborted_clients\00298") },
		{ C("\5\0\0\4\376\0\0\"\0") }, /* EOF */
		{ NULL, 0 }
	};
	int i;
	network_queue *q;
	GPtrArray *fields;

	q = network_queue_new();

	for (i = 0; packets[i].s; i++) {
		network_queue_append(q, g_string_new_len(packets[i].s, packets[i].s_len));
	}

	fields = g_ptr_array_new();
	g_assert(NULL == network_mysqld_proto_get_fielddefs(q->chunks->head, fields));

	network_queue_free(q);
}

/* prepared statements */

/* COM_STMT_PREPARE */
static void t_com_stmt_prepare_new(void) {
	network_mysqld_stmt_prepare_packet_t *cmd;

	cmd = network_mysqld_stmt_prepare_packet_new();
	g_assert(cmd);

	network_mysqld_stmt_prepare_packet_free(cmd);
}

static void t_com_stmt_prepare_from_packet(void) {
	network_mysqld_stmt_prepare_packet_t *cmd;
	const char raw_packet[] = "\x1c\x00\x00\x00\x16SELECT CONCAT(?, ?) AS col1";
	network_packet packet;

	packet.data = g_string_new_len(C(raw_packet));
	packet.offset = 0;

	cmd = network_mysqld_stmt_prepare_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_stmt_prepare_packet(&packet, cmd));
	g_assert_cmpint(sizeof("SELECT CONCAT(?, ?) AS col1") - 1, ==, cmd->stmt_text->len);
	g_assert_cmpstr("SELECT CONCAT(?, ?) AS col1", ==, cmd->stmt_text->str);

	network_mysqld_stmt_prepare_packet_free(cmd);
}

/* COM_STMT_PREPARE OK-result */

static void t_com_stmt_prepare_ok_new(void) {
	network_mysqld_stmt_prepare_ok_packet_t *cmd;

	cmd = network_mysqld_stmt_prepare_ok_packet_new();
	g_assert(cmd);

	network_mysqld_stmt_prepare_ok_packet_free(cmd);
}

/**
 * test if we parse all the fields of a COM_STMT_PREPARE-ok response correctly
 */
static void t_com_stmt_prepare_ok_from_packet(void) {
	network_mysqld_stmt_prepare_ok_packet_t *cmd;
	network_mysqld_eof_packet_t *eof;
	network_mysqld_proto_fielddef_t *coldef;

	/* a response for the COM_STMT_PREPARE command
	 *
	 * the OK part with stmt-id and so on is in the first packet. The others are
	 * the field-defs, a EOF, the param-defs, and the last EOF */
	strings packets[] = {
		{ C("\x0c\x00\x00\x01\x00\x01\x00\x00\x00\x01\x00\x02\x00\x00\x00\x00") }, /* the PREPARE OK packet */
		{ C("\x17\x00\x00\x02\x03\x64\x65\x66\x00\x00\x00\x01\x3f\x00\x0c\x3f\x00\x00\x00\x00\x00\xfd\x80\x00\x00\x00\x00") }, /* column-def: param 1 */
		{ C("\x17\x00\x00\x03\x03\x64\x65\x66\x00\x00\x00\x01\x3f\x00\x0c\x3f\x00\x00\x00\x00\x00\xfd\x80\x00\x00\x00\x00") }, /* column-def: param 2 */
		{ C("\x05\x00\x00\x04\xfe\x00\x00\x02\x00") }, /* the seperator */
		{ C("\x1a\x00\x00\x05\x03\x64\x65\x66\x00\x00\x00\x04\x63\x6f\x6c\x31\x00\x0c\x3f\x00\x00\x00\x00\x00\xfd\x80\x00\x1f\x00\x00") }, /* column-def: result-col 1 */
		{ C("\x05\x00\x00\x06\xfe\x00\x00\x02\x00") } /* the terminator */
	};
	network_packet packet;

	packet.data = g_string_new_len(packets[0].s, packets[0].s_len);
	packet.offset = 0;

	cmd = network_mysqld_stmt_prepare_ok_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_stmt_prepare_ok_packet(&packet, cmd));
	g_assert_cmpint(1, ==, cmd->stmt_id);
	g_assert_cmpint(1, ==, cmd->num_columns);
	g_assert_cmpint(2, ==, cmd->num_params);
	g_assert_cmpint(0, ==, cmd->warnings);
	
	network_mysqld_stmt_prepare_ok_packet_free(cmd);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);

	packet.data = g_string_new_len(packets[1].s, packets[1].s_len);
	packet.offset = 0;

	coldef = network_mysqld_proto_fielddef_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_fielddef(&packet, coldef, CLIENT_PROTOCOL_41));

	network_mysqld_proto_fielddef_free(coldef);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);


	packet.data = g_string_new_len(packets[2].s, packets[2].s_len);
	packet.offset = 0;

	coldef = network_mysqld_proto_fielddef_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_fielddef(&packet, coldef, CLIENT_PROTOCOL_41));
	network_mysqld_proto_fielddef_free(coldef);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);


	packet.data = g_string_new_len(packets[3].s, packets[3].s_len);
	packet.offset = 0;

	eof = network_mysqld_eof_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_eof_packet(&packet, eof));

	network_mysqld_eof_packet_free(eof);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);


	packet.data = g_string_new_len(packets[4].s, packets[4].s_len);
	packet.offset = 0;

	coldef = network_mysqld_proto_fielddef_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_fielddef(&packet, coldef, CLIENT_PROTOCOL_41));
	network_mysqld_proto_fielddef_free(coldef);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);


	packet.data = g_string_new_len(packets[5].s, packets[5].s_len);
	packet.offset = 0;

	eof = network_mysqld_eof_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_eof_packet(&packet, eof));

	network_mysqld_eof_packet_free(eof);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);
}

/* COM_STMT_EXECUTE */

static void t_com_stmt_execute_new(void) {
	network_mysqld_stmt_execute_packet_t *cmd;

	cmd = network_mysqld_stmt_execute_packet_new();
	g_assert(cmd);

	network_mysqld_stmt_execute_packet_free(cmd);
}

static void t_com_stmt_execute_from_packet(void) {
	network_mysqld_stmt_execute_packet_t *cmd;
	const char raw_packet[] = "\x1a\x00\x00\x00\x17\x01\x00\x00\x00\x00\x01\x00\x00\x00\x01\x01\xfe\x00\xfe\x00\xfe\x00\x03" "bar" "\x03" "foo";
	network_packet packet;
	network_mysqld_type_t *type;

	packet.data = g_string_new_len(C(raw_packet));
	packet.offset = 0;

	cmd = network_mysqld_stmt_execute_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_stmt_execute_packet(&packet, cmd, 3));
	g_assert_cmpint(1, ==, cmd->stmt_id);
	g_assert_cmpint(0, ==, cmd->flags);
	g_assert_cmpint(1, ==, cmd->iteration_count);
	g_assert_cmpint(1, ==, cmd->new_params_bound);

	/* we should have 2 string params */
	type = g_ptr_array_index(cmd->params, 0);
	g_assert(type);
	g_assert_cmpint(MYSQL_TYPE_STRING, ==, type->type);
	g_assert_cmpint(TRUE, ==, type->is_null);

	type = g_ptr_array_index(cmd->params, 1);
	g_assert(type);
	g_assert_cmpint(MYSQL_TYPE_STRING, ==, type->type);
	g_assert_cmpint(FALSE, ==, type->is_null);

	type = g_ptr_array_index(cmd->params, 2);
	g_assert(type);
	g_assert_cmpint(MYSQL_TYPE_STRING, ==, type->type);
	g_assert_cmpint(FALSE, ==, type->is_null);

	network_mysqld_stmt_execute_packet_free(cmd);
}

/* COM_STMT_EXECUTE result */

/**
 * test if we parse all the fields of a COM_STMT_EXECUTE result correctly
 */
static void t_com_stmt_execute_result_from_packet(void) {
	network_mysqld_eof_packet_t *eof;
	network_mysqld_proto_fielddefs_t *coldefs;
	network_mysqld_proto_fielddef_t *coldef;
	network_mysqld_resultset_row_t *row;
	network_mysqld_type_t *field;
	GString *data;
	guint64 field_count;
	int packet_id = 0;

	/* response for a 
	 *   SELECT ? AS col2, CONCAT(?, ?) AS col1
	 * with
	 *   1: NULL
	 *   2: STRING bar
	 *   3: STRING foo
	 */
	strings packets[] = {
		{ C("\x01\x00\x00\x01\x02") },
		{ C("\x1a\x00\x00\x02\x03\x64\x65\x66\x00\x00\x00\x04\x63\x6f\x6c\x32\x00\x0c\x3f\x00\x00\x00\x00\x00\xfe\x80\x00\x00\x00\x00") },
		{ C("\x1a\x00\x00\x03\x03\x64\x65\x66\x00\x00\x00\x04\x63\x6f\x6c\x31\x00\x0c\x08\x00\x06\x00\x00\x00\xfd\x00\x00\x1f\x00\x00") },
		{ C("\x05\x00\x00\x04\xfe\x00\x00\x02\x00") },
		{ C("\x09\x00\x00\x05\x00\x04\x06" "barfoo") },
		{ C("\x05\x00\x00\x06\xfe\x00\x00\x02\x00") }

	};
	network_packet packet;

	/* the field-count */
	packet_id = 0;
	packet.data = g_string_new_len(packets[packet_id].s, packets[packet_id].s_len);
	packet.offset = 0;

	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_lenenc_int(&packet, &field_count));
	g_assert_cmpint(2, ==, field_count);
	
	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);


	/* the colum defs */
	packet_id++;
	packet.data = g_string_new_len(packets[packet_id].s, packets[packet_id].s_len);
	packet.offset = 0;

	coldefs = network_mysqld_proto_fielddefs_new();
	coldef = network_mysqld_proto_fielddef_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_fielddef(&packet, coldef, CLIENT_PROTOCOL_41));

	g_ptr_array_add(coldefs, coldef);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);

	/* the string column */
	packet_id++;
	packet.data = g_string_new_len(packets[packet_id].s, packets[packet_id].s_len);
	packet.offset = 0;

	coldef = network_mysqld_proto_fielddef_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_fielddef(&packet, coldef, CLIENT_PROTOCOL_41));

	g_ptr_array_add(coldefs, coldef);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);

	/* the EOF */	
	packet_id++;
	packet.data = g_string_new_len(packets[packet_id].s, packets[packet_id].s_len);
	packet.offset = 0;

	eof = network_mysqld_eof_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_eof_packet(&packet, eof));

	network_mysqld_eof_packet_free(eof);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);

	/* the row */
	packet_id++;
	packet.data = g_string_new_len(packets[packet_id].s, packets[packet_id].s_len);
	packet.offset = 0;

	row = network_mysqld_resultset_row_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_binary_row(&packet, coldefs, row));

	/* check if the 1st field is NULL */
	field = g_ptr_array_index(row, 0);
	g_assert(field);
	g_assert_cmpint(TRUE, ==, field->is_null);

	/* check if the 2nd field is "barfoo" */
	field = g_ptr_array_index(row, 1);
	g_assert(field);
	g_assert_cmpint(MYSQL_TYPE_VAR_STRING, ==, field->type);
	g_assert_cmpint(FALSE, ==, field->is_null);

	/* FIXME: find a way to test this without touching the internal representation */
	data = field->data;
	g_assert(data);
	g_assert_cmpint(data->len, ==, 6);
	g_assert_cmpstr(data->str, ==, "barfoo");

	network_mysqld_resultset_row_free(row);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);

	packet_id++;
	packet.data = g_string_new_len(packets[packet_id].s, packets[packet_id].s_len);
	packet.offset = 0;

	eof = network_mysqld_eof_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_eof_packet(&packet, eof));

	network_mysqld_eof_packet_free(eof);

	g_assert_cmpint(packet.offset, ==, packet.data->len); /* is everything parsed */
	g_string_free(packet.data, TRUE);
	
	network_mysqld_proto_fielddefs_free(coldefs);
}



/* COM_STMT_CLOSE */
static void t_com_stmt_close_new(void) {
	network_mysqld_stmt_close_packet_t *cmd;

	cmd = network_mysqld_stmt_close_packet_new();
	g_assert(cmd);

	network_mysqld_stmt_close_packet_free(cmd);
}

static void t_com_stmt_close_from_packet(void) {
	network_mysqld_stmt_close_packet_t *cmd;
	const char raw_packet[] = "\x05\x00\x00\x00\x19\x01\x00\x00\x00";
	network_packet packet;
	packet.data = g_string_new_len(C(raw_packet));
	packet.offset = 0;

	cmd = network_mysqld_stmt_close_packet_new();
	g_assert_cmpint(0, ==, network_mysqld_proto_skip_network_header(&packet));
	g_assert_cmpint(0, ==, network_mysqld_proto_get_stmt_close_packet(&packet, cmd));
	g_assert_cmpint(1, ==, cmd->stmt_id);

	network_mysqld_stmt_close_packet_free(cmd);
}


/**
 * @cond
 *   don't include the main() function the docs
 */
int main(int argc, char **argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_bug_base("http://bugs.mysql.com/");

	g_test_add_func("/core/ok-packet-new", t_ok_packet_new);
	g_test_add_func("/core/ok-packet-append", t_ok_packet_append);
	g_test_add_func("/core/eof-packet-new", t_eof_packet_new);
	g_test_add_func("/core/eof-packet-append", t_eof_packet_append);
	g_test_add_func("/core/err-packet-new", t_err_packet_new);
	g_test_add_func("/core/err-packet-append", t_err_packet_append);

	g_test_add_func("/core/mysqld-proto-handshake", test_mysqld_handshake);

	g_test_add_func("/core/mysqld-proto-pw-empty", test_mysqld_auth_empty_pw);
	g_test_add_func("/core/mysqld-proto-pw", test_mysqld_auth_with_pw);
	
	g_test_add_func("/core/mysqld-proto-auth-response-new", t_auth_response_new);
	g_test_add_func("/core/mysqld-proto-get-auth-response", t_mysqld_get_auth_response);
	g_test_add_func("/core/mysqld-proto-get-auth-response-pre-4.1", t_mysqld_get_auth_response_pre_41);
	g_test_add_func("/core/mysqld-proto-get-auth-response-no-term", t_mysqld_get_auth_response_no_term);
	
	g_test_add_func("/core/resultset-fields", t_resultset_fields_works);
	g_test_add_func("/core/resultset-fields-broken-proto-err", t_resultset_fields_parse_err);
	g_test_add_func("/core/resultset-fields-broken-proto-null", t_resultset_fields_parse_null);
	g_test_add_func("/core/resultset-fields-broken-proto-field-count-low", t_resultset_fields_parse_low);
	g_test_add_func("/core/resultset-fields-broken-proto-field-count-high", t_resultset_fields_parse_high);

	/* prepared statements */
	g_test_add_func("/core/com_stmt_prepare_new", t_com_stmt_prepare_new);
	g_test_add_func("/core/com_stmt_prepare_from_packet", t_com_stmt_prepare_from_packet);

	g_test_add_func("/core/com_stmt_prepare_ok_new", t_com_stmt_prepare_ok_new);
	g_test_add_func("/core/com_stmt_prepare_ok_from_packet", t_com_stmt_prepare_ok_from_packet);

	g_test_add_func("/core/com_stmt_execute_new", t_com_stmt_execute_new);
	g_test_add_func("/core/com_stmt_execute_from_packet", t_com_stmt_execute_from_packet);
	
	g_test_add_func("/core/com_stmt_execute_result_from_packet", t_com_stmt_execute_result_from_packet);

	g_test_add_func("/core/com_stmt_close_new", t_com_stmt_close_new);
	g_test_add_func("/core/com_stmt_close_from_packet", t_com_stmt_close_from_packet);

	return g_test_run();
}
/** @endcond */
#else
int main() {
	return 77;
}
#endif
