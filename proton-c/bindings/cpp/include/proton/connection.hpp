#ifndef PROTON_CPP_CONNECTION_H
#define PROTON_CPP_CONNECTION_H

/*
 *
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
 *
 */

#include "proton/export.hpp"
#include "proton/endpoint.hpp"
#include "proton/object.hpp"
#include "proton/session.hpp"
#include "proton/types.h"
#include <string>

struct pn_connection_t;

namespace proton {

class handler;
class connection_options;
class sender;
class sender_options;
class receiver;
class receiver_options;

namespace io {
class connection_engine;
}

/// A connection to a remote AMQP peer.
class
PN_CPP_CLASS_EXTERN connection : public internal::object<pn_connection_t>, public endpoint {
    /// @cond INTERNAL
    connection(pn_connection_t* c) : internal::object<pn_connection_t>(c) {}
    /// @endcond

  public:
    connection() : internal::object<pn_connection_t>(0) {}

    /// Get the state of this connection.
    PN_CPP_EXTERN bool uninitialized() const;
    PN_CPP_EXTERN bool active() const;
    PN_CPP_EXTERN bool closed() const;

    PN_CPP_EXTERN class error_condition error() const;

    /// Get the container.
    ///
    /// @throw proton::error if this connection is not managed by a
    /// container
    PN_CPP_EXTERN class container &container() const;

    /// Get the transport for the connection.
    PN_CPP_EXTERN class transport transport() const;

    /// Return the AMQP host name for the connection.
    PN_CPP_EXTERN std::string virtual_host() const;

    /// Return the container ID for the connection.
    PN_CPP_EXTERN std::string container_id() const;

    /// Initiate local open.  The operation is not complete till
    /// handler::on_connection_open().
    PN_CPP_EXTERN void open();
    PN_CPP_EXTERN void open(const connection_options &);

    /// Initiate local close.  The operation is not complete till
    /// handler::on_connection_close().
    PN_CPP_EXTERN void close();

    /// Initiate close with an error condition.
    /// The operation is not complete till handler::on_connection_close().
    PN_CPP_EXTERN void close(const error_condition&);

    /// Open a new session.
    PN_CPP_EXTERN session open_session();
    PN_CPP_EXTERN session open_session(const session_options &);

    /// Get the default session.  A default session is created on the
    /// first call and reused for the lifetime of the connection.
    PN_CPP_EXTERN session default_session();

    /// Open a sender for `addr` on default_session().
    PN_CPP_EXTERN sender open_sender(const std::string &addr);
    PN_CPP_EXTERN sender open_sender(const std::string &addr, const sender_options &);

    /// Open a receiver for `addr` on default_session().
    PN_CPP_EXTERN receiver open_receiver(const std::string &addr);
    PN_CPP_EXTERN receiver open_receiver(const std::string &addr,
                                         const receiver_options &);

    /// Return all sessions on this connection.
    PN_CPP_EXTERN session_range sessions() const;

    /// Return all receivers on this connection.
    PN_CPP_EXTERN receiver_range receivers() const;

    /// Return all senders on this connection.
    PN_CPP_EXTERN sender_range senders() const;

    PN_CPP_EXTERN uint32_t max_frame_size() const;
    PN_CPP_EXTERN uint16_t max_sessions() const;
    PN_CPP_EXTERN uint32_t idle_timeout() const;

    /// @cond INTERNAL
  private:
    void user(const std::string &);
    void password(const std::string &);

    friend class internal::factory<connection>;
    friend class connector;
    /// @endcond
};

}

#endif // PROTON_CPP_CONNECTION_H
