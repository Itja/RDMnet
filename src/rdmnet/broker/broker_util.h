/******************************************************************************
************************* IMPORTANT NOTE -- READ ME!!! ************************
*******************************************************************************
* THIS SOFTWARE IMPLEMENTS A **DRAFT** STANDARD, BSR E1.33 REV. 77. UNDER NO
* CIRCUMSTANCES SHOULD THIS SOFTWARE BE USED FOR ANY PRODUCT AVAILABLE FOR
* GENERAL SALE TO THE PUBLIC. DUE TO THE INEVITABLE CHANGE OF DRAFT PROTOCOL
* VALUES AND BEHAVIORAL REQUIREMENTS, PRODUCTS USING THIS SOFTWARE WILL **NOT**
* BE INTEROPERABLE WITH PRODUCTS IMPLEMENTING THE FINAL RATIFIED STANDARD.
*******************************************************************************
* Copyright 2019 ETC Inc.
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

/// \file broker_util.h
#ifndef _BROKER_UTIL_H_
#define _BROKER_UTIL_H_

#include <stdexcept>
#include <queue>

#include "lwpa/lock.h"
#include "rdmnet/core/rpt_prot.h"

// Guard classes for locking and unlocking mutexes and read-write locks

class BrokerMutexGuard
{
public:
  explicit BrokerMutexGuard(lwpa_mutex_t &mutex) : m_mutex(mutex)
  {
    if (!lwpa_mutex_take(&m_mutex, LWPA_WAIT_FOREVER))
      throw std::runtime_error("Broker failed to take a mutex.");
  }
  ~BrokerMutexGuard() { lwpa_mutex_give(&m_mutex); }

private:
  lwpa_mutex_t &m_mutex;
};

class BrokerReadGuard
{
public:
  explicit BrokerReadGuard(lwpa_rwlock_t &rwlock) : m_rwlock(rwlock)
  {
    if (!lwpa_rwlock_readlock(&m_rwlock, LWPA_WAIT_FOREVER))
      throw std::runtime_error("Broker failed to take a read lock.");
  }
  ~BrokerReadGuard() { lwpa_rwlock_readunlock(&m_rwlock); }

private:
  lwpa_rwlock_t &m_rwlock;
};

class BrokerWriteGuard
{
public:
  explicit BrokerWriteGuard(lwpa_rwlock_t &rwlock) : m_rwlock(rwlock)
  {
    if (!lwpa_rwlock_writelock(&m_rwlock, LWPA_WAIT_FOREVER))
      throw std::runtime_error("Broker failed to take a write lock.");
  }
  ~BrokerWriteGuard() { lwpa_rwlock_writeunlock(&m_rwlock); }

private:
  lwpa_rwlock_t &m_rwlock;
};

// Utility functions for manipulating messages
RptHeader SwapHeaderData(const RptHeader &source);
std::vector<RdmBuffer> RdmBufListToVect(const RdmBufListEntry *list_head);

#endif  // _BROKER_UTIL_H_
