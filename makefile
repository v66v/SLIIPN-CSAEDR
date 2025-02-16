# Copyright 2024 v66v
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SRC_DIR ?= src
BUILDDIR := build
DESTDIR ?=  $(CURDIR)/$(BUILDDIR)
export DESTDIR

INC_DIRS = $(SRC_DIR)/
INCS := $(addprefix -I,$(INC_DIRS))

FIND_NOT := -not -path "$(SRC_DIR)/prog/*" -not -path "$(SRC_DIR)/gtk/*"
SRC_FILES_C := $(shell find $(SRC_DIR)/ $(FIND_NOT) -name "*.c")
SRC_FILES_CPP := $(shell find $(SRC_DIR)/ $(FIND_NOT) -name "*.cpp")
SRC_FILES_PROG_C := $(shell find $(SRC_DIR)/prog/ -name "*.c")
SRC_FILES_PROG_CPP := $(shell find $(SRC_DIR)/prog/ -name "*.cpp")

OBJS_DIR_C = $(BUILDDIR)/objects-c
OBJS_DIR_CPP = $(BUILDDIR)/objects-cpp
OBJS_DIR_PROG = $(OBJS_DIR_C)/prog/ $(OBJS_DIR_CPP)/prog/

OBJS_C := $(SRC_FILES_C:$(SRC_DIR)/%.c=$(OBJS_DIR_C)/%.o)
OBJS_CPP := $(SRC_FILES_CPP:$(SRC_DIR)/%.cpp=$(OBJS_DIR_CPP)/%.o)

OBJS_PROG_C := $(SRC_FILES_PROG_C:$(SRC_DIR)/%.c=$(OBJS_DIR_C)/%.o)
OBJS_PROG_CPP := $(SRC_FILES_PROG_CPP:$(SRC_DIR)/%.cpp=$(OBJS_DIR_CPP)/%.o)

DEPS := $(OBJS_C:.o=.d)
DEPS += $(OBJS_CPP:.o=.d)
DEPS += $(OBJS_PROG_C:.o=.d)
DEPS += $(OBJS_PROG_CPP:.o=.d)
-include $(DEPS)

PKGCONFIGS_CFLAGS :=
PKGCONFIGS_LIBS :=

ifneq ($(PKGCONFIGS_CFLAGS),)
	PKGCONFIG_CFLAGS = `pkg-config --cflags $(PKGCONFIGS_CFLAGS)`
endif

ifneq ($(PKGCONFIGS_LIBS),)
	PKGCONFIG_LIBS = `pkg-config --libs $(PKGCONFIGS_LIBS)`
endif

LIB_NAMES := crypto ssl sqlite3 std
LIBS := $(addprefix -l,$(LIB_NAMES))
LIBS += $(PKGCONFIG_LIBS)

CFLAGS = -std=gnu11 -MMD -MP -flto -O2
CFLAGS += $(PKGCONFIG_CFLAGS)

CXXFLAGS = -std=gnu++17 -MMD -MP -flto -O2
CXXFLAGS += $(PKGCONFIG_CFLAGS)

CC = gcc
CXX = g++

bins := main can_send can_play sha sql gtk sqlite_server
BINS := $(addprefix $(BUILDDIR)/bin/, $(bins))

.DEFAULT_GOAL := all
all: build

debug: CFLAGS += -DDEBUG -ggdb3 -Wall -Wextra -Wswitch-enum -Wswitch-default
debug: CXXFLAGS += -DDEBUG -ggdb3 -Wall -Wextra -Wswitch-enum -Wswitch-default
debug: debug_info build

build: dirs makefile $(BINS)


$(BUILDDIR)/bin/main: $(OBJS_C) $(OBJS_CPP) $(OBJS_DIR_CPP)/prog/main.o
	$(info [CXX] Linking object files: $@)
	@$(CXX) $^ -o $@ $(LIBS)

$(BUILDDIR)/bin/can_play: $(OBJS_DIR_C)/can/can_t.o $(OBJS_DIR_C)/can/can.o $(OBJS_DIR_C)/prog/can_play.o
	$(info [CC] Linking object files: $@)
	@$(CC) $^ -o $@

$(BUILDDIR)/bin/can_send: $(OBJS_DIR_C)/can/can_t.o $(OBJS_DIR_C)/can/can.o $(OBJS_DIR_C)/prog/can_send.o
	$(info [CC] Linking object files: $@)
	@$(CC) $^ -o $@

$(BUILDDIR)/bin/sqlite_server: $(OBJS_DIR_C)/sha/sha.o $(OBJS_DIR_C)/global.o $(OBJS_DIR_C)/db/db_utils.o $(OBJS_DIR_C)/db/sqlite.o $(OBJS_DIR_C)/prog/sqlite_server.o
	$(info [CC] Linking object files: $@)
	@$(CC) $^ -lsqlite3 -lstd -lssl -lcrypto -o $@

.PHONY: $(BUILDDIR)/bin/gtk
$(BUILDDIR)/bin/gtk:
	@$(MAKE) -C $(SRC_DIR)/gtk --no-print-directory

$(BUILDDIR)/bin/sha: $(OBJS_DIR_C)/sha/sha.o $(OBJS_DIR_C)/prog/sha.o
	$(info [CC] Linking object files: $@)
	@$(CC) $^ -o $@ $(LIBS)

$(BUILDDIR)/bin/sql: $(OBJS_DIR_C)/prog/sql.o
	$(info [CC] Linking object files: $@)
	@$(CC) $^ -o $@ -lsqlite3 -lstd

$(OBJS_DIR_C)/%.o: $(SRC_DIR)/%.c makefile
	$(info [CC] $<)
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(OBJS_DIR_CPP)/%.o: $(SRC_DIR)/%.cpp makefile
	$(info [CXX] $<)
	@$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

install: install_info build
	@mkdir -p $(DESTDIR)/
ifneq ($(BUILDDIR), $(DESTDIR))
	@cp -r $(BUILDDIR)/bin/ $(DESTDIR)/
else
	$(info [WRN] BUILDDIR($(BUILDDIR)) == DESTDIR($(DESTDIR)))
endif

dirs: $(BUILDDIR)/etc/ $(BUILDDIR)/bin/ $(dir $(OBJS_C)) $(dir $(OBJS_CPP)) $(OBJS_DIR_PROG)

.PRECIOUS: $(BUILDDIR)/. $(BUILDDIR)/%/.
$(BUILDDIR)/:
	$(info [INFO] Creating directory $@)
	@mkdir -p $@

$(BUILDDIR)/%/:
	$(info [INFO] Creating directory $@)
	@mkdir -p $@

.PHONY: clean ccls debug_info install_info clean_build clean_data
debug_info:
	$(info [INFO] Compiling In Debug Mode)

install_info:
	$(info [INFO] Installing Executable [$(DESTDIR)/bin/$(BIN_NAME)])

clean: clean_build clean_data

clean_build:
	$(info [INFO] Cleaning Build)
	@rm -rf $(BUILDDIR)/

clean_data:
	$(info [INFO] Cleaning data)
	@rm -rf data/csv/
	@rm -rf data/images/
	@rm -rf data/raw/
	@rm -rf log/

ccls:
	$(info [INFO] Creating .ccls file)
	@printf "%s\\n" $(CXX) $(subst ' ','\n',$(LIBS)) '-lsqlite3' $(CXXFLAGS) \
		$(subst ' ','\n',$(INCS)) -I$(subst :, -I,$(C_INCLUDE_PATH)) \
		-I$(subst :, -I,$(CPLUS_INCLUDE_PATH)) > .ccls
