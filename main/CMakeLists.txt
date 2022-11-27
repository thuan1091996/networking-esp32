idf_component_register(SRCS "networking-app.c" "./http_client/http_client.c" "./socket_client/udp_client_socket.c" "./socket_client/tcp_client_socket.c"
                    INCLUDE_DIRS "." "./task_common/" "./http_client/" "./socket_client"
                    EMBED_TXTFILES ../main/certs/worldtime.pem)

