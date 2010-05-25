//
// detail/reactive_socket_service_base.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_REACTIVE_SOCKET_SERVICE_BASE_IPP
#define ASIO_DETAIL_IMPL_REACTIVE_SOCKET_SERVICE_BASE_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/reactive_socket_service_base.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

reactive_socket_service_base::reactive_socket_service_base(
    asio::io_service& io_service)
  : reactor_(use_service<reactor>(io_service))
{
  reactor_.init_task();
}

void reactive_socket_service_base::shutdown_service()
{
}

void reactive_socket_service_base::construct(
    reactive_socket_service_base::base_implementation_type& impl)
{
  impl.socket_ = invalid_socket;
  impl.state_ = 0;
}

void reactive_socket_service_base::destroy(
    reactive_socket_service_base::base_implementation_type& impl)
{
  if (impl.socket_ != invalid_socket)
  {
    reactor_.close_descriptor(impl.socket_, impl.reactor_data_);

    asio::error_code ignored_ec;
    socket_ops::close(impl.socket_, impl.state_, true, ignored_ec);
  }
}

asio::error_code reactive_socket_service_base::close(
    reactive_socket_service_base::base_implementation_type& impl,
    asio::error_code& ec)
{
  if (is_open(impl))
    reactor_.close_descriptor(impl.socket_, impl.reactor_data_);

  if (socket_ops::close(impl.socket_, impl.state_, true, ec) == 0)
    construct(impl);

  return ec;
}

asio::error_code reactive_socket_service_base::cancel(
    reactive_socket_service_base::base_implementation_type& impl,
    asio::error_code& ec)
{
  if (!is_open(impl))
  {
    ec = asio::error::bad_descriptor;
    return ec;
  }

  reactor_.cancel_ops(impl.socket_, impl.reactor_data_);
  ec = asio::error_code();
  return ec;
}

void reactive_socket_service_base::start_op(
    reactive_socket_service_base::base_implementation_type& impl,
    int op_type, reactor_op* op, bool non_blocking, bool noop)
{
  if (!noop)
  {
    if ((impl.state_ & socket_ops::non_blocking)
        || socket_ops::set_internal_non_blocking(
          impl.socket_, impl.state_, op->ec_))
    {
      reactor_.start_op(op_type, impl.socket_,
          impl.reactor_data_, op, non_blocking);
      return;
    }
  }

  reactor_.post_immediate_completion(op);
}

void reactive_socket_service_base::start_accept_op(
    reactive_socket_service_base::base_implementation_type& impl,
    reactor_op* op, bool peer_is_open)
{
  if (!peer_is_open)
    start_op(impl, reactor::read_op, op, true, false);
  else
  {
    op->ec_ = asio::error::already_open;
    reactor_.post_immediate_completion(op);
  }
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_REACTIVE_SOCKET_SERVICE_BASE_IPP