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

/*! \file rdmnet/rptprot.h
 *  \brief Functions to pack, send and parse RPT PDUs and their encapsulated
 *         messages.
 *  \author Sam Kearney
 */
#ifndef _RDMNET_RPTPROT_H_
#define _RDMNET_RPTPROT_H_

#include <stddef.h>
#include "lwpa_int.h"
#include "lwpa_cid.h"
#include "lwpa_error.h"
#include "lwpa_rootlayerpdu.h"
#include "estardmnet.h"
#include "rdmnet/rdmtypes.h"
#include "rdmnet/opts.h"

/*! \addtogroup rdmnet_message
 *  @{
 */

/*! The header size of an RPT PDU (not including encapsulating PDUs) */
#define RPT_PDU_HEADER_SIZE                                                   \
  (3 /* Flags + Length */ + 4 /* Vector */ + 6         /* Source UID */       \
   + 2 /* Source Endpoint ID */ + 6 /* Dest UID */ + 2 /* Dest Endpoint ID */ \
   + 4 /* Sequence Number */ + 1 /* Reserved */)
/*! The header size of an RPT PDU, including encapsulating PDUs */
#define RPT_PDU_FULL_HEADER_SIZE (RPT_PDU_HEADER_SIZE + RLP_HEADER_SIZE_EXT_LEN + ACN_TCP_PREAMBLE_SIZE)

/*! The header size of an RPT Status PDU (not including encapsulating PDUs) */
#define RPT_STATUS_HEADER_SIZE (3 /* Flags + Length */ + 2 /* Vector */)
/*! The maximum length of the Status String portion of an RPT Status message. */
#define RPT_STATUS_STRING_MAXLEN 1024
/*! The maximum length of an RPT Status message, including all encapsulating PDUs. */
#define RPT_STATUS_FULL_MSG_MAX_SIZE (RPT_PDU_FULL_HEADER_SIZE + RPT_STATUS_HEADER_SIZE + RPT_STATUS_STRING_MAXLEN)

/*! The header of an RPT message. */
typedef struct RptHeader
{
  /*! The UID of the RPT Component that originated this message. */
  LwpaUid source_uid;
  /*! Identifier for the Endpoint from which this message originated. */
  uint16_t source_endpoint_id;
  /*! The UID of the RPT Component to which this message is addressed. */
  LwpaUid dest_uid;
  /*! Identifier for the Endpoint to which this message is directed. */
  uint16_t dest_endpoint_id;
  /*! A sequence number that identifies this RPT Transaction. */
  uint32_t seqnum;
} RptHeader;

/*! RPT status code defines for the RptStatusMsg */
typedef enum
{
  /*! The Destination UID in the RPT PDU could not be found. */
  kRptStatusUnknownRptUid = VECTOR_RPT_STATUS_UNKNOWN_RPT_UID,
  /*! No RDM response was received from a Gateway's RDM responder. */
  kRptStatusRdmTimeout = VECTOR_RPT_STATUS_RDM_TIMEOUT,
  /*! An invalid RDM response was received from a Gateway's RDM responder. */
  kRptStatusInvalidRdmResponse = VECTOR_RPT_STATUS_RDM_INVALID_RESPONSE,
  /*! The Destination UID in an encapsulated RDM Command could not be found. */
  kRptStatusUnknownRdmUid = VECTOR_RPT_STATUS_UNKNOWN_RDM_UID,
  /*! The Destination Endpoint ID in the RPT PDU could not be found. */
  kRptStatusUnknownEndpoint = VECTOR_RPT_STATUS_UNKNOWN_ENDPOINT,
  /*! A Broadcasted RPT Request was sent to at least one Device. */
  kRptStatusBroadcastComplete = VECTOR_RPT_STATUS_BROADCAST_COMPLETE,
  /*! An RPT PDU was received with an unsupported Vector. */
  kRptStatusUnknownVector = VECTOR_RPT_STATUS_UNKNOWN_VECTOR,
  /*! The inner PDU contained by the RPT PDU was malformed. */
  kRptStatusInvalidMessage = VECTOR_RPT_STATUS_INVALID_MESSAGE,
  /*! The Command Class of an encapsulated RDM Command was invalid. */
  kRptStatusInvalidCommandClass = VECTOR_RPT_STATUS_INVALID_COMMAND_CLASS
} rpt_status_code_t;

/*! The RPT Status message in the RPT protocol. */
typedef struct RptStatusMsg
{
  /*! A status code that indicates the specific error or status condition. */
  rpt_status_code_t status_code;
#if RDMNET_DYNAMIC_MEM && !defined(__DOXYGEN__)
  const char *status_string;
#else
  /*! An optional implementation-defined status string to accompany this status message. */
  char status_string[RPT_STATUS_STRING_MAXLEN + 1];
#endif
} RptStatusMsg;

typedef struct RdmCmdListEntry RdmCmdListEntry;

/*! An entry in a linked list of packed RDM commands. */
struct RdmCmdListEntry
{
  RdmBuffer msg;
  RdmCmdListEntry *next;
};

/*! A list of packed RDM Commands. Two types of RPT messages contain an RdmCmdList:
 *  Request and Notification. */
typedef struct RdmCmdList
{
  /*! This message contains a partial list. This can be set when the library runs out
   *  of static memory in which to store RDM Commands and must deliver the partial
   *  list before continuing. The application should store the entries in the list
   *  but should not act on the list until another RdmCmdList is received with
   *  partial set to false. */
  bool partial;
  /*! The head of a linked list of packed RDM Commands. */
  RdmCmdListEntry *list;
} RdmCmdList;

/*! An RPT message. */
typedef struct RptMessage
{
  /*! The vector indicates which type of message is present in the data section.
   *  Valid values are indicated by VECTOR_RPT_* in estardmnet.h. */
  uint32_t vector;
  /*! The header contains routing information and metadata for the RPT message. */
  RptHeader header;
  /*! The encapsulated message; use the helper macros to access it. */
  union
  {
    RptStatusMsg status;
    RdmCmdList rdm;
  } data;
} RptMessage;

/*! \brief Determine whether an RptMessage contains an RDM Command List. Multiple types
 *         of RPT Messages can contain RDM Command Lists.
 *  \param rptmsgptr Pointer to RptMessage.
 *  \return (true or false) Whether the message contains an RDM Command List. */
#define is_rdm_cmd_list(rptmsgptr) \
  ((rptmsgptr)->vector == VECTOR_RPT_REQUEST || (rptmsgptr)->vector == VECTOR_RPT_NOTIFICATION)
/*! \brief Get the encapsulated RDM Command List from an RptMessage.
 *  \param rptmsgptr Pointer to RptMessage.
 *  \return Pointer to encapsulated RDM Command List (RdmCmdList *). */
#define get_rdm_cmd_list(rptmsgptr) (&(rptmsgptr)->data.rdm)
/*! \brief Determine whether an RptMessage contains an RPT Status Message.
 *  \param rptmsgptr Pointer to RptMessage.
 *  \return (true or false) Whether the message contains an RPT Status Message. */
#define is_rpt_status_msg(rptmsgptr) ((rptmsgptr)->vector == VECTOR_RPT_STATUS)
/*! \brief Get the encapsulated RPT Status message from an RptMessage.
 *  \param rptmsgptr Pointer to RptMessage.
 *  \return Pointer to encapsulated RPT Status Message (RptStatusMsg *). */
#define get_rpt_status_msg(rptmsgptr) (&(rptmsgptr)->data.status)

#ifdef __cplusplus
extern "C" {
#endif

size_t bufsize_rpt_request(const RdmBuffer *cmd);
size_t bufsize_rpt_status(const RptStatusMsg *status);
size_t bufsize_rpt_notification(const RdmCmdListEntry *cmd_list);

size_t pack_rpt_request(uint8_t *buf, size_t buflen, const LwpaCid *local_cid, const RptHeader *header,
                        const RdmBuffer *cmd);
size_t pack_rpt_status(uint8_t *buf, size_t buflen, const LwpaCid *local_cid, const RptHeader *header,
                       const RptStatusMsg *status);
size_t pack_rpt_notification(uint8_t *buf, size_t buflen, const LwpaCid *local_cid, const RptHeader *header,
                             const RdmCmdListEntry *cmd_list);

lwpa_error_t send_rpt_request(int handle, const LwpaCid *local_cid, const RptHeader *header, const RdmBuffer *cmd);
lwpa_error_t send_rpt_status(int handle, const LwpaCid *local_cid, const RptHeader *header, const RptStatusMsg *status);
lwpa_error_t send_rpt_notification(int handle, const LwpaCid *local_cid, const RptHeader *header,
                                   const RdmCmdListEntry *cmd_list);

#ifdef __cplusplus
}
#endif

/*!@}*/

#endif /* _RDMNET_RPTPROT_H_ */
