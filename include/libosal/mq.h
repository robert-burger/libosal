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
 * libosal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libosal is distributed in the hope that 
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libosal. If not, see <http://www.gnu.org/licenses/>.
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

#define OSAL_MQ_ATTR__OFLAG__RDONLY           0x00000001u
#define OSAL_MQ_ATTR__OFLAG__WRONLY           0x00000002u
#define OSAL_MQ_ATTR__OFLAG__RDWR             0x00000004u
#define OSAL_MQ_ATTR__OFLAG__CREAT            0x00000008u
#define OSAL_MQ_ATTR__OFLAG__CLOEXEC          0x00000010u
#define OSAL_MQ_ATTR__OFLAG__EXCL             0x00000020u

typedef struct osal_mq_attr {
    osal_uint32_t   oflags;
    osal_mode_t     mode;
    osal_size_t     max_messages;
    osal_size_t     max_message_size;
} osal_mq_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a mq.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mq attributes. Can be NULL then
 *                      the defaults of the underlying mq will be used.
 *
 * \return OK or ERROR_CODE.
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
 * \param[in]   msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to receive.
 * \param[in]   prio    Receive priority.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_receive(osal_mq_t *mq, const osal_char_t *msg, const osal_size_t msg_len, const osal_uint32_t prio);

//! \brief Receive a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to receive.
 * \param[in]   prio    Receive priority.
 * \param[in]   to      Timeout waiting if message queue is full.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_timedreceive(osal_mq_t *mq, const osal_char_t *msg, const osal_size_t msg_len, 
        const osal_uint32_t prio, const osal_timer_t *to);

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

#endif /* LIBOSAL_MQ__H */


