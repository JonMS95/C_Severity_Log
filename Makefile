src_svr_log = Source_files/SeverityLog.c

so_svr_log	= Dynamic_libraries/libSeverityLog.so

shell_dirs	= Shell_files/directories.sh

all: directories clean log.so msg

directories:
	@./$(shell_dirs)

log.so: $(src_svr_log)
	gcc -Wall -g -fPIC -shared $(src_svr_log) -o $(so_svr_log)

clean:
	rm -rf Dynamic_libraries/*

msg:
	@echo "**************************************************************************************";
	