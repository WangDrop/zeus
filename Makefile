SOURCE_CORE_DIR=src/core
SOURCE_EVENT_DIR=src/event
SOURCE_LOGIC_DIR=src/logic

VPATH=$(SOURCE_CORE_DIR):$(SOURCE_EVENT_DIR):$(SOURCE_LOGIC_DIR)

STRING_DEPENDENCY=zeus_common.h zeus_string.h zeus_string.c
STRING_COMMAND=zeus_string.c

LOG_DEPENDENCY=zeus_common.h zeus_log.h zeus_log.c
LOG_COMMAND=zeus_log.c

TIME_DEPENDENCY=zeus_common.h zeus_times.h zeus_times.c
TIME_COMMAND=zeus_times.c

ALLOC_DEPENDENCY=zeus_common.h zeus_alloc.h zeus_alloc.c
ALLOC_COMMAND=zeus_alloc.c

FILE_DEPENDENCY=zeus_common.h zeus_file.h zeus_file.c
FILE_COMMAND=zeus_file.c

PROCESS_DEPENDENCY=zeus_common.h zeus_process.h zeus_process.c
PROCESS_COMMAND=zeus_process.c

HASH_DEPENDENCY=zeus_common.h zeus_hash.h zeus_hash.c
HASH_COMMAND=zeus_hash.c

ZEUS_DEPENDENCY=zeus_common.h zeus.h zeus.c
ZEUS_COMMAND=zeus.c

ALL_OBJ=zeus_string.o zeus_log.o zeus_times.o zeus_alloc.o zeus_file.o zeus_process.o zeus_hash.o zeus.o

zeus:$(ALL_OBJ)
	cc $(ALL_OBJ) -o zeus

zeus_string.o:$(STRING_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(STRING_COMMAND)

zeus_log.o:$(LOG_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(LOG_COMMAND)

zeus_times.o:$(TIME_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(TIME_COMMAND)

zeus_alloc.o:$(ALLOC_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(ALLOC_COMMAND)

zeus_file.o:$(FILE_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(FILE_COMMAND)

zeus_process.o:$(PROCESS_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(PROCESS_COMMAND)

zeus_hash.o:$(HASH_DEPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(HASH_COMMAND)

zeus.o:$(ZEUS_DENPENDENCY)
	cc -c $(SOURCE_CORE_DIR)/$(ZEUS_COMMAND)

.PHONY:clean
clean:
	rm -rf *.o zeus
