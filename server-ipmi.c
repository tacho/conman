/*****************************************************************************
 *  $Id$
 *****************************************************************************
 *  Written by Levi Pearson <lpearson@lnxi.com>.
 *
 *  This file is part of ConMan: The Console Manager.
 *  For details, see <http://home.gna.org/conman/>.
 *
 *  This is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *****************************************************************************/


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <ipmiconsole.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "list.h"
#include "log.h"
#include "server.h"
#include "tpoll.h"
#include "util-str.h"


extern tpoll_t tp_global;               /* defined in server.c */
static int ipmi_engine_started = 0;

static int parse_key(unsigned char *dst, const char *src, size_t dstlen);
static int create_ipmi_ctx(obj_t *ipmi);
static int connect_ipmi_obj(obj_t *ipmi);
static void disconnect_ipmi_obj(obj_t *ipmi);


void ipmi_init(int num_consoles)
{
/*  Starts the ipmiconsole engine to handle 'num_consoles' IPMI SOL consoles.
 */
    int num_threads;

    if (num_consoles <= 0) {
        return;
    }
    if (ipmi_engine_started) {
        return;
    }
    num_threads = ((num_consoles - 1) / IPMI_ENGINE_CONSOLES_PER_THREAD) + 1;

    if (ipmiconsole_engine_init(num_threads, 0) < 0) {
        log_err(0, "Unable to start IPMI engine");
    }
    else {
        log_msg(LOG_INFO,
            "IPMI engine started with %d thread%s for %d console%s",
            num_threads, (num_threads == 1) ? "" : "s",
            num_consoles, (num_consoles == 1) ? "" : "s");
    }
    ipmi_engine_started = 1;
    return;
}


void ipmi_fini(void)
{
/*  Stops the ipmiconsole engine.
 */
    if (!ipmi_engine_started) {
        return;
    }
    ipmiconsole_engine_teardown();
    ipmi_engine_started = 0;
    return;
}


static int parse_key(unsigned char *dst, const char *src, size_t dstlen)
{
/*  Parses the NUL-terminated key string 'src', writing the result into
 *    buffer 'dst' of length 'dstlen'.  The 'dst' buffer will always be
 *    NUL-terminated.
 *  The 'src' is interpreted as ASCII text unless it is prefixed with
 *    "0x" or "0X" and contains only hexadecimal digits (ie, [0-9A-Fa-f]).
 *    A hexadecimal string will be converted to binary and may contain
 *    embedded NUL characters.
 *  Returns the length of the key (in bytes) written to 'dst'
 *    (not including the final NUL-termination character),
 *    or -1 if truncation occurred.
 */
    const char *hexdigits = "0123456789ABCDEFabcdef";
    char       *dstend;
    char       *p;
    char       *q;
    int         n;

    assert(dst != NULL);
    assert(src != NULL);

    if ((src[0] == '0') && (src[1] == 'x' || src[1] == 'X')
            && (strspn(src + 2, hexdigits) == strlen(src + 2))) {
        dstend = dst + dstlen - 1;      /* reserve space for terminating NUL */
        p = (char *) src + 2;
        q = dst;
        n = 0;
        while (*p && (q < dstend)) {
            if (((p - src) & 0x01) == 0) {
                *q = (toint(*p++) << 4) & 0xf0;
                n++;
            }
            else {
                *q++ |= (toint(*p++)) & 0x0f;
            }
        }
        dst[n] = '\0';
        if (*p) {
            return(-1);
        }
    }
    else {
        if (strlcpy(dst, src, dstlen) >= dstlen) {
            return(-1);
        }
        n = strlen(dst);
    }
    assert(n < dstlen);
    return(n);
}


int parse_ipmi_opts(
    ipmiopt_t *iopts, const char *str, char *errbuf, int errlen)
{
/*  Parses 'str' for ipmi device options 'iopts'.
 *    The 'iopts' struct should be initialized to a default value.
 *    The 'str' string is of the form "[<username>[,<password>[,<K_g key>]]]".
 *  An empty 'str' is valid and denotes specifying no username & password.
 *  Returns 0 and updates the 'iopts' struct on sucess; o/w, returns -1
 *    (writing an error message into 'errbuf' if defined).
 */
    ipmiopt_t ioptsTmp;
    char buf[MAX_LINE];
    const char * const separators = ",";
    char *tok;
    int n;

    assert(iopts != NULL);

    memset(&ioptsTmp, 0, sizeof(ioptsTmp));

    if (strlcpy(buf, str, sizeof(buf)) >= sizeof(buf)) {
        if ((errbuf != NULL) && (errlen > 0)) {
            snprintf(errbuf, errlen,
                "ipmiopt string exceeded buffer size");
        }
        return(-1);
    }
    if ((tok = strtok(buf, separators))) {
        n = strlcpy(ioptsTmp.username, tok, sizeof(ioptsTmp.username));
        if (n >= sizeof(ioptsTmp.username)) {
            if ((errbuf != NULL) && (errlen > 0)) {
                snprintf(errbuf, errlen,
                    "ipmiopt username exceeds %d-byte max length",
                    IPMI_MAX_USER_LEN);
            }
            return(-1);
        }
    }
    if ((tok = strtok(NULL, separators))) {
        n = parse_key(ioptsTmp.password, tok, sizeof(ioptsTmp.password));
        if (n < 0) {
            if ((errbuf != NULL) && (errlen > 0)) {
                snprintf(errbuf, errlen,
                    "ipmiopt password exceeds %d-byte max length",
                    IPMI_MAX_PSWD_LEN);
            }
            return(-1);
        }
        ioptsTmp.passwordLen = n;
    }
    if ((tok = strtok(NULL, separators))) {
        n = parse_key(ioptsTmp.kg, tok, sizeof(ioptsTmp.kg));
        if (n < 0) {
            if ((errbuf != NULL) && (errlen > 0)) {
                snprintf(errbuf, errlen,
                    "ipmiopt k_g exceeds %d-byte max length",
                    IPMI_MAX_KG_LEN);
            }
            return(-1);
        }
        ioptsTmp.kgLen = n;
    }
    *iopts = ioptsTmp;
    return(0);
}


obj_t * create_ipmi_obj(server_conf_t *conf, char *name,
    ipmiopt_t *iconf, char *hostname, char *errbuf, int errlen)
{
/*  Creates a new ipmi device object and adds it to the master objs list.
 *  Returns the new object, or NULL on error.
 */
    ListIterator i;
    obj_t *ipmi;

    assert(conf != NULL);
    assert((name != NULL) && (name[0] != '\0'));
    assert(iconf != NULL);

    /*  Check for duplicate console names.
     */
    i = list_iterator_create(conf->objs);
    while ((ipmi = list_next(i))) {
        if (is_console_obj(ipmi) && !strcmp(ipmi->name, name)) {
            snprintf(errbuf, errlen,
                "console [%s] specifies duplicate console name", name);
            break;
        }
        if (is_ipmi_obj(ipmi) && !strcmp(ipmi->aux.ipmi.hostname, hostname)) {
            snprintf(errbuf, errlen,
                "console [%s] specifies duplicate hostname \"%s\"",
                name, hostname);
            break;
        }
    }
    list_iterator_destroy(i);
    if (ipmi != NULL) {
        return(NULL);
    }
    ipmi = create_obj(conf, name, -1, CONMAN_OBJ_IPMI);
    ipmi->aux.ipmi.hostname = strdup(hostname);
    ipmi->aux.ipmi.iconf = *iconf;
    ipmi->aux.ipmi.ctx = NULL;
    ipmi->aux.ipmi.logfile = NULL;
    ipmi->aux.ipmi.state = CONMAN_IPMI_DOWN;
    ipmi->aux.ipmi.timer = -1;
    conf->numIpmiObjs++;
    /*
     *  Add obj to the master conf->objs list.
     */
    list_append(conf->objs, ipmi);

    return(ipmi);
}


static int create_ipmi_ctx(obj_t *ipmi)
{
/*  Returns 0 if the IPMI ctx is sucessfully created; o/w, returns -1.
 */
    struct ipmiconsole_ipmi_config ipmi_config;
    struct ipmiconsole_protocol_config protocol_config;
    int n;

    assert(ipmi != NULL);
    assert(is_ipmi_obj(ipmi));

    /*  Only create a ctx object if one doesn't already exist.
     */
    if (ipmi->aux.ipmi.ctx) {
        return(0);
    }
    /*  Setup configuration structs for the ctx creation.
     */
    ipmi_config.username = strdup(ipmi->aux.ipmi.iconf.username);
    ipmi_config.password = strdup(ipmi->aux.ipmi.iconf.password);
    n = ipmi->aux.ipmi.iconf.kgLen;
    ipmi_config.k_g = malloc(n);
    memcpy(ipmi_config.k_g, ipmi->aux.ipmi.iconf.kg, n);
    ipmi_config.k_g_len = n;
    ipmi_config.privilege_level = -1;
    ipmi_config.cipher_suite_id = -1;

    protocol_config.session_timeout_len = -1;
    protocol_config.retransmission_timeout_len = -1;
    protocol_config.retransmission_backoff_count = -1;
    protocol_config.keepalive_timeout_len = -1;
    protocol_config.retransmission_keepalive_timeout_len = -1;
    protocol_config.acceptable_packet_errors_count = -1;
    protocol_config.maximum_retransmission_count = -1;
    protocol_config.debug_flags = 0;
    protocol_config.security_flags = 0;
    protocol_config.workaround_flags = 0;

    ipmi->aux.ipmi.ctx = ipmiconsole_ctx_create(ipmi->aux.ipmi.hostname,
        &ipmi_config, &protocol_config);
    if (!ipmi->aux.ipmi.ctx) {
        return(-1);
    }
    ipmi->aux.ipmi.state = CONMAN_IPMI_DOWN;
    return(0);
}


int open_ipmi_obj(obj_t *ipmi)
{
/*  (Re)opens the specified 'ipmi' obj.
 *  Returns 0 if the IPMI console is sucessfully opened; o/w, returns -1.
 */
    int rv = 0;
    assert(is_ipmi_obj(ipmi));

    if (ipmi->aux.ipmi.state == CONMAN_IPMI_UP) {
        disconnect_ipmi_obj(ipmi);
    }
    else {
        rv = create_ipmi_ctx(ipmi);
        if (rv < 0)
            return(rv);
        rv = connect_ipmi_obj(ipmi);
    }
    DPRINTF((9, "Opened [%s] ipmi: host=%s state=%d.\n",
        ipmi->name, ipmi->aux.ipmi.hostname, ipmi->fd,
        (int) ipmi->aux.ipmi.state));
    return(rv);
}


int send_ipmi_break(obj_t *ipmi)
{
    assert(ipmi != NULL);
    assert(is_ipmi_obj(ipmi));
    assert(ipmi->aux.ipmi.ctx != NULL);

    return(ipmiconsole_ctx_generate_break(ipmi->aux.ipmi.ctx));
}


static int connect_ipmi_obj(obj_t *ipmi)
{
    int rv = 0;

    assert(ipmi != NULL);
    assert(is_ipmi_obj(ipmi));
    assert(ipmi->aux.ipmi.state != CONMAN_IPMI_UP);
    assert(ipmi->aux.ipmi.ctx != NULL);

    if (ipmi->aux.ipmi.timer >= 0) {
        (void) tpoll_timeout_cancel(tp_global, ipmi->aux.ipmi.timer);
        ipmi->aux.ipmi.timer = -1;
    }
    if (ipmi->aux.ipmi.state == CONMAN_IPMI_DOWN) {
        /*
         *  Do a non-blocking submit of this ctx to the engine.
         */
        DPRINTF((10, "Connecting to <%s> for [%s].\n",
            ipmi->aux.ipmi.hostname, ipmi->name));
        rv = ipmiconsole_engine_submit(ipmi->aux.ipmi.ctx);
        if (rv < 0) {
            log_msg(LOG_WARNING,
                "Unable to submit ipmi ctx to engine for [%s]: %s", ipmi->name,
                ipmiconsole_ctx_strerror(ipmiconsole_ctx_errnum(
                    ipmi->aux.ipmi.ctx)));
            return(-1);
        }
        if ((ipmi->fd = ipmiconsole_ctx_fd(ipmi->aux.ipmi.ctx)) < 0) {
            log_msg(LOG_WARNING,
                "Unable to get a file descriptor for [%s]", ipmi->name);
            return(-1);
        }
        ipmi->aux.ipmi.state = CONMAN_IPMI_PENDING;
        ipmi->aux.ipmi.timer = tpoll_timeout_relative(tp_global,
            (callback_f) connect_ipmi_obj,
            ipmi,
            IPMI_STATUS_CHECK_TIMEOUT * 1000);
        return(-1);
    }
    else if (ipmi->aux.ipmi.state == CONMAN_IPMI_PENDING) {
        /*
         *  Check with the engine to see if the connection was successful.
         */
        rv = ipmiconsole_ctx_status(ipmi->aux.ipmi.ctx);
        if (rv < 0) {
            /* something wonky happened and we couldn't get the conn status */
            log_msg(LOG_WARNING,
                "Error retrieving ipmi ctx status from engine for [%s]",
                ipmi->name);
            disconnect_ipmi_obj(ipmi);
            return(-1);
        }

        if (rv == IPMICONSOLE_CONTEXT_STATUS_ERROR) {
            /* an error occurred with the ipmi connection */
            rv = ipmiconsole_ctx_errnum(ipmi->aux.ipmi.ctx);
            log_msg(LOG_WARNING, "Error establishing ipmi link for [%s]: %s",
                ipmi->name, ipmiconsole_ctx_strerror(rv));
            ipmi->fd = -1;
            disconnect_ipmi_obj(ipmi);
            return(-1);
        }

        if (rv == IPMICONSOLE_CONTEXT_STATUS_NONE) {
            /* wait a bit longer */
            ipmi->aux.ipmi.timer = tpoll_timeout_relative(tp_global,
                (callback_f) connect_ipmi_obj, ipmi,
                IPMI_STATUS_CHECK_TIMEOUT * 1000);
            return(-1);
        }

        if (rv == IPMICONSOLE_CONTEXT_STATUS_SOL_ESTABLISHED) {
            /* success! */
            write_notify_msg(ipmi, LOG_INFO,
                "Console [%s] connected to <%s> via IPMI",
                ipmi->name, ipmi->aux.ipmi.hostname);
            ipmi->aux.ipmi.state = CONMAN_IPMI_UP;
            return(0);
        }
        else {
            log_msg(LOG_WARNING,
                "Unrecognize ipmi ctx status value %d for [%s]",
                rv, ipmi->name);
            disconnect_ipmi_obj(ipmi);
            return(-1);
        }
    }
    return(rv);
}


static void disconnect_ipmi_obj(obj_t *ipmi)
{
    assert(ipmi != NULL);
    assert(is_ipmi_obj(ipmi));
    int rv;

    if (ipmi->aux.ipmi.timer >= 0) {
        (void) tpoll_timeout_cancel(tp_global, ipmi->aux.ipmi.timer);
        ipmi->aux.ipmi.timer = -1;
    }
    if (ipmi->fd >= 0) {
        close(ipmi->fd);
        ipmi->fd = -1;
    }
    if (ipmiconsole_ctx_destroy(ipmi->aux.ipmi.ctx) < 0) {
        ipmi->aux.ipmi.timer = tpoll_timeout_relative(tp_global,
            (callback_f) disconnect_ipmi_obj, ipmi,
            IPMI_STATUS_CHECK_TIMEOUT * 1000);
    }
    else {
        ipmi->aux.ipmi.ctx = NULL;
        if (ipmi->aux.ipmi.state == CONMAN_IPMI_UP) {
            write_notify_msg(ipmi, LOG_NOTICE,
                "Console [%s] disconnected from <%s> via IPMI",
                ipmi->name, ipmi->aux.ipmi.hostname);
        }
        rv = create_ipmi_ctx(ipmi);
        if (rv < 0)
            log_err(rv, "create_ipmi_ctx failed");
        ipmi->aux.ipmi.state = CONMAN_IPMI_DOWN;
        ipmi->aux.ipmi.timer = tpoll_timeout_relative(tp_global,
            (callback_f) connect_ipmi_obj, ipmi,
            IPMI_CONNECT_RETRY_TIMEOUT * 1000);
    }
}
