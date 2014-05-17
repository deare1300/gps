DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin


SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o, $(notdir ${SRC}))

TARGET = terminalclient
CRT_DIR = ${shell pwd}
BIN_TARGET = ${DIR_BIN}/${TARGET}


CC = gcc
CFLAGS = -g -Wall -I ${CRT_DIR} -lpthread

${BIN_TARGET}:${OBJ}
	@echo ${BIN_TARGET} -LINK ${OBJ} -FLAG ${CFLAGS}......
	$(CC) -o $@ $^ ${CFLAGS}

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo compiled object.o $@ ...

DESC = desc

${DESC}:
	@echo "----"${SRC} ${OBJ}....
	@echo ${BIN_TARGET}
all: ${DESC} ${BIN_TARGET} 


.PHONY:clean
clean:
	find ${DIR_OBJ} -name *.o -exec rm -rf '{}' \;
