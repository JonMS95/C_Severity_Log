src_svr_log = Source_files/SeverityLog.c

so_svr_log	= Dynamic_libraries/libSeverityLog.so

shell_dirs	= Shell_files/directories.sh
shell_sym_links	= Shell_files/sym_links.sh

p_api	= config/API/Header_files
P_api	= API/Header_files
s_api	= config/API/Dynamic_libraries
S_api	= API/Dynamic_libraries

all: directories clean log.so api msg

directories:
	@./$(shell_dirs)

clean:
	rm -rf Dynamic_libraries/* API/Header_files/* API/Dependency_files/*

log.so: $(src_svr_log)
	gcc -Wall -g -fPIC -shared $(src_svr_log) -o $(so_svr_log)

api:
	@bash $(shell_sym_links) -p $(p_api) -P $(P_api) -s $(s_api) -S $(S_api)

msg:
	@echo "**************************************************************************************";
	