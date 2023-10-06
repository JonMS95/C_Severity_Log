#############################################################################################################################################################
# Common variables
config_file	= config.xml

SH_FILES_PATH := $(shell xmlstarlet sel -t -v "config/Common_shell_files/@local_path" $(config_file))
PRJ_DATA_NODE := config/Project_data/
VERSION_MAJOR := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_major" $(config_file))"
VERSION_MINOR := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_minor" $(config_file))"
VERSION := v$(VERSION_MAJOR)_$(VERSION_MINOR)
VERSION_MODE := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_mode" $(config_file))"

shell_dirs			= Common_shell_files/directories.sh
shell_sym_links		= Common_shell_files/sym_links.sh
shell_gen_versions 	= Common_shell_files/gen_version.sh
shell_test			= Shell_files/test.sh

ifeq ($(VERSION_MODE), "DEBUG")
	DEBUG_INFO := -g -Wall
else
	DEBUG_INFO :=
endif
#############################################################################################################################################################

#################################################
# Library variables
src_svr_log = Source_files/SeverityLog.c

so_svr_log	= Dynamic_libraries/libSeverityLog.so
#################################################

#############################################
# Compound rules
all: clean ln_sh_files directories log.so api
#############################################

#############################################################################
# Exe Rules
clean:
	rm -rf Common_shell_files Object_files Dynamic_libraries Dependency_files

ln_sh_files:
	ln -sf $(SH_FILES_PATH) Common_shell_files

directories:
	@./$(shell_dirs)

log.so: $(src_svr_log)
	gcc $(DEBUG_INFO) -fPIC -shared $(src_svr_log) -o $(so_svr_log)

api:
	@bash $(shell_gen_versions)

# Use this one carefully. Non-tagged versions will be impossible to recover if used.
clean_api:
	rm -rf API
#############################################################################