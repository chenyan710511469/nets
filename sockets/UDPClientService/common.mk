#########################################################################
###
###  DESCRIPTION:
###    Common definitions for all Makefiles.
###
###  MAKEFILE OPTIONS:
###    These are options that should be specified in the Makefile.
###    At a minimum, one or more of ARC_TARGET, SO_TARGET,
###    APP_TARGET, and/or DIRS must be defined.
###
###    ARC_TARGET - target name for archive file, excluding file
###      extension.
###    SO_TARGET - target name for shared object file, excluding file
###      extension.
###    APP_TARGET - target name of application.
###    DIRS - list of subdirectories to execute "make" in.
###    OBJS - list of object files (without .o extension) that compose
###      the target.
###    LIBS - optional list of libraries that the target uses--
###      don't include the suffix (.a). Libraries that other libraries
###      depend on (e.g. romcon) should be put later in the list.
###    INSTALL_INC - list of header files to install. (Libs will always
###      be installed.)
###    INSTALL_INC_LOC - Use "os" to specify that headers should be
###      installed into the os-specific directory. Otherwise the
###      headers will be installed to "common" (default=common).
###    INSTALL_LIB_LOC - Use "os" to specify that libraries should be
###      installed into the os-specific directory. Otherwise the
###      libraries will be installed to "common" (default=common).
###
###  TOOLKIT ENVIRONMENT VARIABLES:
###    These environment variables should be set before compiling.
###
###    ETI_TOOLKIT - must point to the base directory of the
###      Equator toolkit.
###    ETI_TOOLKIT_INSTALL - must point to a location to put
###      the built driver binaries.
###    ETI_TOOLKIT_LOCAL - optional variable that can point to a
###      location to pick up headers and libraries before the
###      Equator toolkit. It should point to the same location
###      as ETI_TOOLKIT_INSTALL.
###
###  OPTIONAL ENVIRONMENT VARIABLES:
###    These are options that are normally set in environment varibles.
###    They can also be set in the Makefile or on the make command line
###    (e.g. "make DEBUG=1"). Settings in the Makefile takes precendence
###    over the command line, which takes precendence over environment
###    variables. Unless otherwise stated, options should be set to 1
###    or 0 or left unset to use the default. Settings on the make
###    command line will propagate down to subdirectories when building
###    a tree.
###
###    DEBUG - include symbols and define "DEBUG" symbol during
###      compile (default=0).
###    INC_PATH - Additional directories to be searched for headers,
###      separated by spaces. Default is to use the compiler's default
###      path, adding RTOS_INCLUDES for VxWorks builds.
###    LIB_PATH - Additional directories to be searched for libraries,
###      separated by spaces. Default is to use the linker's default
###      path.
###    CFLAGS - Additional compile options.
###    LDFLAGS - Additional link options.
###
#########################################################################

## Add appropriate suffixes and extensions

ifneq ($(ARC_TARGET),)
  ARC_TARGET := lib$(ARC_TARGET)$(LIB_SUFFIX).a
endif

ifneq ($(SO_TARGET),)
  SO_TARGET := lib$(SO_TARGET)$(LIB_SUFFIX).so
endif

## Special variables to help with clean targets

DIRSC := $(foreach dir,$(DIRS),$(dir)(clean))

ASMS := $(foreach obj,$(OBJS),$(basename $(obj)).s)


## Put the extension on all objs

OBJS := $(foreach obj,$(OBJS),$(obj).o)

## Turn on Exception call stack
CFLAGS += -fno-omit-frame-pointer -fPIC -std=c++11 

## Turn on debug flag and define DEBUG symbol for debug builds
ifeq ($(DEBUG),0)
  CFLAGS += -O2
  CFLAGS += -DNDEBUG
endif

ifeq ($(DEBUG),1)
  CFLAGS +=  -O0
  CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(DEBUG),2)
  CFLAGS +=  -g
  CFLAGS += -DDEBUG=1
endif

ifeq ($(DEBUG),3)
    CFLAGS += -O3
endif

ifeq ($(LINUX_COMPILER),_EQUATOR_)
   CFLAGS += -D_EQUATOR_
endif

ifeq ($(PWLIB_SUPPORT),1)
   CFLAGS += -DPWLIB_SUPPORT -DPTRACING=1 -D_REENTRANT -DPHAS_TEMPLATES -DPMEMORY_CHECK=1 -DPASN_LEANANDMEAN -pipe -fPIC
endif

CFLAGS += -D_LINUX_ 
## Add include path and constant definitions to
## compile options

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))


## Add library path and libraries to link options
LDFLAGS += $(foreach lib,$(LIB_PATH),-L$(lib))

ifeq ($(LINUX_COMPILER),_HHPPC_)
   LDFLAGS += --static
endif

ifeq ($(LINUX_COMPILER),_ARM_DM6446_)
   LDFLAGS += --static
endif

ifeq ($(LINUX_COMPILER),_NETRA600_)
   CFLAGS += -DTI_816X_BUILD -DPLATFORM_EVM_SI -DUTILS_ASSERT_ENABLE -DTI816X_DVR -D_LINUX_ -D_NETRA600_
endif


## When using a shared object library and not building
## the shared object library itself, don't link with the
## libraries. Don't know how to do "or" in make, so use
## an intermediate variable.
LDFLAGS += $(foreach lib,$(LIBS),-l$(lib)$(LIB_SUFFIX))

## Set up library install location
ifndef INSTALL_LIB_PATH
  ifeq ($(INSTALL_LIB_LOC),os)
    INSTALL_LIB_PATH = $(LIB_PATH)
  else
    INSTALL_LIB_PATH = $(LIB_PATH)
  endif
endif


## Set up application install location
ifndef INSTALL_APP_PATH
  APP_DIR ?= bin
  ETI_TOOLKIT_INSTALL = .
  INSTALL_APP_PATH = $(ETI_TOOLKIT_INSTALL)/$(APP_DIR)
endif


## Set up the tools to use
ifeq ($(LINUX_COMPILER),_HHPPC_)
CROSS = ppc_82xx-
endif

ifeq ($(PPC_LINUX_VERSION),_85xx_)
	CROSS = ppc_85xx-
endif

ifeq ($(LINUX_COMPILER),_ARM_DM6446_)
   CROSS = /opt/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-
endif

CC      = $(CROSS)gcc
CPP     = $(CROSS)g++
LD      = $(CROSS)g++
AR      = $(CROSS)ar
INSTALL = install -D -m 644
OBJDUMP = objdump
RM      = rm -f


ifeq ($(LINUX_COMPILER),_EQUATOR_)
CC      = /usr/local/Equator/v6.0/tools/i686_Linux/bin/ecc
STRIP   = /usr/local/Equator/v6.0/tools/i686_Linux/bin/estrip
CPP     = /usr/local/Equator/v6.0/tools/i686_Linux/bin/ecc
LD      = /usr/local/Equator/v6.0/tools/i686_Linux/bin/ecc
endif

ifeq ($(LINUX_COMPILER),_ARMLINUX_)
CC      = /opt/arm/bin/arm-linux-gcc
STRIP   = /root/arm9/bin/arm-linux-strip
CPP     = /opt/arm/bin/arm-linux-g++
LD      = /root/arm9/bin/arm-linux-ar
endif

ifeq ($(LINUX_COMPILER),_DAVINCI_DM6446_)
CC      = /opt/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-gcc
STRIP   = /opt/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-strip
CPP     = /opt/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-g++
LD      = /opt/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-g++
AR      = /opt/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-ar
endif

ifeq ($(LINUX_COMPILER),_NETRA600_)
CC      = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc
STRIP   = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-strip
CPP     = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-g++
LD      = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-g++
AR      = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-ar
endif

##------------------------------------------------------------------------
## Rules

## Suffix rules

$(SRC_DIR)/%.o: $(SRC_DIR)/%.s
	$(CC) -c -o $@ $(CFLAGS) $<
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $(CFLAGS) $<
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CPP) -c -o $@ $(CFLAGS) $<


## Rules for making archives

ifneq ($(strip $(ARC_TARGET)),)
  
  ifneq ($(LINUX_COMPILER),_HHPPC_)
      CFLAGS += -DFD_SETSIZE=512
  endif
  
  all: install
  
  install: install_inc install_arc
  
  install_arc: $(ARC_TARGET)
	$(INSTALL) $(ARC_TARGET) $(INSTALL_LIB_PATH)/$(ARC_TARGET)
	chmod +x $(INSTALL_LIB_PATH)/$(ARC_TARGET)
	$(RM) $(ARC_TARGET)
  $(ARC_TARGET) : $(OBJS)
	$(AR) crus $(ARC_TARGET) $(OBJS)

  uninstall: uninstallarc
  
  uninstallarc:
	$(foreach file, $(INSTALL_INC), $(RM) $(INSTALL_INC_PATH)/$(file) ;)
	$(RM) $(INSTALL_LIB_PATH)/$(ARC_TARGET)


endif


## Rules for making shared object

ifneq ($(strip $(SO_TARGET)),)
  
  all: install
  
  install: install_inc install_so
  
  install_so: $(SO_TARGET)
	$(INSTALL) $(SO_TARGET) $(INSTALL_LIB_PATH)/$(SO_TARGET)
	chmod +x $(INSTALL_LIB_PATH)/$(SO_TARGET)
	$(RM) $(SO_TARGET)
	$(RM) $(SO_TARGET).map
  $(SO_TARGET) : $(OBJS)
	$(LD) $(OBJS) -fPIC -shared -o $(SO_TARGET) $(LDFLAGS)
	$(OBJDUMP) --syms $(SO_TARGET) | sort | grep " g" > $(SO_TARGET).map

  uninstall: uninstallso

  uninstallso:
	$(foreach file, $(INSTALL_INC), $(RM) $(INSTALL_INC_PATH)/$(file) ;)
	$(RM) $(INSTALL_LIB_PATH)/$(ARC_TARGET)

endif


## Rules for making applications

ifneq ($(strip $(APP_TARGET)),)

  all: $(APP_TARGET) install

  install: install_inc install_app
  
  install_app: $(APP_TARGET)
	$(INSTALL) $(APP_TARGET) $(INSTALL_APP_PATH)/$(APP_TARGET)
	chmod +x $(INSTALL_APP_PATH)/$(APP_TARGET)
	$(RM) $(APP_TARGET)
  $(APP_TARGET): $(OBJS)
	$(LD) $(OBJS) -o $(APP_TARGET) $(LDFLAGS)

ifneq ($(strip $(STRIP)),)
		$(STRIP) $(APP_TARGET)
endif	
endif


## Rules for making subdirectories

ifneq ($(strip $(DIRS)),)

  all: $(DIRS)
  $(DIRS): FORCE
	$(MAKE) -C $@
  $(DIRSC): FORCE
	$(MAKE) -C $@ clean
  FORCE:
  clean: $(DIRSC)

endif


## Shared rules

install: install_inc

install_inc:
	$(foreach file, $(INSTALL_INC), $(INSTALL) $(file) $(INSTALL_INC_PATH)/$(notdir $(file)) ;)

clean:
	$(RM) $(ASMS)  $(OBJS)  *.pdb *.map
	$(RM) $(APP_TARGET) $(INSTALL_APP_PATH)/$(APP_TARGET)
	$(RM) $(SO_TARGET) $(INSTALL_LIB_PATH)/$(SO_TARGET)
	$(RM) $(ARC_TARGET) $(INSTALL_LIB_PATH)/$(ARC_TARGET)

## Rule to pre-install all headers

setup:
	(cd $(TOP);    \
	make install_inc;   \
	echo )
