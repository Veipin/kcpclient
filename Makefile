######################################################################
# makefile v2, write by helq at 2012-11-30
# Author: helq
# Date: xxxx-xx-xx
# Revison:
#   1、xxx add/modify/delete xxx at xxxx-xx-xx
# demo:
# Note:
######################################################################
## cmd
SHELL               := /bin/sh

CT                  := g++    # CT: compile tool
AR                  := ar
CP                  := cp -i
CD                  := cd
LN                  := ln
RM                  := rm -rf
MKDIR               := mkdir
TAR                 := tar

## project dir
PROJ_DIR            := /usr/simroom
PROJ_BIN            := ${PROJ_DIR}/bin
PROJ_INC            := ${PROJ_DIR}/include
PROJ_LIB            := ${PROJ_DIR}/lib
PROJ_CONF           := ${PROJ_DIR}/cf.d

## app compile platform
COS                 := rhels
COV                 := 5.5
ARCH                := $(shell arch)
CPLATFORM           := ${COS}${COV}_${ARCH}

## app version
APP_VER_MAJOR       := 1
APP_VER_MINOR       := 0
APP_VER_RELEASE     := 0
APP_VER             := $(APP_VER_MAJOR).$(APP_VER_MINOR).$(APP_VER_RELEASE)

APP_DEBUG_FLAGS     := APP_DEBUG_CODE
APP_DEBUG_MACRO     := ${patsubst %,-D%,${APP_DEBUG_FLAGS}}

##add by leer 2014-11-05
CT_START	    := 0

BUILD_NO            := 0
BUILD_NO_FN         := buildno
ifeq  (${BUILD_NO_FN}, $(wildcard ${BUILD_NO_FN}))
    BUILD_NO=$(shell awk '{print $$3}' ${BUILD_NO_FN})
endif
BUILD_NO:=$(shell expr ${BUILD_NO} + 1)
$(shell echo "#define BUILD_NO "${BUILD_NO} > ${BUILD_NO_FN})

## app name
#   app type: static/shared/app
APP_TYPE            := app
APP_NAME            := kcpclient
#ll libRoomClient
APP_FILE_NAME       :=${APP_NAME}
APP_FULL_NAME       := ${APP_NAME}_${APP_VER}_${CPLATFORM}
APP_FULL_NAME_RELEASE:=${APP_FULL_NAME}_release
APP_FULL_NAME_DEBUG := ${APP_FULL_NAME}_debug
ifeq    (shared,${APP_TYPE})
    APP_FULL_NAME_RELEASE:=${APP_FULL_NAME_RELEASE}.so
    APP_FULL_NAME_DEBUG := ${APP_FULL_NAME_DEBUG}.so
    APP_FILE_NAME:=${APP_FILE_NAME}.so
else
    ifeq    (static,${APP_TYPE})
        APP_FULL_NAME_RELEASE:=${APP_FULL_NAME_RELEASE}.a
        APP_FULL_NAME_DEBUG := ${APP_FULL_NAME_DEBUG}.a
        APP_FILE_NAME:=${APP_FILE_NAME}.a
    else
    endif
endif

TAR_RELEASE_NAME    := ${APP_FULL_NAME}_`date +"%Y%m%d"`.tgz
TAR_SRC_NAME        := ${APP_NAME}_${APP_VER}_src_`date +"%Y%m%d"`.tgz
CHANG_LOG_NAME      := ChangLog
CONF_FILE_NAME      := ${APP_NAME}.conf
MAKE_FILE_NAME      := Makefile

## files
CFILES          := $(shell ls *.c 2>/dev/null)
CPPFILES        := $(shell ls *.cpp 2>/dev/null)
HFILES          := $(shell ls *.h 2>/dev/null) $(shell ls *.hpp 2>/dev/null)
EOFILES         := 
OBJ_DIR_NAME    := obj
COFILES         := $(addprefix ${OBJ_DIR_NAME}/, $(CFILES:%.c=%.o))
CPPCOFILES      := $(addprefix ${OBJ_DIR_NAME}/, $(CPPFILES:%.cpp=%.o))
OFILES          := ${COFILES} ${CPPCOFILES} ${EOFILES}
CLEAN_FILES     := ${OFILES} ${OBJ_DIR_NAME}  \
                   ${APP_FILE_NAME} ${APP_FULL_NAME_RELEASE} ${APP_FULL_NAME_DEBUG}
#SRC_DIR_NAME    := .

# /////////////////////////////////

## build options
INC_DIR     := $(PROJ_INC) ./ ../ /tlg/include/public
LIB_DIR     := $(PROJ_LIB) ../ /tlg/lib
LIB_NAME    := -lpublic
#
SHARED      := -shared -fpic
STATIC      := -crv
MACRO_LIST  += -DAPP_VER="\"$(APP_VER)\"" -DAPP_NAME="\"${APP_NAME}\""      \
               -DCPLATFORM="\"$(CPLATFORM)\""  -DPROJ_DIR="\"$(PROJ_DIR)\"" \
               -DBUILD_NO="\"${BUILD_NO}\"" -DLOGGROUP

INC_DIR     := ${patsubst %,-I%,${INC_DIR}}
LIB_DIR     := ${patsubst %,-L%,${LIB_DIR}}
INSTALL_DIR :=${PROJ_BIN}

CFLAGS_DEBUG        := -g -Wall
CFLAGS_RELEASE      := -O2 -Wall
ifeq    (shared,${APP_TYPE})
    CFLAGS_DEBUG    := ${CFLAGS_DEBUG} ${SHARED}
    CFLAGS_RELEASE  := ${CFLAGS_RELEASE} ${SHARED}
    INSTALL_DIR     :=${PROJ_LIB}
else
    ifeq    (static,${APP_TYPE})
        INSTALL_DIR     :=${PROJ_LIB}
    else
    endif
endif
CFLAGS=${CFLAGS_DEBUG}

## temp var
PRINT_FILE_NAME     := make_print_tmp_file
#################################################################################
#
.SUFFIXES: 
.SUFFIXES: .cpp .c .o
.PHONY: all configure debug release clean install uninstall         \
        print tar test dist

#all: configure clean debug
#all: debug
alld: clean debug installd
allr: clean release installr

debug: override CFLAGS:=${CFLAGS_DEBUG}
debug: override MACRO_LIST+=${APP_DEBUG_MACRO}
debug: configure ${APP_FULL_NAME_DEBUG}
	ln -sf ${APP_FULL_NAME_DEBUG} ${APP_FILE_NAME}
ifeq    (app,${APP_TYPE})
	chmod u+x ${APP_FULL_NAME_DEBUG}
endif

release: override CFLAGS=${CFLAGS_RELEASE}
release: configure ${APP_FULL_NAME_RELEASE}
	ln -sf ${APP_FULL_NAME_RELEASE} ${APP_FILE_NAME}
ifeq    (app,${APP_TYPE})
	chmod u+x ${APP_FULL_NAME_RELEASE}
endif

${APP_FULL_NAME_RELEASE}: ${OFILES} $(MAKE_FILE_NAME)
ifeq ($(findstring print, $(MAKECMDGOALS)), print)
	@echo $? >> ${PRINT_FILE_NAME}
else
	-@echo 
	-@echo "build $@..."
ifeq    (static,${APP_TYPE})
	$(AR) ${STATIC} ${APP_FULL_NAME_RELEASE} ${OFILES}
else
	$(CT) -o ${APP_FULL_NAME_RELEASE} ${OFILES} $(LIB_DIR) $(LIB_NAME)     \
	    $(CFLAGS) $(INC_DIR) $(MACRO_LIST)
endif
endif

${APP_FULL_NAME_DEBUG}: ${OFILES} $(MAKE_FILE_NAME)
ifeq ($(findstring print, $(MAKECMDGOALS)), print)
	@echo $? >> ${PRINT_FILE_NAME}
else
	-@echo 
	-@echo "build $@..."
ifeq    (static,${APP_TYPE})
	$(AR) ${STATIC} ${APP_FULL_NAME_DEBUG} ${OFILES}
else
	$(CT) -o ${APP_FULL_NAME_DEBUG} ${OFILES} $(LIB_DIR) $(LIB_NAME)     \
	    $(CFLAGS) $(INC_DIR) $(MACRO_LIST)
endif
endif

${OBJ_DIR_NAME}/%.o: %.c $(HFILES) ${MAKE_FILE_NAME}
ifeq ($(findstring print, $(MAKECMDGOALS)), print)
	@echo $? >> ${PRINT_FILE_NAME}
else
	-@echo 
	-@echo "build $@..."
	$(CT) -c $< $(CFLAGS) -o $@ $(INC_DIR) $(MACRO_LIST)
endif

${OBJ_DIR_NAME}/%.o: %.cpp $(HFILES) ${MAKE_FILE_NAME}
ifeq ($(findstring print, $(MAKECMDGOALS)), print)
	@echo $? >> ${PRINT_FILE_NAME}
else
	-@echo 
	-@echo "build $@..."
	$(CT) -c $< $(CFLAGS) -o $@ $(INC_DIR) $(MACRO_LIST)
endif

help:
	-@echo "Usage:"
	-@echo "  make [debug]    build ${APP_FULL_NAME_DEBUG}"
	-@echo "  make release    build ${APP_FULL_NAME_RELEASE}"
	-@echo "  make configure  configure before build"
	-@echo "  make clean      clean file created by make"
	-@echo "  make installr   install release module"
	-@echo "  make installd   install debug module"
	-@echo "  make uninstall  uninstall module, debug and release"
	-@echo "  make tar        pack release to ${TAR_RELEASE_NAME}"
	-@echo "  make dist       pack src files to ${TAR_SRC_NAME}"
	-@echo "  make printr     print release newer files"
	-@echo "  make printd     print debug newer files"

configure:
	-@echo "start to configure..."
	@if [ ! -d ${OBJ_DIR_NAME} ]; then \
	    echo "    ${MKDIR} obj dir: ./${OBJ_DIR_NAME}"; \
	    $(MKDIR) -p ${OBJ_DIR_NAME}; \
	fi;

clean:
	-@echo "start to clean..."
	@$(foreach CF, $(wildcard ${CLEAN_FILES}),       \
	    echo "    clean ${CF}";                     \
	    $(RM) ${CF};                                \
	    )

installr:
	-@echo "start to install..."
	$(RM) $(INSTALL_DIR)/$(APP_FULL_NAME_RELEASE)
	$(RM) $(INSTALL_DIR)/${APP_FILE_NAME}
	$(CP) ${APP_FULL_NAME_RELEASE}  $(INSTALL_DIR)/
	$(CD) $(INSTALL_DIR)/;$(LN) -sf  ${APP_FULL_NAME_RELEASE} ${APP_FILE_NAME}

installd:
	-@echo "start to install..."
	$(RM) $(INSTALL_DIR)/$(APP_FULL_NAME_DEBUG)
	$(RM) $(INSTALL_DIR)/${APP_FILE_NAME}
	$(CP) ${APP_FULL_NAME_DEBUG}  $(INSTALL_DIR)/
	$(CD) $(INSTALL_DIR)/;$(LN) -sf  ${APP_FULL_NAME_DEBUG} ${APP_FILE_NAME}

uninstall:
	-@echo "start to uninstall..."
	${RM} $(INSTALL_DIR)/$(APP_FULL_NAME_DEBUG)
	${RM} $(INSTALL_DIR)/$(APP_FULL_NAME_RELEASE)
	${RM} $(INSTALL_DIR)/${APP_FILE_NAME}

tar:
	-@echo "start to tar..."
ifdef CONF_FILE_NAME
	cp ${PROJ_CONF}/${CONF_FILE_NAME} ${CONF_FILE_NAME}
endif
	$(TAR) -czvf ${TAR_RELEASE_NAME} ${APP_FULL_NAME_RELEASE} ${CHANG_LOG_NAME} ${CONF_FILE_NAME}
ifdef CONF_FILE_NAME
	${RM} ${CONF_FILE_NAME}
endif

dist:
ifdef CONF_FILE_NAME
	cp ${PROJ_CONF}/${CONF_FILE_NAME} ${CONF_FILE_NAME}
endif
	$(TAR) -czvf ${TAR_SRC_NAME} $(CFILES) $(CPPFILES) $(HFILES) ${CHANG_LOG_NAME} ${MAKE_FILE_NAME} ${CONF_FILE_NAME}
ifdef CONF_FILE_NAME
	${RM} ${CONF_FILE_NAME}
endif

printr: ${APP_FULL_NAME_RELEASE}
	@echo "newer file list: "$(sort $(shell cat ${PRINT_FILE_NAME}))
	@${RM} ${PRINT_FILE_NAME}

printd: ${APP_FULL_NAME_DEBUG}
	@echo "newer file list: "$(sort $(shell cat ${PRINT_FILE_NAME}))
	@${RM} ${PRINT_FILE_NAME}

test:
	-@echo "start to test..."
	-@echo "APP_FULL_NAME_RELEASE: ${APP_FULL_NAME_RELEASE}"
#	$(warning "warning info")
#	$(error "error info")
	-@echo "c src  file list: ${CFILES}"
	-@echo "cpp src  file list: ${CPPFILES}"
	-@echo "head file list: ${HFILES}"
	-@echo "obj  file list: ${OFILES}"


