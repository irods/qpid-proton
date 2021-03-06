/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */


#include "test_bits.hpp"
#include <proton/uuid.hpp>
#include <proton/io/connection_engine.hpp>
#include <proton/handler.hpp>
#include <proton/types_fwd.hpp>
#include <proton/link.hpp>
#include <deque>
#include <algorithm>

#if __cplusplus < 201103L
#define override
#endif

namespace {

using namespace proton::io;
using namespace proton;
using namespace test;
using namespace std;

typedef std::deque<char> byte_stream;

/// In memory connection_engine that reads and writes from byte_streams
struct in_memory_engine : public connection_engine {

    byte_stream& reads;
    byte_stream& writes;

    in_memory_engine(byte_stream& rd, byte_stream& wr, handler &h,
                     const connection_options &opts = connection_options()) :
        connection_engine(h, opts), reads(rd), writes(wr) {}

    void do_read() {
        mutable_buffer rbuf = read_buffer();
        size_t size = std::min(reads.size(), rbuf.size);
        if (size) {
            copy(reads.begin(), reads.begin()+size, static_cast<char*>(rbuf.data));
            read_done(size);
            reads.erase(reads.begin(), reads.begin()+size);
        }
    }

    void do_write() {
        const_buffer wbuf = write_buffer();
        if (wbuf.size) {
            writes.insert(writes.begin(),
                          static_cast<const char*>(wbuf.data),
                          static_cast<const char*>(wbuf.data) + wbuf.size);
            write_done(wbuf.size);
        }
    }

    void process() { do_read(); do_write(); dispatch(); }
};

/// A pair of engines that talk to each other in-memory.
struct engine_pair {
    byte_stream ab, ba;
    in_memory_engine a, b;

    engine_pair(handler& ha, handler& hb,
                const connection_options& ca = connection_options(),
                const connection_options& cb = connection_options()) :
        a(ba, ab, ha, ca), b(ab, ba, hb, cb) {}

    void process() { a.process(); b.process(); }
};

template <class S> typename S::value_type quick_pop(S& s) {
    ASSERT(!s.empty());
    typename S::value_type x = s.front();
    s.pop_front();
    return x;
}

/// A handler that records incoming endpoints, errors etc.
struct record_handler : public handler {
    std::deque<proton::receiver> receivers;
    std::deque<proton::sender> senders;
    std::deque<proton::session> sessions;
    std::deque<std::string> unhandled_errors, transport_errors, connection_errors;

    void on_receiver_open(receiver &l) override {
        receivers.push_back(l);
    }

    void on_sender_open(sender &l) override {
        senders.push_back(l);
    }

    void on_session_open(session &s) override {
        sessions.push_back(s);
    }

    void on_transport_error(transport& t) override {
        transport_errors.push_back(t.error().what());
    }

    void on_connection_error(connection& c) override {
        connection_errors.push_back(c.error().what());
    }

    void on_error(const proton::error_condition& c) override {
        unhandled_errors.push_back(c.what());
    }
};

void test_engine_container_id() {
    // Set container ID and prefix explicitly
    record_handler ha, hb;
    engine_pair e(ha, hb,
                  connection_options().container_id("a"),
                  connection_options().container_id("b"));
    e.a.connection().open();
    ASSERT_EQUAL("a", e.a.connection().container_id());
    e.b.connection().open();
    ASSERT_EQUAL("b", e.b.connection().container_id());
}

void test_endpoint_close() {
    record_handler ha, hb;
    engine_pair e(ha, hb);
    e.a.connection().open();
    e.a.connection().open_sender("x");
    e.a.connection().open_receiver("y");
    while (ha.senders.size()+ha.receivers.size() < 2 ||
           hb.senders.size()+hb.receivers.size() < 2) e.process();
    proton::link ax = quick_pop(ha.senders), ay = quick_pop(ha.receivers);
    proton::link bx = quick_pop(hb.receivers), by = quick_pop(hb.senders);

    // Close a link
    ax.close(proton::error_condition("err", "foo bar"));
    while (!bx.closed()) e.process();
    proton::error_condition c = bx.error();
    ASSERT_EQUAL("err", c.name());
    ASSERT_EQUAL("foo bar", c.description());
    ASSERT_EQUAL("err: foo bar", c.what());

    // Close a link with an empty condition
    ay.close(proton::error_condition());
    while (!by.closed()) e.process();
    ASSERT(by.error().empty());

    // Close a connection
    connection ca = e.a.connection(), cb = e.b.connection();
    ca.close(proton::error_condition("conn", "bad connection"));
    while (!cb.closed()) e.process();
    ASSERT_EQUAL("conn: bad connection", cb.error().what());
    ASSERT_EQUAL(1u, hb.connection_errors.size());
    ASSERT_EQUAL("conn: bad connection", hb.connection_errors.front());
}

void test_transport_close() {
    // Make sure an engine close calls the local on_transport_error() and aborts the remote.
    record_handler ha, hb;
    engine_pair e(ha, hb);
    e.a.connection().open();
    while (!e.b.connection().active()) e.process();
    e.a.close(proton::error_condition("oops", "engine failure"));
    ASSERT(!e.a.dispatch());    // Final dispatch on a.
    ASSERT_EQUAL(1u, ha.transport_errors.size());
    ASSERT_EQUAL("oops: engine failure", ha.transport_errors.front());
    ASSERT_EQUAL(proton::error_condition("oops", "engine failure"),e.a.transport().error());
    // But connectoin was never protocol closed.
    ASSERT(!e.a.connection().closed());
    ASSERT_EQUAL(0u, ha.connection_errors.size());
}

}

int main(int, char**) {
    int failed = 0;
    RUN_TEST(failed, test_engine_container_id());
    RUN_TEST(failed, test_endpoint_close());
    RUN_TEST(failed, test_transport_close());
    return failed;
}
