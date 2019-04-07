/******************************************************************************
************************* IMPORTANT NOTE -- READ ME!!! ************************
*******************************************************************************
* THIS SOFTWARE IMPLEMENTS A **DRAFT** STANDARD, BSR E1.33 REV. 63. UNDER NO
* CIRCUMSTANCES SHOULD THIS SOFTWARE BE USED FOR ANY PRODUCT AVAILABLE FOR
* GENERAL SALE TO THE PUBLIC. DUE TO THE INEVITABLE CHANGE OF DRAFT PROTOCOL
* VALUES AND BEHAVIORAL REQUIREMENTS, PRODUCTS USING THIS SOFTWARE WILL **NOT**
* BE INTEROPERABLE WITH PRODUCTS IMPLEMENTING THE FINAL RATIFIED STANDARD.
*******************************************************************************
* Copyright 2018 ETC Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************
* This file is a part of RDMnet. For more information, go to:
* https://github.com/ETCLabs/RDMnet
******************************************************************************/

// Windows override of RDMnet::BrokerSocketManager.
// Uses Windows I/O completion ports, the most efficient and scalable socket management tool
// available from the Windows API.

#ifndef _WIN_SOCKET_MANAGER_H_
#define _WIN_SOCKET_MANAGER_H_

#include <winsock2.h>
#include <windows.h>
#include <process.h>

#include <map>
#include <vector>
#include <memory>

#include "rdmnet/broker/socket_manager.h"

// Wrapper around Windows thread functions to increase the testability of this module.
class WindowsThreadInterface
{
public:
  virtual HANDLE StartThread(_beginthreadex_proc_type start_address, void *arg_list) = 0;
  virtual BOOL CleanupThread(HANDLE thread_handle) = 0;
};

class DefaultWindowsThreads : public WindowsThreadInterface
{
public:
  virtual HANDLE StartThread(_beginthreadex_proc_type start_address, void *arg_list) override
  {
    return reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, start_address, arg_list, 0, NULL));
  }
  virtual BOOL CleanupThread(HANDLE thread_handle) override { return CloseHandle(thread_handle); }
};

// The set of data allocated per-socket.
struct SocketData
{
  SocketData()
  {
    ws_recv_buf.buf = reinterpret_cast<char *>(recv_buf.get());
    ws_recv_buf.len = kRecvBufSize;
  }

  WSAOVERLAPPED overlapped{};
  SOCKET socket{INVALID_SOCKET};
  rdmnet_conn_t conn_handle{RDMNET_CONN_INVALID};

  // Socket receive data
  WSABUF ws_recv_buf;                           // The variable Winsock uses for receive buffers
  static constexpr size_t kRecvBufSize = 1000;  // TODO get constant
  // Receive buffer for socket recv operations
  std::unique_ptr<uint8_t[]> recv_buf{std::make_unique<uint8_t[]>(kRecvBufSize)};
};

// A class to manage RDMnet Broker sockets on Windows.
// This handles receiving data on all RDMnet client connections, using I/O completion ports for
// maximum performance. Sending on connections is done in the core Broker library through the lwpa
// interface. Other miscellaneous Broker socket operations like LLRP are also handled in the core
// library.
class WinBrokerSocketManager : public RDMnet::BrokerSocketManager
{
public:
  WinBrokerSocketManager(WindowsThreadInterface *thread_interface = new DefaultWindowsThreads)
      : thread_interface_(thread_interface)
  {
  }

  // RDMnet::BrokerSocketManager interface
  bool Startup(RDMnet::BrokerSocketManagerNotify *notify) override;
  bool Shutdown() override;
  bool AddSocket(rdmnet_conn_t conn_handle, lwpa_socket_t socket) override;
  void RemoveSocket(rdmnet_conn_t conn_handle) override;

  // Callback functions called from worker threads
  void WorkerNotifySocketError(rdmnet_conn_t conn_handle);
  void WorkerNotifyRecvData(rdmnet_conn_t conn_handle);
  void WorkerNotifySocketGracefulClose(rdmnet_conn_t handle);

  // Accessors
  HANDLE iocp() { return iocp_; }

private:
  // Thread pool management
  HANDLE iocp_{nullptr};
  std::vector<HANDLE> worker_threads_;
  std::unique_ptr<WindowsThreadInterface> thread_interface_;

  // The set of sockets being managed
  std::map<rdmnet_conn_t, std::unique_ptr<SocketData>> sockets_;

  // The callback instance
  RDMnet::BrokerSocketManagerNotify *notify_{nullptr};
};

#endif  // _WIN_SOCKET_MANAGER_H_