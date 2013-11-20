#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/src/camera/stonyman.o ${OBJECTDIR}/src/fatfs/ff.o ${OBJECTDIR}/src/fatfs/mmcPIC32.o ${OBJECTDIR}/src/Peripheral_Function/adc.o ${OBJECTDIR}/src/Peripheral_Function/uart.o ${OBJECTDIR}/src/sys_func.o ${OBJECTDIR}/src/interrupts.o ${OBJECTDIR}/src/sd_test.o ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/predict_gaze.o
POSSIBLE_DEPFILES=${OBJECTDIR}/src/camera/stonyman.o.d ${OBJECTDIR}/src/fatfs/ff.o.d ${OBJECTDIR}/src/fatfs/mmcPIC32.o.d ${OBJECTDIR}/src/Peripheral_Function/adc.o.d ${OBJECTDIR}/src/Peripheral_Function/uart.o.d ${OBJECTDIR}/src/sys_func.o.d ${OBJECTDIR}/src/interrupts.o.d ${OBJECTDIR}/src/sd_test.o.d ${OBJECTDIR}/src/main.o.d ${OBJECTDIR}/src/predict_gaze.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/src/camera/stonyman.o ${OBJECTDIR}/src/fatfs/ff.o ${OBJECTDIR}/src/fatfs/mmcPIC32.o ${OBJECTDIR}/src/Peripheral_Function/adc.o ${OBJECTDIR}/src/Peripheral_Function/uart.o ${OBJECTDIR}/src/sys_func.o ${OBJECTDIR}/src/interrupts.o ${OBJECTDIR}/src/sd_test.o ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/predict_gaze.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE} ${MAKE_OPTIONS} -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX795F512L
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/camera/stonyman.o: src/camera/stonyman.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/camera 
	@${RM} ${OBJECTDIR}/src/camera/stonyman.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/camera/stonyman.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/camera/stonyman.o.d" -o ${OBJECTDIR}/src/camera/stonyman.o src/camera/stonyman.c   
	
${OBJECTDIR}/src/fatfs/ff.o: src/fatfs/ff.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/fatfs 
	@${RM} ${OBJECTDIR}/src/fatfs/ff.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/fatfs/ff.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/fatfs/ff.o.d" -o ${OBJECTDIR}/src/fatfs/ff.o src/fatfs/ff.c   
	
${OBJECTDIR}/src/fatfs/mmcPIC32.o: src/fatfs/mmcPIC32.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/fatfs 
	@${RM} ${OBJECTDIR}/src/fatfs/mmcPIC32.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/fatfs/mmcPIC32.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/fatfs/mmcPIC32.o.d" -o ${OBJECTDIR}/src/fatfs/mmcPIC32.o src/fatfs/mmcPIC32.c   
	
${OBJECTDIR}/src/Peripheral_Function/adc.o: src/Peripheral_Function/adc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/Peripheral_Function 
	@${RM} ${OBJECTDIR}/src/Peripheral_Function/adc.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/Peripheral_Function/adc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/Peripheral_Function/adc.o.d" -o ${OBJECTDIR}/src/Peripheral_Function/adc.o src/Peripheral_Function/adc.c   
	
${OBJECTDIR}/src/Peripheral_Function/uart.o: src/Peripheral_Function/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/Peripheral_Function 
	@${RM} ${OBJECTDIR}/src/Peripheral_Function/uart.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/Peripheral_Function/uart.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/Peripheral_Function/uart.o.d" -o ${OBJECTDIR}/src/Peripheral_Function/uart.o src/Peripheral_Function/uart.c   
	
${OBJECTDIR}/src/sys_func.o: src/sys_func.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/sys_func.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/sys_func.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/sys_func.o.d" -o ${OBJECTDIR}/src/sys_func.o src/sys_func.c   
	
${OBJECTDIR}/src/interrupts.o: src/interrupts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/interrupts.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/interrupts.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/interrupts.o.d" -o ${OBJECTDIR}/src/interrupts.o src/interrupts.c   
	
${OBJECTDIR}/src/sd_test.o: src/sd_test.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/sd_test.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/sd_test.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/sd_test.o.d" -o ${OBJECTDIR}/src/sd_test.o src/sd_test.c   
	
${OBJECTDIR}/src/main.o: src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/main.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/main.o.d" -o ${OBJECTDIR}/src/main.o src/main.c   
	
${OBJECTDIR}/src/predict_gaze.o: src/predict_gaze.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/predict_gaze.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/predict_gaze.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/predict_gaze.o.d" -o ${OBJECTDIR}/src/predict_gaze.o src/predict_gaze.c   
	
else
${OBJECTDIR}/src/camera/stonyman.o: src/camera/stonyman.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/camera 
	@${RM} ${OBJECTDIR}/src/camera/stonyman.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/camera/stonyman.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/camera/stonyman.o.d" -o ${OBJECTDIR}/src/camera/stonyman.o src/camera/stonyman.c   
	
${OBJECTDIR}/src/fatfs/ff.o: src/fatfs/ff.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/fatfs 
	@${RM} ${OBJECTDIR}/src/fatfs/ff.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/fatfs/ff.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/fatfs/ff.o.d" -o ${OBJECTDIR}/src/fatfs/ff.o src/fatfs/ff.c   
	
${OBJECTDIR}/src/fatfs/mmcPIC32.o: src/fatfs/mmcPIC32.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/fatfs 
	@${RM} ${OBJECTDIR}/src/fatfs/mmcPIC32.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/fatfs/mmcPIC32.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/fatfs/mmcPIC32.o.d" -o ${OBJECTDIR}/src/fatfs/mmcPIC32.o src/fatfs/mmcPIC32.c   
	
${OBJECTDIR}/src/Peripheral_Function/adc.o: src/Peripheral_Function/adc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/Peripheral_Function 
	@${RM} ${OBJECTDIR}/src/Peripheral_Function/adc.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/Peripheral_Function/adc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/Peripheral_Function/adc.o.d" -o ${OBJECTDIR}/src/Peripheral_Function/adc.o src/Peripheral_Function/adc.c   
	
${OBJECTDIR}/src/Peripheral_Function/uart.o: src/Peripheral_Function/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src/Peripheral_Function 
	@${RM} ${OBJECTDIR}/src/Peripheral_Function/uart.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/Peripheral_Function/uart.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/Peripheral_Function/uart.o.d" -o ${OBJECTDIR}/src/Peripheral_Function/uart.o src/Peripheral_Function/uart.c   
	
${OBJECTDIR}/src/sys_func.o: src/sys_func.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/sys_func.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/sys_func.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/sys_func.o.d" -o ${OBJECTDIR}/src/sys_func.o src/sys_func.c   
	
${OBJECTDIR}/src/interrupts.o: src/interrupts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/interrupts.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/interrupts.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/interrupts.o.d" -o ${OBJECTDIR}/src/interrupts.o src/interrupts.c   
	
${OBJECTDIR}/src/sd_test.o: src/sd_test.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/sd_test.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/sd_test.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/sd_test.o.d" -o ${OBJECTDIR}/src/sd_test.o src/sd_test.c   
	
${OBJECTDIR}/src/main.o: src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/main.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/main.o.d" -o ${OBJECTDIR}/src/main.o src/main.c   
	
${OBJECTDIR}/src/predict_gaze.o: src/predict_gaze.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/predict_gaze.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/predict_gaze.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/src/predict_gaze.o.d" -o ${OBJECTDIR}/src/predict_gaze.o src/predict_gaze.c   
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/Eye_Glasses_serial.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
