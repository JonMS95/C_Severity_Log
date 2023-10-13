#############################################################################################################################################################
# Common variables
CONFIG_FILE	:= config.xml

# Common shell files data
SH_FILES_LOCAL_NAME 			:= Common_shell_files
SH_FILES_NODE 					:= config/Common_shell_files/
SH_FILES_PATH 					:= $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@local_path" $(CONFIG_FILE))
SH_FILES_LOCAL_PATH_BASENAME 	:= $(shell basename $(SH_FILES_PATH))
SH_FILES_VERSION_MAJOR 			:= $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@version_major" $(CONFIG_FILE))
SH_FILES_VERSION_MINOR 			:= $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@version_minor" $(CONFIG_FILE))
SH_FILES_VERSION_MODE 			:= $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@version_mode" $(CONFIG_FILE))
SH_FILES_URL 					:= $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@URL" $(CONFIG_FILE))

ifeq ($(SH_FILES_VERSION_MODE), DEBUG)
	SH_FILES_VERSION_MODE_SUFFIX := _DEBUG
else
	SH_FILES_VERSION_MODE_SUFFIX :=
endif

SH_FILES_VERSION 				:= v$(SH_FILES_VERSION_MAJOR)_$(SH_FILES_VERSION_MINOR)$(SH_FILES_VERSION_MODE_SUFFIX)
SH_FILES_DEP_PATH 				:= $(SH_FILES_PATH)/API/$(SH_FILES_VERSION)

# Project data
PRJ_DATA_NODE 	:= config/Project_data/
VERSION_MODE 	:= $(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_mode" $(CONFIG_FILE))
VERSION_MAJOR 	:= $(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_major" $(CONFIG_FILE))
VERSION_MINOR	:= $(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_minor" $(CONFIG_FILE))
LIBRARY_LANG	:= $(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@language" $(CONFIG_FILE))
LIBRARY_NAME 	:= $(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@library_name" $(CONFIG_FILE))
SO_FILE_NAME 	:= lib$(LIBRARY_NAME).so.$(VERSION_MAJOR).$(VERSION_MINOR)

# Shell files
SHELL_DIRS			:= $(SH_FILES_LOCAL_NAME)/directories.sh
SHELL_SYM_LINKS		:= $(SH_FILES_LOCAL_NAME)/sym_links.sh
SHELL_GEN_VERSIONS 	:= $(SH_FILES_LOCAL_NAME)/gen_version.sh

LOCAL_SHELL_TEST	:= Shell_files/test.sh

# Debug flags
ifeq ("$(VERSION_MODE)", "DEBUG")
	DEBUG_INFO := -g -Wall
else
	DEBUG_INFO :=
endif

# Compiler selection and flags
ifeq ($(LIBRARY_LANG), C)
	COMP := $(CC)
	CFLAGS := $(DEBUG_INFO)
	FLAGS := $(CFLAGS)
else ifeq ($(LIBRARY_LANG), C++)
	COMP := $(CXX)
	CXXFLAGS := $(DEBUG_INFO)
	FLAGS := $(CXXFLAGS)
endif

# Dependencies directories
HEADER_DEPS_DIR			:= Dependency_files/Header_files
SO_DEPS_DIR				:= Dependency_files/Dynamic_libraries

TEST_HEADER_DEPS_DIR	:= Tests/$(HEADER_DEPS_DIR)
TEST_SO_DEPS_DIR		:= Tests/$(SO_DEPS_DIR)
#############################################################################################################################################################

#################################################
# Library variables
LIB_SOURCES		:= Source_files/*
LIB_SO			:= Dynamic_libraries/$(SO_FILE_NAME)

TEST_SRC_MAIN	:= Tests/Source_files/main.c
TEST_EXE_MAIN	:= Tests/Executable_files/main

D_TEST_DEPS		:= config/Tests/Dependencies/
#################################################

############################################################################
# Compound rules
exe: clean check_xmlstarlet check_sh_deps ln_sh_files directories so_lib api

test: clean_test directories test_deps test_main test_exe
############################################################################

##########################################################################
# Basic dependencies

check_xmlstarlet:
	@if ! command -v xmlstarlet > /dev/null 2>&1; then 			 \
		echo "xmlstarlet is not installed on the machine."		;\
		sudo apt install xmlstarlet								;\
	fi															;\

sh_echo:
	@echo "SH_FILES_DEP_PATH = $(SH_FILES_DEP_PATH)"
	@echo "SH_FILES_VERSION_MODE = $(SH_FILES_VERSION_MODE)"
	@echo "SH_FILES_PATH = $(SH_FILES_PATH)"
	@echo "SH_FILES_URL = $(SH_FILES_URL)"
	@echo "SH_FILES_LOCAL_PATH_BASENAME = $(SH_FILES_LOCAL_PATH_BASENAME)"
	@echo "SH_FILES_VERSION = $(SH_FILES_VERSION)"
	@echo "SO library name: $(SO_FILE_NAME)"
	@echo "LIB LANGUAGE = $(LIBRARY_LANG)"
	@echo "COMPILER = $(COMP)"

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
	rm -rf $(SH_FILES_LOCAL_NAME) Object_files Dynamic_libraries Dependency_files

ln_sh_files:
	ln -sf $(SH_FILES_DEP_PATH) $(SH_FILES_LOCAL_NAME)

directories:
	@./$(SHELL_DIRS)

so_lib:
	$(COMP) $(FLAGS) -I$(HEADER_DEPS_DIR) -fPIC -shared $(LIB_SOURCES) -o $(LIB_SO)

api:
	@bash $(SHELL_GEN_VERSIONS)

# Use this one carefully. Non-tagged versions will be impossible to recover if used.
clean_api:
	rm -rf API
#############################################################################

######################################################################################################################
# Test Rules
clean_test:
	rm -rf Tests/Dependency_files Tests/Object_files Tests/Executable_files

test_deps:
	@bash $(SHELL_SYM_LINKS) -d $(D_TEST_DEPS)

test_main:
	$(COMP) $(FLAGS) -I$(TEST_HEADER_DEPS_DIR) $(TEST_SRC_MAIN) -L$(TEST_SO_DEPS_DIR) -lSeverityLog -o $(TEST_EXE_MAIN)

test_exe:
	@./$(LOCAL_SHELL_TEST)
######################################################################################################################
