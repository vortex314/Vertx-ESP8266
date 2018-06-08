#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
# COMPONENT_ADD_INCLUDEDIRS=.
# CXXFLAGS +="-DESP32_IDF=1"
# Component makefile for extras/ArduinoJson

# expected anyone using bmp driver includes it as 'ArduinoJson/MQTT*.h'
INC_DIRS += $(ArduinoJson_ROOT)..  $(ArduinoJson_ROOT)

# args for passing into compile rule generation
ArduinoJson_SRC_DIR =  $(ArduinoJson_ROOT)

$(eval $(call component_compile_rules,ArduinoJson))
