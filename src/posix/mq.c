/**
 * \file posix/mq.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mq posix source.
 *
 * OSAL mq posix source.
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

#include <libosal/mq.h>
#include <libosal/osal.h>
#include <libosal/config.h>

#include <assert.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <errno.h>


//! \brief Initialize a mq.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mq attributes. Can be NULL then
 *                      the defaults of the underlying mq will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_open(osal_mq_t *mq, const osal_char_t *name,  const osal_mq_attr_t *attr) {
    assert(mq != NULL);
    assert(name != NULL);

    osal_retval_t ret = OSAL_OK;
    
    int oflags = 0;
    int mode = 0;
    struct mq_attr local_attr;

    if (attr != 0) {
        if (attr->oflags & OSAL_MQ_ATTR__OFLAG__RDONLY) {
            oflags |= O_RDONLY;
        }
        if (attr->oflags & OSAL_MQ_ATTR__OFLAG__WRONLY) {
            oflags |= O_WRONLY;
        } 
        if (attr->oflags & OSAL_MQ_ATTR__OFLAG__RDWR) {
            oflags |= O_RDWR;
        }
        if (attr->oflags & OSAL_MQ_ATTR__OFLAG__CREAT) {
            oflags |= O_CREAT;
        }

        mode = attr->mode;

        local_attr.mq_maxmsg = attr->max_messages;    
        local_attr.mq_msgsize = attr->max_message_size;
    }

    mq->mq_desc = mq_open(name, oflags, mode, &local_attr);
	if (mq->mq_desc == (mqd_t)-1) {
        switch (errno) {
            case EACCES:        // The queue exists, but the caller does not have permission to open it in the specified mode.
                                // name contained more than one slash.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case EEXIST:        // Both O_CREAT and O_EXCL were specified in oflag, but a queue with this name already exists.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case EINVAL:        // name doesn't follow the format in mq_overview(7).
                                // O_CREAT was specified in oflag, and attr was not NULL, but attr->mq_maxmsg or attr->mq_msqsize 
                                // was invalid.  Both of these fields must be greater than zero.  In  a  process  that  is  unprivileged
                                // (does  not have the CAP_SYS_RESOURCE capability), attr->mq_maxmsg must be less than or equal to 
                                // the msg_max limit, and attr->mq_msgsize must be less than or equal to the msgsize_max limit.  In ad-
                                // dition, even in a privileged process, attr->mq_maxmsg cannot exceed the HARD_MAX limit. 
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EMFILE:        // The per-process limit on the number of open file and message queue descriptors has been reached (see
                                // the description of RLIMIT_NOFILE in getrlimit(2)).
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case ENAMETOOLONG:  // name was too long.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case ENFILE:        // The system-wide limit on the total number of open files and message queues has been reached.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case ENOENT:        // The O_CREAT flag was not specified in oflag, and no queue with this name exists.
                                // name was just "/" followed by no other characters.
                ret = OSAL_ERR_NOT_FOUND;
                break;
            case ENOMEM:        // Insufficient memory.
                ret = OSAL_ERR_OUT_OF_MEMORY;
                break;
            case ENOSPC:        // Insufficient space for the creation of a new message queue.  This probably occurred because the 
                                // queues_max limit was encountered; see mq_overview(7).
                ret = OSAL_ERR_OUT_OF_MEMORY;
                break;
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
        }
    }

    return ret;
}

//! \brief Send a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[in]   msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to send.
 * \param[in]   prio    Send priority.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_send(osal_mq_t *mq, const osal_char_t *msg, const osal_size_t msg_len, const osal_uint32_t prio) {
    assert(mq != NULL);
    assert(msg != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret = mq_send(mq->mq_desc, msg, msg_len, prio);
    if (local_ret == -1) {
        switch (errno) {
            case EAGAIN:    // The queue was full, and the O_NONBLOCK flag was set for the message queue description 
                            // referred to by mqdes.
                ret = OSAL_ERR_BUSY;
                break;
            case EBADF:     // The descriptor specified in mqdes was invalid or not opened for writing.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EINTR:     // The call was interrupted by a signal handler; see signal(7).
                ret = OSAL_ERR_INTERRUPTED;
                break;
            case EINVAL:    // The call would have blocked, and abs_timeout was invalid, either because tv_sec 
                            // was less than zero, or because tv_nsec was less than zero or greater than 1000 million.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EMSGSIZE:  // msg_len was greater than the mq_msgsize attribute of the message queue.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
        }
    }

    return ret;
}


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
        const osal_uint32_t prio, const osal_timer_t *to) {
    assert(mq != NULL);
    assert(msg != NULL);
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;

    struct timespec ts;
    ts.tv_sec = to->sec;
    ts.tv_nsec = to->nsec;

    while (ret != OSAL_ERR_TIMEOUT) {
        int local_ret = mq_timedsend(mq->mq_desc, msg, msg_len, prio, &ts);
        if (local_ret == -1) {
            switch (errno) {
                case EAGAIN:    // The queue was full, and the O_NONBLOCK flag was set for the message queue description 
                                // referred to by mqdes.
                    ret = OSAL_ERR_BUSY;
                    break;
                case EBADF:     // The descriptor specified in mqdes was invalid or not opened for writing.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case EINTR:     // The call was interrupted by a signal handler; see signal(7).
                    ret = OSAL_ERR_INTERRUPTED;
                    break;
                case EINVAL:    // The call would have blocked, and abs_timeout was invalid, either because tv_sec 
                                // was less than zero, or because tv_nsec was less than zero or greater than 1000 million.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case EMSGSIZE:  // msg_len was greater than the mq_msgsize attribute of the message queue.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case ETIMEDOUT: // The call timed out before a message could be transferred.
                    ret = OSAL_ERR_TIMEOUT;
                    break;
                default:
                    ret = OSAL_ERR_OPERATION_FAILED;
                    break;
            }
        } else {
            ret = OSAL_OK;
            break;
        }
    }

    return ret;
}


//! \brief Receive a message through message queue.
/*!
 * \param[in]   mq      Pointer to osal mq structure. Content is OS dependent.
 * \param[out]  msg     Pointer to message buffer.
 * \param[in]   msg_len Lenght of message to receive.
 * \param[out]  prio    Receive priority.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_receive(osal_mq_t *mq, osal_char_t *msg, const osal_size_t msg_len, osal_uint32_t *prio) {
    assert(mq != NULL);
    assert(msg != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret = mq_receive(mq->mq_desc, msg, msg_len, prio);
    if (local_ret == -1) {
        switch (errno) {
            case EAGAIN:    // The queue was full, and the O_NONBLOCK flag was set for the message queue description 
                            // referred to by mqdes.
                ret = OSAL_ERR_BUSY;
                break;
            case EBADF:     // The descriptor specified in mqdes was invalid or not opened for writing.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EINTR:     // The call was interrupted by a signal handler; see signal(7).
                ret = OSAL_ERR_INTERRUPTED;
                break;
            case EINVAL:    // The call would have blocked, and abs_timeout was invalid, either because tv_sec 
                            // was less than zero, or because tv_nsec was less than zero or greater than 1000 million.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EMSGSIZE:  // msg_len was greater than the mq_msgsize attribute of the message queue.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
        }
    }
    return ret;
}


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
        osal_uint32_t *prio, const osal_timer_t *to) {
    assert(mq != NULL);
    assert(msg != NULL);
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;

    struct timespec ts;
    ts.tv_sec = to->sec;
    ts.tv_nsec = to->nsec;

    while (ret != OSAL_ERR_TIMEOUT) {
        int local_ret = mq_timedreceive(mq->mq_desc, msg, msg_len, prio, &ts);
        if (local_ret == -1) {
            switch (errno) {
                case EAGAIN:    // The queue was full, and the O_NONBLOCK flag was set for the message queue description 
                                // referred to by mqdes.
                    ret = OSAL_ERR_BUSY;
                    break;
                case EBADF:     // The descriptor specified in mqdes was invalid or not opened for writing.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case EINTR:     // The call was interrupted by a signal handler; see signal(7).
                    ret = OSAL_ERR_INTERRUPTED;
                    break;
                case EINVAL:    // The call would have blocked, and abs_timeout was invalid, either because tv_sec 
                                // was less than zero, or because tv_nsec was less than zero or greater than 1000 million.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case EMSGSIZE:  // msg_len was greater than the mq_msgsize attribute of the message queue.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case ETIMEDOUT: // The call timed out before a message could be transferred.
                    ret = OSAL_ERR_TIMEOUT;
                    break;
                default:
                    ret = OSAL_ERR_OPERATION_FAILED;
                    break;
            }
        } else {
            ret = OSAL_OK;
            break;
        }
    }
    return ret;
}


//! \brief Closes an open mq.
/*!
 * \param[in]   mq     Pointer to osal mq structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mq_close(osal_mq_t *mq) {
    assert(mq != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret = mq_close(mq->mq_desc);
    if (local_ret == -1) {
        // only EBADF could be set
        ret = OSAL_ERR_INVALID_PARAM;
    }

    return ret;
}

