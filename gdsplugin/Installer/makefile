INSTALLER = GDSDjVuIndexerSetup-1.0.exe
DLL = DjVuIndexer.dll

all: $(INSTALLER)

$(INSTALLER): $(DLL) "Setup.nsi"
	@"C:\Program Files\NSIS\makensis.exe" "Setup.nsi"

$(DLL): "..\Release\$(DLL)"
	@copy /b /y "..\Release\$(DLL)" . > nul
	@upx --best --force $(DLL)
