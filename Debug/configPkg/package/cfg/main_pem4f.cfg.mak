# invoke SourceDir generated makefile for main.pem4f
main.pem4f: .libraries,main.pem4f
.libraries,main.pem4f: package/cfg/main_pem4f.xdl
	$(MAKE) -f C:\dev\workspace_v10\TI_RTOS_IoT_GestureBasedWeatherTracking/src/makefile.libs

clean::
	$(MAKE) -f C:\dev\workspace_v10\TI_RTOS_IoT_GestureBasedWeatherTracking/src/makefile.libs clean

