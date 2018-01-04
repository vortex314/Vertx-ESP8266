#include <Telnet.h>


Telnet::Telnet(const char *name)
    : VerticleTask(name, 312, 5)
{
}

void Telnet::run()
{
    while (true) {
        struct netconn *conn, *newconn;
        err_t err;
//            LWIP_UNUSED_ARG(arg);

        /* Create a new connection identifier. */
        /* Bind connection to well known port number 7. */
#if LWIP_IPV6
        conn = netconn_new(NETCONN_TCP_IPV6);
        netconn_bind(conn, IP6_ADDR_ANY, 23);
#else  /* LWIP_IPV6 */
        conn = netconn_new(NETCONN_TCP);
        netconn_bind(conn, IP_ADDR_ANY, 23);
#endif /* LWIP_IPV6 */
        LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

        /* Tell connection to go into listening mode. */
        netconn_listen(conn);

        while (1) {

            /* Grab new connection. */
            err = netconn_accept(conn, &newconn);
            /*INFO("accepted new connection %p\n", newconn);*/
            /* Process the new connection. */
            if (err == ERR_OK) {
                INFO(" connection accepted.");
                struct netbuf *buf;
                void *data;
                u16_t len;

                netconn_write(newconn, " $ >", 4, NETCONN_COPY);

                while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
                    /*INFO("Recved\n");*/
                    do {
                        netbuf_data(buf, &data, &len);
                        err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 1
                        if (err != ERR_OK) {
                            INFO("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
                        }
#endif
                    } while (netbuf_next(buf) >= 0);
                    netbuf_delete(buf);
                }
                INFO("Got EOF, looping");
                /* Close connection and discard connection identifier. */
                netconn_close(newconn);
                netconn_delete(newconn);
            }
        }
    }
}
