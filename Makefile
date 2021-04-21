CC := g++
CFLAGS := -g -Wall -lpthread
TARGET := mutex rwlock spinlock sync mutex_proc


########################  all   ##########################
.PHONY: all
all: $(TARGET)


######################## thread ###########################
mutex: mutex.cpp helper.h
	$(CC) $(CFLAGS) -o $@ $<

rwlock: rwlock.cpp helper.h
	$(CC) $(CFLAGS) -o $@ $<

spinlock: spinlock.cpp helper.h
	$(CC) $(CFLAGS) -o $@ $<
	
sync: sync.cpp helper.h
	$(CC) $(CFLAGS) -o $@ $<
	
######################## process ###########################
mutex_proc: mutex_proc.cpp helper.h
	$(CC) $(CFLAGS) -o $@ $<
	
	
######################## clean ############################	
clean:
	rm -rf mutex rwlock spinlock sync mutex_proc
