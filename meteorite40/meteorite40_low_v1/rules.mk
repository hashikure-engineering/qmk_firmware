# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
MOUSEKEY_ENABLE = yes       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = no            # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = no       # Enable keyboard RGB underglow
AUDIO_ENABLE = no           # Audio output
SPLIT_KEYBOARD = no
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = pmw3360
#ENCODER_ENABLE = yes
#ENCODER_MAP_ENABLE = yes
LTO_ENABLE = yes

ifdef DEBUG
	CONSOLE_ENABLE = yes         # Console for debug
endif

# Include common library
SRC += lib/meteorite40.c
