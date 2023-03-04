src_svr_log = Source_files/SeverityLog.c

so_svr_log	= Dynamic_libraries/libSeverityLog.so

shell_dirs	= Shell_files/directories.sh
shell_API	= Shell_files/make_api.sh

all: directories clean log.so api msg

directories:
	@./$(shell_dirs)

clean:
	rm -rf Dynamic_libraries/*

log.so: $(src_svr_log)
	gcc -Wall -g -fPIC -shared $(src_svr_log) -o $(so_svr_log)

api:
	@./$(shell_API)

msg:
	@echo "**************************************************************************************";
	