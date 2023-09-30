src_svr_log = Source_files/SeverityLog.c

so_svr_log	= Dynamic_libraries/libSeverityLog.so

shell_dirs		= Common_shell_files/directories.sh
shell_sym_links	= Common_shell_files/sym_links.sh

p_api	= config/API/Header_files
P_api	= API/Header_files
s_api	= config/API/Dynamic_libraries
S_api	= API/Dynamic_libraries

# Retrieve the XML selection
SH_FILES_PATH := $(shell xmlstarlet sel -t -v "//$$(xmlstarlet el -a "config.xml" | grep "config/Common_shell_files.*@.*PathToShellFiles")" "config.xml")

all: clean ln_sh_files directories log.so api msg

ln_sh_files:
	ln -sf $(SH_FILES_PATH) Common_shell_files

directories:
	@./$(shell_dirs)

clean:
	rm -rf Common_shell_files Dynamic_libraries/* API/Header_files/* API/Dynamic_libraries/*

log.so: $(src_svr_log)
	gcc -Wall -g -fPIC -shared $(src_svr_log) -o $(so_svr_log)

api:
	@bash $(shell_sym_links) -p $(p_api) -P $(P_api) -s $(s_api) -S $(S_api)

msg:
	@echo "**************************************************************************************";
	