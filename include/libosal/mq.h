/**
 * \file mq.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mq header.
 *
 * OSAL mq include header.
 */

/*
 * This file is part of libosal.
 *
 * libosal is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * libosal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with libosal; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LIBOSAL_MQ__H
#define LIBOSAL_MQ__H

#include <libosal/config.h>
#include <libosal/types.h>
#include <libosal/timer.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/mq.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/mq.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/mq.h>
#endif

/** \defgroup mq_group Message queue
 * Message queues are an asynchronous communication mechanism between two or more 
 * processes/tasks. They follow the publish/subscribe pattern.
 *
 * @{
 */

#define OSAL_MQ_ATTR__OFLAG__RDONLY           0x00000001u   //!< \brief Message queue attribute flag read-only
#define OSAL_MQ_ATTR__OFLAG__WRONLY           0x00000002u   //!< \brief Message queue attribute flag write-only
#define OSAL_MQ_ATTR__OFLAG__RDWR             0x00000004u   //!< \brief Message queue attribute flag read-write
#define OSAL_MQ_ATTR__OFLAG__CREAT            0x00000008u   //!< \brief Message queue attribute flag create
#define OSAL_MQ_ATTR__OFLAG__CLOEXEC          0x00000010u   //!< \brief Message queue attribute flag close execute
#define OSAL_MQ_ATTR__OFLAG__EXCL             0x00000020u   //!< \brief Message queue attribute flag exclusive

typedef struct osal_mq_attr {
    osal_uint32_t   oflags;                 //!< \brief Message queue open flags.
    osal_mode_t     mode;                   //!< \brief Message queue mode.
    osal_size_t     max_messages;           //!< \brief Message queue maximum number of messages.
    osal_size_t     max_message_size;       //!< \brief Message queue maximum message size.
} osal_mq_attr_t;                           //!< \brief Message queue attribute type.

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a mq.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   name    Pointer containing message queue name.
 * \param[in]   attr    Pointer to initial mq attributes. Can be NULL then
 *                      the defaults of the underlying mq will be used.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_PERMISSION_DENIED       Tried to open an existing queue without sufficient persmission.
 *                                          Tried to create an already existing message queue.
 * \retval OSAL_ERR_INVALID_PARAM           Name is not in correct format or it is too long.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    Limit of open files has been reached.
 * \retval OSAL_ERR_NOT_FOUND               Tried to open a non-existing message queue.
 * \retval OSAL_ERR_OUT_OF_MEMORY           System is out of memory.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 */
osal_retval_t osal_mq_open(osal_mq_t *mq, const osal_char_t *name,  const osal_mq_attr_t *attr);

//! \brief Send a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to send.
 * \param[in]   prio    Send priority.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_send(osal_mq_t *mq, const osal_char_t *msg, const osal_size_t msg_len, const osal_uint32_t prio);

//! \brief Send a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to send.
 * \param[in]   prio    Send priority.
 * \param[in]   to      Timeout waiting if message queue is full.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_timedsend(osal_mq_t *mq, const osal_char_t *msg, const osal_size_t msg_len, 
        const osal_uint32_t prio, const osal_timer_t *to);

//! \brief Receive a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[out]  msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to receive.
 * \param[out]  prio    Receive priority.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_receive(osal_mq_t *mq, osal_char_t *msg, const osal_size_t msg_len, osal_uint32_t *prio);

//! \brief Receive a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[out]  msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to receive.
 * \param[out]  prio    Receive priority.
 * \param[in]   to      Timeout waiting if message queue is full.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_timedreceive(osal_mq_t *mq, osal_char_t *msg, const osal_size_t msg_len, 
        osal_uint32_t *prio, const osal_timer_t *to);

//! \brief Closes an open mq.
/*!
 * \param[in]   mq     Pointer to osal mq structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_close(osal_mq_t *mq);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_MQ__H */


