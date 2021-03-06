#ifndef PROTON_CPP_LINK_H
#define PROTON_CPP_LINK_H

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

#include <proton/endpoint.hpp>
#include <proton/export.hpp>
#include <proton/message.hpp>
#include <proton/source.hpp>
#include <proton/target.hpp>
#include <proton/object.hpp>
#include <proton/sender_options.hpp>
#include <proton/receiver_options.hpp>

#include <proton/types.h>

#include <string>

namespace proton {

class sender;
class receiver;
class error_condition;
class link_context;
class proton_event;
class messaging_adapter;
class proton_handler;
class delivery;
class connection;
class container;
class session;
class sender_iterator;
class receiver_iterator;

/// A named channel for sending or receiving messages.  It is the base
/// class for sender and receiver.
class
PN_CPP_CLASS_EXTERN link : public internal::object<pn_link_t> , public endpoint {
    /// @cond INTERNAL
    link(pn_link_t* l) : internal::object<pn_link_t>(l) {}
    /// @endcond

  public:
    link() : internal::object<pn_link_t>(0) {}

    // Endpoint behaviours
    PN_CPP_EXTERN bool uninitialized() const;
    PN_CPP_EXTERN bool active() const;
    PN_CPP_EXTERN bool closed() const;

    PN_CPP_EXTERN class error_condition error() const;

    /// Locally close the link.  The operation is not complete till
    /// handler::on_link_close.
    PN_CPP_EXTERN void close();

    /// Initiate close with an error condition.
    /// The operation is not complete till handler::on_connection_close().
    PN_CPP_EXTERN void close(const error_condition&);

    /// Suspend the link without closing it.  A suspended link may be
    /// reopened with the same or different link options if supported by
    /// the peer. A suspended durable subscriptions becomes inactive
    /// without cancelling it.
    PN_CPP_EXTERN void detach();

    /// Credit available on the link.
    PN_CPP_EXTERN int credit() const;

    /// True for a receiver if a drain cycle has been started and the
    /// corresponding on_receiver_drain_finish event is still pending.
    /// @see receiver::drain.  True for a sender if the receiver has
    /// requested a drain of credit and the sender has unused credit.
    PN_CPP_EXTERN bool draining();

    /// Get the link name.
    PN_CPP_EXTERN std::string name() const;

    /// Return the container for this link
    PN_CPP_EXTERN class container &container() const;

    /// Connection that owns this link.
    PN_CPP_EXTERN class connection connection() const;

    /// Session that owns this link.
    PN_CPP_EXTERN class session session() const;

    ///@cond INTERNAL
  protected:
    /// Initiate the AMQP attach frame.  The operation is not complete till
    /// handler::on_link_open.
    void attach();

    friend class internal::factory<link>;
    ///@endcond
};

}

#endif // PROTON_CPP_LINK_H
