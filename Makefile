#############################################################################################################################################################
# Common variables
config_file	:= config.xml

SH_FILES_NODE := config/Common_shell_files/
SH_FILES_PATH := $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@local_path" $(config_file))
SH_FILES_LOCAL_PATH_BASENAME := $(shell basename $(SH_FILES_PATH))
SH_FILES_VERSION_MAJOR := $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@version_major" $(config_file))
SH_FILES_VERSION_MINOR := $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@version_minor" $(config_file))
SH_FILES_VERSION_MODE := $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@version_mode" $(config_file))
SH_FILES_URL := $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@URL" $(config_file))
ifeq ($(SH_FILES_VERSION_MODE), DEBUG)
	SH_FILES_VERSION_MODE_SUFFIX := _DEBUG
else
	SH_FILES_VERSION_MODE_SUFFIX :=
endif
SH_FILES_VERSION := v$(SH_FILES_VERSION_MAJOR)_$(SH_FILES_VERSION_MINOR)$(SH_FILES_VERSION_MODE_SUFFIX)
SH_FILES_DEP_PATH := $(SH_FILES_PATH)/API/$(SH_FILES_VERSION)

PRJ_DATA_NODE := config/Project_data/
VERSION_MODE := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_mode" $(config_file))"

shell_dirs			:= Common_shell_files/directories.sh
shell_sym_links		:= Common_shell_files/sym_links.sh
shell_gen_versions 	:= Common_shell_files/gen_version.sh
shell_test			:= Shell_files/test.sh

ifeq ($(VERSION_MODE), "DEBUG")
	DEBUG_INFO := -g -Wall
else
	DEBUG_INFO :=
endif

HEADER_DEPS_DIR			:= Dependency_files/Header_files
SO_DEPS_DIR				:= Dependency_files/Dynamic_libraries

TEST_HEADER_DEPS_DIR	:= Tests/$(HEADER_DEPS_DIR)
TEST_SO_DEPS_DIR		:= Tests/$(SO_DEPS_DIR)

API_FILE := $(shell find Source_files -maxdepth 1 -type f -name '*_api.h')
OUTPUT_SO := $(patsubst %_api.h,%.so,$(notdir $(API_FILE)))
LIBRARY_NAME := lib$(OUTPUT_SO)
#############################################################################################################################################################

#################################################
# Library variables
lib_sources	:= Source_files/*

lib_so		:= Dynamic_libraries/$(LIBRARY_NAME)

src_main	:= Tests/Source_files/main.c

exe_main	:= Tests/Executable_files/main

d_test_deps	:= config/Tests/Dependencies/
#################################################

############################################################################
# Compound rules
exe: clean check_xmlstarlet check_sh_deps ln_sh_files directories log.so api

test: clean_test directories test_deps test_main test_exe
############################################################################

##########################################################################
# Basic dependencies

check_xmlstarlet:
	@if ! command -v xmlstarlet > /dev/null 2>&1; then 			 \
		echo "xmlstarlet is not installed on the machine."		;\
		sudo apt install xmlstarlet								;\
	fi															;\

check_sh_deps:
	@if [ ! -d $(SH_FILES_DEP_PATH) ]; then 							 \
																		 \
		echo "$(SH_FILES_DEP_PATH) does not exist!"						;\
																		 \
		if [ $(SH_FILES_VERSION_MODE) = "DEBUG" ]; then 				 \
			echo "Cannot download DEBUG versions from GitHub"			;\
			exit 1														;\
		fi																;\
																		 \
		if [ -d $$(dirname $(SH_FILES_PATH)) ]; then 					 \
			echo "path $$(dirname $(SH_FILES_PATH)) exists"				;\
		else															 \
			echo "path $$(dirname $(SH_FILES_PATH)) DOES NOT exist"		;\
			mkdir -p $$(dirname $(SH_FILES_PATH))						;\
		fi																;\
																		 \
		echo "Cloning from $(SH_FILES_URL)"								;\
		git clone $(SH_FILES_URL)										;\
		cd $(SH_FILES_LOCAL_PATH_BASENAME)								;\
		git checkout "tags/$(SH_FILES_VERSION)"							;\
		./Source_files/gen_CSF_version.sh								;\
		git checkout main												;\
		git pull														;\
		cd ..															;\
		mv $(SH_FILES_LOCAL_PATH_BASENAME) $(SH_FILES_PATH)				;\
																		 \
	fi																	 \

##########################################################################

#############################################################################
# Exe Rules
clean:
	rm -rf Common_shell_files Object_files Dynamic_libraries Dependency_files

ln_sh_files:
	ln -sf $(SH_FILES_DEP_PATH) Common_shell_files

directories:
	@./$(shell_dirs)

log.so:
	gcc $(DEBUG_INFO) -I$(HEADER_DEPS_DIR) -fPIC -shared $(lib_sources) -o $(lib_so)

api:
	@bash $(shell_gen_versions)

# Use this one carefully. Non-tagged versions will be impossible to recover if used.
clean_api:
	rm -rf API
#############################################################################

######################################################################################################################
# Test Rules
clean_test:
	rm -rf Tests/Dependency_files Tests/Object_files Tests/Executable_files

test_deps:
	@bash $(shell_sym_links) -d $(d_test_deps)

test_main:
	gcc $(DEBUG_INFO) -I$(TEST_HEADER_DEPS_DIR) $(src_main) -L$(TEST_SO_DEPS_DIR) -lSeverityLog -o $(exe_main)

test_exe:
	@./$(shell_test)
######################################################################################################################
