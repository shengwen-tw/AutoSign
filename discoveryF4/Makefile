all: demos

libstm_build:
	$(MAKE) -C libstm/STM32F4xx_StdPeriph_Driver/build

demos:  libstm_build
	$(MAKE) -C serialUSB

clean:
	$(MAKE) -C serialUSB clean
	$(MAKE) -C libstm/STM32F4xx_StdPeriph_Driver/build clean
