#########################################################################
###
###  描述:
###    程序编译模板
###
#########################################################################

# common.mk 文件所在的目录
COMM_DIR := ./

# cpp 文件所在的目录、目前不支持多目录
SRC_DIR := .

# 编译目标名称
APP_TARGET		:= ChatD
# SO_TARGET		:= ChatD
# ARC_TARGET	:= ChatD

###编译程序位数设置-m32代表32位 -m64代表64位系统
Bits=-m64
## 定义宏

DEBUG = 2
LINUX_COMPILER = g++
#CFLAGS := -D_CARRIER_ -D__UNIX__ $(Bits) -DWITH_GZIP -g -shared -std=gnu++11 -Wl,--stack=0x4000000
CFLAGS := -D_CARRIER_ -D__UNIX__ $(Bits) -DWITH_GZIP -std=gnu++11

# 处理cpp文件

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst %.cpp,%, $(SRCS))

## 依赖的库名称
LIBS += pthread TCPClientService TCPService UDPClientService UDPService ProtocolService json_linux-gcc-7_libmt tinyxml2

## 包含的头文件
INC_PATH += ./
INC_PATH += /home/chenyan/projects/project1/include \
            /home/chenyan/projects/project1/udpClient/include


## 库所在的目录
LIB_PATH += /home/chenyan/projects/project1/lib64

###根据系统导入公共库文件目录
ifeq ($(Bits),-m64)
   LIB_PATH += ./lib64
else ifeq ($(Bits), -m32)
   LIB_PATH += ./lib32
else
	LIB_PATH += ./libs
endif

###输出程序路径
#INSTALL_APP_PATH := ./bin

include common.mk


