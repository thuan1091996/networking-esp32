idf_component_register(SRCS "station_example_main.c" "./http_client/http_client.c"
                    INCLUDE_DIRS "." "./http_client/"
                    EMBED_TXTFILES ../main/certs/worldtime.pem)

