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


extern tpoll_t tp_global;		/* defined in server.c */
static int ipmi_engine_started = 0;

static int connect_ipmi_obj(obj_t *ipmi);
static void disconnect_ipmi_obj(obj_t *ipmi);


void ipmi_init(int num_consoles)
{
/*  Starts the ipmiconsole engine to handle [num_consoles] IPMI SOL consoles.
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
    if (!ipmi_engine_started) {
        return;
    }
    ipmiconsole_engine_teardown();
    ipmi_engine_started = 0;
    return;
}


static int parse_kg(unsigned char *outbuf, int outsz, const char *instr)
{
/*  A k_g key is interpreted as ascii text unless it is prefixed with "0x",
 *    in which case is it interpreted as hexadecimal.
 */
    char *p, *q;
    int i, j;
    char buf[3] = {0, 0, 0};

    assert(outbuf != NULL);
    assert(instr != NULL);
    assert(outsz == IPMI_K_G_MAX);

    if (strlen(instr) == 0)
        return(0);

    if (strncmp(instr, "0x", 2) == 0) {
        if (strlen(instr) > IPMI_K_G_MAX*2+2)
            return(-1);
        p = (char *)instr + 2;
        memset(outbuf, 0, IPMI_K_G_MAX);
        for (i = j = 0; i < strlen(p); i+=2, j++) {
            if (p[i+1] == '\0')
                return(-1);
            buf[0] = p[i]; buf[1] = p[i+1]; buf[2] = 0;
            errno = 0;
            outbuf[j] = strtoul(buf, &q, 16);
            if (errno || (q != buf + 2))
                return(-1);
        }
    }
    else {
        if (strlen(instr) > IPMI_K_G_MAX)
            return(-1);
        memset(outbuf, 0, IPMI_K_G_MAX);
        memcpy(outbuf, instr, strlen(instr));
    }

    return(1);
}


int parse_ipmi_opts(
    ipmiopt_t *iopts, const char *str, char *errbuf, int errlen)
{
/*  Parses 'str' for ipmi device options 'iopts'.
 *    The 'iopts' struct should be initialized to a default value.
 *    The 'str' string is of the form "<username>,<password>[,<K_g key>]".
 *  Returns 0 and updates the 'iopts' struct on sucess; o/w, returns -1
 *    (writing an error message into 'errbuf' if defined).
 */
    ipmiopt_t ioptsTmp;
    char buf[MAX_LINE];
    const char * const separators = ",";
    char *tok;

    assert(iopts != NULL);

    memset(&ioptsTmp, 0, sizeof(ipmiopt_t));

    if ((str == NULL) || str[0] == '\0') {
        if ((errbuf != NULL) && (errlen > 0))
            snprintf(errbuf, errlen, "encountered empty options string");
        return(-1);
    }

    if (strlcpy(buf, str, sizeof(buf)) >= sizeof(buf)) {
        if ((errbuf != NULL) && (errlen > 0))
            snprintf(errbuf, errlen, "ipmiopt string exceeded buffer size");
        return(-1);
    }

    tok = strtok(buf, separators);
    if (tok == NULL) {
        if ((errbuf != NULL) && (errlen > 0))
            snprintf(errbuf, errlen, "ipmiopt username not specified");
        return(-1);
    }
    if (strlen(tok) > IPMI_USERNAME_MAX) {
        if ((errbuf != NULL) && (errlen > 0))
            snprintf(errbuf, errlen, "ipmiopt username exceeds max length");
        return(-1);
    }

    ioptsTmp.username = strdup(tok);

    tok = strtok(NULL, separators);
    if (tok == NULL) {
        if ((errbuf != NULL) && (errlen > 0))
            snprintf(errbuf, errlen, "ipmiopt password not specified");
        goto cleanup;
    }
    if (strlen(tok) > IPMI_PASSWORD_MAX) {
        if ((errbuf != NULL) && (errlen > 0))
            snprintf(errbuf, errlen, "ipmiopt password exceeds max length");
        goto cleanup;
    }

    ioptsTmp.password = strdup(tok);

    tok = strtok(NULL, separators);
    if (tok == NULL) {
        memcpy(ioptsTmp.k_g, iopts->k_g, IPMI_K_G_MAX);
    }
    else {
        if (parse_kg(ioptsTmp.k_g, IPMI_K_G_MAX, tok) < 0) {
            if ((errbuf != NULL && (errlen > 0)))
                snprintf(errbuf, errlen, "ipmiopt k_g is invalid");
            goto cleanup;
        }
    }

    *iopts = ioptsTmp;
    return(0);

 cleanup:
    if (ioptsTmp.username)
        free(ioptsTmp.username);
    if (ioptsTmp.password)
        free(ioptsTmp.password);
    return(-1);
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
    conf->numIpmiObjs++;
    /*
     *  Add obj to the master conf->objs list.
     */
    list_append(conf->objs, ipmi);

    return(ipmi);
}


int new_ipmi_ctx(obj_t *ipmi)
{
/*  Returns 0 if the IPMI ctx is sucessfully created; o/w, returns -1.
 */
    struct ipmiconsole_ipmi_config ipmi_config;
    struct ipmiconsole_protocol_config protocol_config;

    assert(ipmi != NULL);
    assert(is_ipmi_obj(ipmi));

    /*  Create the ctx object if it doesn't already exist.
     */
    if (ipmi->aux.ipmi.ctx == NULL) {
        /*
         *  Setup configuration structs for the ctx creation.
         */
        ipmi_config.username = ipmi->aux.ipmi.iconf.username;
        ipmi_config.password = ipmi->aux.ipmi.iconf.password;
        ipmi_config.k_g = malloc(IPMI_K_G_MAX);
        memcpy(ipmi_config.k_g, ipmi->aux.ipmi.iconf.k_g, IPMI_K_G_MAX);

        ipmi_config.k_g_len = IPMI_K_G_MAX;
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
        if (!ipmi->aux.ipmi.ctx)
            return(-1);

        ipmi->aux.ipmi.state = CONMAN_IPMI_DOWN;
    }
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
        rv = new_ipmi_ctx(ipmi);
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
        rv = new_ipmi_ctx(ipmi);
        if (rv < 0)
            log_err(rv, "new_ipmi_ctx failed");
        ipmi->aux.ipmi.state = CONMAN_IPMI_DOWN;
        ipmi->aux.ipmi.timer = tpoll_timeout_relative(tp_global,
            (callback_f) connect_ipmi_obj, ipmi,
            IPMI_CONNECT_RETRY_TIMEOUT * 1000);
    }
}
