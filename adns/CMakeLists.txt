SET(LIBADNS_SRC check.c event.c general.c parse.c poll.c query.c reply.c setup.c transmit.c types.c)
ADD_LIBRARY(adns STATIC ${LIBADNS_SRC})
