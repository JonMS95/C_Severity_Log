#############################################################################################################################################################
# Common variables
CONFIG_FILE	:= config.xml

# Common shell files data
SH_FILES_LOCAL_NAME 			:= Common_shell_files
SH_FILES_NODE 					:= config/Common_shell_files/
SH_FILES_PATH 					:= $(shell xmlstarlet sel -t -v "$(SH_FILES_NODE)@local_path" $(CONFIG_FILE))
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
	VISIBILITY := -fvisibility=hidden
else ifeq ($(LIBRARY_LANG), C++)
	COMP := $(CXX)
	CXXFLAGS := $(DEBUG_INFO)
	FLAGS := $(CXXFLAGS)
	VISIBILITY := 
endif

# Basic system dependencies
BASIC_SYSTEM_DEPS := $(COMP) xmlstarlet git

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

TEST_SRC_MAIN	:= Tests/Source_files/*
TEST_EXE_MAIN	:= Tests/Executable_files/main

D_TEST_DEPS		:= config/Tests/Dependencies/
#################################################

#################################################################################
# Compound rules
exe: clean check_basic_deps check_sh_deps ln_sh_files directories deps so_lib api

test: clean_test directories test_deps test_main test_exe
#################################################################################

##########################################################################
# Basic dependencies
check_basic_deps:
	@for i in $(BASIC_SYSTEM_DEPS); do								 \
		if ! command -v $${i} > /dev/null 2>&1; then 			 	 \
			echo "$${i} is not installed in the machine."			;\
			sudo apt install $${i}									;\
		else														 \
			echo "$${i} is already installed in the machine"		;\
		fi															;\
	done															;\

check_sh_deps:
	@echo "Check whether or not do Common shell files deps exist"	;\
	if [ -d $(SH_FILES_PATH) ]; then								 \
																	 \
		if [ -d $(SH_FILES_DEP_PATH) ]; then						 \
																	 \
			echo "API found, everything is OK." 					;\
																	 \
		else 														 \
																	 \
			if [ $(SH_FILES_VERSION_MODE) = "DEBUG" ]; then 		 \
				echo "Cannot download DEBUG versions from GitHub"	;\
				exit 1 												;\
			fi 														;\
																	 \
			cd $(SH_FILES_PATH) 									;\
																	 \
			if [ ! -d API ]; then 									 \
				./Source_files/directories.sh 						;\
			fi 														;\
																	 \
			git pull 												;\
			git checkout "tags/$(SH_FILES_VERSION)" 				;\
			./Source_files/gen_CSF_version.sh 						;\
			git checkout main 										;\
	 		git pull 												;\
		fi 															;\
																	 \
	else															 \
		if [ $(SH_FILES_VERSION_MODE) = "DEBUG" ]; then 			 \
			echo "Cannot download DEBUG versions from GitHub"		;\
			exit 1 													;\
		fi 															;\
																	 \
		if [ ! -d $$(dirname $(SH_FILES_PATH)) ]; then 				 \
			mkdir -p $$(dirname $(SH_FILES_PATH))					;\
		fi 															;\
																	 \
		cd $$(dirname $(SH_FILES_PATH)) 							;\
		git clone $(SH_FILES_URL) 									;\
		cd $(SH_FILES_PATH) 										;\
		git checkout "tags/$(SH_FILES_VERSION)" 					;\
		./Source_files/gen_CSF_version.sh 							;\
		git checkout main 											;\
	 	git pull 													;\
																	 \
	fi 																;\

##########################################################################

#############################################################################
# Exe Rules
clean:
	rm -rf $(SH_FILES_LOCAL_NAME) Object_files Dynamic_libraries Dependency_files

ln_sh_files:
	ln -sf $(SH_FILES_DEP_PATH) $(SH_FILES_LOCAL_NAME)

directories:
	@./$(SHELL_DIRS)

deps:
	@bash $(SHELL_SYM_LINKS)

$(LIB_SO): $(LIB_SOURCES)
	$(COMP) $(VISIBILITY) $(FLAGS) -I$(HEADER_DEPS_DIR) -fPIC -shared $(LIB_SOURCES) -o $(LIB_SO)

so_lib: $(LIB_SO)

api:
	@bash $(SHELL_GEN_VERSIONS)

# Use this one carefully. Non-tagged versions will be impossible to recover if used.
clean_api:
	rm -rf API
#############################################################################

##########################################################################################################################
# Test Rules
clean_test:
	rm -rf Tests/Dependency_files Tests/Object_files Tests/Executable_files

test_deps:
	@bash $(SHELL_SYM_LINKS) -d $(D_TEST_DEPS)

$(TEST_EXE_MAIN): $(TEST_SRC_MAIN) $(wildcard $(TEST_SO_DEPS_DIR)/*.so) $(wildcard $(TEST_HEADER_DEPS_DIR)/*.h)
	$(COMP) $(FLAGS) -I$(TEST_HEADER_DEPS_DIR) $(TEST_SRC_MAIN) -L$(TEST_SO_DEPS_DIR) $(addprefix -l,$(patsubst lib%.so,%,$(shell ls $(TEST_SO_DEPS_DIR)))) -o $(TEST_EXE_MAIN)

test_main: $(TEST_EXE_MAIN)

test_exe:
	@./$(LOCAL_SHELL_TEST)
##########################################################################################################################
