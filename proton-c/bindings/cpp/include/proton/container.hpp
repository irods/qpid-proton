#ifndef PROTON_CPP_CONTAINER_H
#define PROTON_CPP_CONTAINER_H

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

#include "proton/duration.hpp"
#include "proton/export.hpp"
#include "proton/pn_unique_ptr.hpp"

#include <string>

namespace proton {

class connection;
class connection_options;
class acceptor;
class handler;
class handler;
class receiver;
class receiver_options;
class sender;
class sender_options;
class task;
class container_impl;

namespace internal {
class link;
}

/// A top-level container of connections, sessions, senders and receivers.
///
/// A container gives a unique identity to each communicating peer. It
/// is often a process-level object.

/// It serves as an entry point to the API, allowing connections, senders
/// and receivers to be established. It can be supplied with an event handler
/// in order to intercept important messaging events, such as newly
/// received messages or newly issued credit for sending
/// messages.
class container {
  public:
    /// Create a container.
    ///
    /// Container ID should be unique within your system. By default a
    /// random ID is generated.
    ///
    /// This container will not be very useful unless event handlers are supplied
    /// as options when creating a connection/listener/sender or receiver.
    PN_CPP_EXTERN container();
    PN_CPP_EXTERN container(const std::string& id);

    /// Create a container with an event handler.
    ///
    /// Container ID should be unique within your system. By default a
    /// random ID is generated.
    PN_CPP_EXTERN container(handler& mhandler);
    PN_CPP_EXTERN container(handler& mhandler, const std::string& id);

    PN_CPP_EXTERN ~container();

    /// Open a connection to `url`.
    PN_CPP_EXTERN connection connect(const std::string& url);
    PN_CPP_EXTERN connection connect(const std::string& url,
                                     const connection_options &opts);

    /// Listen on `url` for incoming connections.
    PN_CPP_EXTERN acceptor listen(const std::string &url);
    PN_CPP_EXTERN acceptor listen(const std::string &url,
                                  const connection_options &opts);

    /// Start processing events. It returns when all connections and
    /// acceptors are closed.
    PN_CPP_EXTERN void run();

    /// Open a connection to `url` and open a sender for `url.path()`.
    /// Any supplied sender or connection options will override the
    /// container's template options.
    PN_CPP_EXTERN sender open_sender(const std::string &url);
    PN_CPP_EXTERN sender open_sender(const std::string &url,
                                     const proton::sender_options &o);
    PN_CPP_EXTERN sender open_sender(const std::string &url,
                                     const proton::sender_options &o,
                                     const connection_options &c);

    /// Open a connection to `url` and open a receiver for
    /// `url.path()`.  Any supplied receiver or connection options will
    /// override the container's template options.
    PN_CPP_EXTERN receiver open_receiver(const std::string&url);
    PN_CPP_EXTERN receiver open_receiver(const std::string&url,
                                         const proton::receiver_options &o);
    PN_CPP_EXTERN receiver open_receiver(const std::string&url,
                                         const proton::receiver_options &o,
                                         const connection_options &c);

    /// A unique identifier for the container.
    PN_CPP_EXTERN std::string id() const;

    /// @cond INTERNAL
    /// XXX settle some API questions
    /// Schedule a timer task event in delay milliseconds.
    PN_CPP_EXTERN task schedule(int delay);
    PN_CPP_EXTERN task schedule(int delay, handler *h);

    /// @endcond

    /// Copy the connection options to a template which will be
    /// applied to subsequent outgoing connections.  These are applied
    /// first and overriden by additional connection options provided
    /// in other methods.
    PN_CPP_EXTERN void client_connection_options(const connection_options &);

    /// Copy the connection options to a template which will be
    /// applied to incoming connections.  These are applied before the
    /// first open event on the connection.
    PN_CPP_EXTERN void server_connection_options(const connection_options &);

    /// Copy the sender options to a template applied to new senders
    /// created and opened by this container.  They are applied before
    /// the open event on the sender and may be overriden by sender
    /// options in other methods.
    PN_CPP_EXTERN void sender_options(const sender_options &);

    /// Copy the receiver options to a template applied to new receivers
    /// created and opened by this container.  They are applied before
    /// the open event on the receiver and may be overriden by receiver
    /// options in other methods.
    PN_CPP_EXTERN void receiver_options(const receiver_options &);

    /// @cond INTERNAL
  private:
    internal::pn_unique_ptr<container_impl> impl_;

    friend class connector;
    friend class messaging_adapter;
    friend class receiver_options;
    friend class sender_options;
    friend class session_options;
    /// @endcond
};

}

#endif // PROTON_CPP_CONTAINER_H
