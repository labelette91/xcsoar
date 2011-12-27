ifeq ($(WINHOST),y)
  IM_PREFIX := im-
else
  IM_PREFIX :=
endif

####### market icons

SVG_MARKET_ICONS = Data/graphics/xcsoarswiftsplash.svg Data/graphics/xcsoarswiftsplash_red.svg
PNG_MARKET_ICONS = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_market.png,$(SVG_MARKET_ICONS))

market-icons: $(PNG_MARKET_ICONS)

# render from SVG to PNG
$(PNG_MARKET_ICONS): $(DATA)/graphics/%_market.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=512 $< -o $@

####### icons

SVG_ICONS = $(wildcard Data/icons/*.svg)
SVG_NOALIAS_ICONS = $(patsubst Data/icons/%.svg,$(DATA)/icons/%.svg,$(SVG_ICONS))
PNG_ICONS = $(patsubst Data/icons/%.svg,$(DATA)/icons/%.png,$(SVG_ICONS))
BMP_ICONS = $(PNG_ICONS:.png=.bmp)
PNG_ICONS_160 = $(patsubst Data/icons/%.svg,$(DATA)/icons/%_160.png,$(SVG_ICONS))
BMP_ICONS_160 = $(PNG_ICONS_160:.png=.bmp)

# modify working copy of SVG to improve rendering
$(SVG_NOALIAS_ICONS): $(DATA)/icons/%.svg: build/svg_preprocess.xsl Data/icons/%.svg | $(DATA)/icons/dirstamp
	@$(NQ)echo "  XSLT    $@"
	$(Q)xsltproc --stringparam DisableAA_Select "MASK_NOAA_" --output $@ $^

# render from SVG to PNG
# Default 100PPI (eg 320x240 4" display)
$(PNG_ICONS): $(DATA)/icons/%.png: $(DATA)/icons/%.svg | $(DATA)/icons/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --x-zoom=1.0 --y-zoom=1.0 $< -o $@

#160PPI (eg 640x480 5" display)
$(PNG_ICONS_160): $(DATA)/icons/%_160.png: $(DATA)/icons/%.svg | $(DATA)/icons/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --x-zoom=1.6316 --y-zoom=1.6316 $< -o $@

# extract alpha channel
%_alpha.png: %.png
	$(Q)$(IM_PREFIX)convert $< -alpha Extract +matte +dither -colors 8 $@

# extract RGB channels
%_rgb.png: %.png
	$(Q)$(IM_PREFIX)convert $< -background white -flatten +matte +dither -colors 64 $@

# tile both images
%_tile.png: %_alpha.png %_rgb.png
	$(Q)$(IM_PREFIX)montage -tile 2x1 -geometry +0+0 $^ -depth 8 $@

# convert to uncompressed 8-bit BMP
$(BMP_ICONS): %.bmp: %_tile.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< +dither -compress none -type optimize -colors 256 $@
$(BMP_ICONS_160): %.bmp: %_tile.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< +dither -compress none -type optimize -colors 256 $@

####### splash logo

SVG_SPLASH = Data/graphics/xcsoarswiftsplash.svg Data/graphics/xcsoarswiftsplash_red.svg
PNG_SPLASH_160 = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_160.png,$(SVG_SPLASH))
BMP_SPLASH_160 = $(PNG_SPLASH_160:.png=.bmp)
PNG_SPLASH_80 = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_80.png,$(SVG_SPLASH))
BMP_SPLASH_80 = $(PNG_SPLASH_80:.png=.bmp)
PNG_SPLASH_128 = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_128.png,$(SVG_SPLASH))
ICNS_SPLASH_128 = $(PNG_SPLASH_128:.png=.icns)

# render from SVG to PNG
$(PNG_SPLASH_160): $(DATA)/graphics/%_160.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=160 $< -o $@
$(PNG_SPLASH_80): $(DATA)/graphics/%_80.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=80 $< -o $@
$(PNG_SPLASH_128): $(DATA)/graphics/%_128.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=128 $< -o $@

# convert to uncompressed 8-bit BMP
$(BMP_SPLASH_160): %.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 $@
$(BMP_SPLASH_80): %.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 $@

# convert to icns (mac os x icon)
$(ICNS_SPLASH_128): %.icns: %.png
	@$(NQ)echo "  ICNS    $@"
	$(Q)$(IM_PREFIX)png2icns $@ $<

####### version

SVG_TITLE = Data/graphics/title.svg Data/graphics/title_red.svg
PNG_TITLE_110 = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_110.png,$(SVG_TITLE))
BMP_TITLE_110 = $(PNG_TITLE_110:.png=.bmp)
PNG_TITLE_320 = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_320.png,$(SVG_TITLE))
BMP_TITLE_320 = $(PNG_TITLE_320:.png=.bmp)

# render from SVG to PNG
$(PNG_TITLE_110): $(DATA)/graphics/%_110.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=110 $< -o $@

# convert to uncompressed 8-bit BMP
$(BMP_TITLE_110): %.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 $@

# render from SVG to PNG
$(PNG_TITLE_320): $(DATA)/graphics/%_320.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=320 $< -o $@

# convert to uncompressed 8-bit BMP
$(BMP_TITLE_320): %.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 $@

####### dialog title

SVG_DIALOG_TITLE = Data/graphics/dialog_title.svg Data/graphics/dialog_title_red.svg
PNG_DIALOG_TITLE = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%.png,$(SVG_DIALOG_TITLE))
BMP_DIALOG_TITLE = $(PNG_DIALOG_TITLE:.png=.bmp)

# render from SVG to PNG
$(PNG_DIALOG_TITLE): $(DATA)/graphics/%.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert $< -o $@

# convert to uncompressed 8-bit BMP
$(BMP_DIALOG_TITLE): %.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 $@

####### progress bar border

SVG_PROGRESS_BORDER = Data/graphics/progress_border.svg Data/graphics/progress_border_red.svg
PNG_PROGRESS_BORDER = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%.png,$(SVG_PROGRESS_BORDER))
BMP_PROGRESS_BORDER = $(PNG_PROGRESS_BORDER:.png=.bmp)

# render from SVG to PNG
$(PNG_PROGRESS_BORDER): $(DATA)/graphics/%.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert $< -o $@

# convert to uncompressed 8-bit BMP
$(BMP_PROGRESS_BORDER): %.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 $@

####### launcher graphics

SVG_LAUNCH = Data/graphics/launcher.svg Data/graphics/launcher_red.svg
PNG_LAUNCH_224 = $(patsubst Data/graphics/%.svg,$(DATA)/graphics/%_224.png,$(SVG_LAUNCH))
BMP_LAUNCH_FLY_224 = $(PNG_LAUNCH_224:.png=_1.bmp)
BMP_LAUNCH_SIM_224 = $(PNG_LAUNCH_224:.png=_2.bmp)
BMP_LAUNCH_DLL_FLY_224 = $(PNG_LAUNCH_224:.png=_dll_1.bmp)
BMP_LAUNCH_DLL_SIM_224 = $(PNG_LAUNCH_224:.png=_dll_2.bmp)

# render from SVG to PNG
$(PNG_LAUNCH_224): $(DATA)/graphics/%_224.png: Data/graphics/%.svg | $(DATA)/graphics/dirstamp
	@$(NQ)echo "  SVG     $@"
	$(Q)rsvg-convert --width=224 $< -o $@

# split into two uncompressed 8-bit BMPs (single 'convert' operation)
$(BMP_LAUNCH_FLY_224): %_1.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	@$(NQ)echo "  BMP     $(@:1.bmp=2.bmp)"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 -crop '50%x100%' -scene 1 $(@:1.bmp=%d.bmp)
$(BMP_LAUNCH_SIM_224): %_2.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	@$(NQ)echo "  BMP     $(@:1.bmp=2.bmp)"
	$(Q)$(IM_PREFIX)convert $< -background white -layers flatten +matte +dither -compress none -type optimize -colors 256 -crop '50%x100%' -scene 1 $(@:1.bmp=%d.bmp)

# split into two uncompressed 8-bit BMPs (single 'convert' operation)
$(BMP_LAUNCH_DLL_FLY_224): %_dll_1.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	@$(NQ)echo "  BMP     $(@:1.bmp=2.bmp)"
	$(Q)$(IM_PREFIX)convert $< -background blue -layers flatten +matte +dither -compress none -type optimize -colors 256 -crop '50%x100%' -scene 1 $(@:1.bmp=%d.bmp)
$(BMP_LAUNCH_DLL_SIM_224): %_dll_2.bmp: %.png
	@$(NQ)echo "  BMP     $@"
	@$(NQ)echo "  BMP     $(@:1.bmp=2.bmp)"
	$(Q)$(IM_PREFIX)convert $< -background blue -layers flatten +matte +dither -compress none -type optimize -colors 256 -crop '50%x100%' -scene 1 $(@:1.bmp=%d.bmp)

#######

DIALOG_FILES = $(wildcard Data/Dialogs/*.xml)
DIALOG_FILES += $(wildcard Data/Dialogs/Infobox/*.xml)
DIALOG_FILES += $(wildcard Data/Dialogs/Configuration/*.xml)

DIALOG_COMPRESSED = $(patsubst Data/Dialogs/%.xml,$(DATA)/dialogs/%.xml.gz,$(DIALOG_FILES))
$(DIALOG_COMPRESSED): $(DATA)/dialogs/%.xml.gz: Data/Dialogs/%.xml \
	| $(DATA)/dialogs/Configuration/dirstamp $(DATA)/dialogs/Infobox/dirstamp
	@$(NQ)echo "  GZIP    $@"
	$(Q)gzip --best <$< >$@.tmp
	$(Q)mv $@.tmp $@

TEXT_FILES = AUTHORS COPYING

TEXT_COMPRESSED = $(patsubst %,$(DATA)/%.gz,$(TEXT_FILES))
$(TEXT_COMPRESSED): $(DATA)/%.gz: % | $(DATA)/dirstamp
	@$(NQ)echo "  GZIP    $@"
	$(Q)gzip --best <$< >$@.tmp
	$(Q)mv $@.tmp $@

RESOURCE_FILES = $(DIALOG_COMPRESSED) $(TEXT_COMPRESSED)

ifeq ($(TARGET),ANDROID)
RESOURCE_FILES += $(patsubst po/%.po,$(OUT)/po/%.mo,$(wildcard po/*.po))
else
RESOURCE_FILES += $(wildcard Data/bitmaps/*.bmp)
RESOURCE_FILES += $(BMP_ICONS) $(BMP_ICONS_160) 
RESOURCE_FILES += $(BMP_SPLASH_160) $(BMP_SPLASH_80)
RESOURCE_FILES += $(BMP_DIALOG_TITLE) $(BMP_PROGRESS_BORDER)
RESOURCE_FILES += $(BMP_TITLE_320) $(BMP_TITLE_110)
RESOURCE_FILES += $(BMP_LAUNCH_FLY_224) $(BMP_LAUNCH_SIM_224)
RESOURCE_FILES += $(BMP_LAUNCH_DLL_FLY_224) $(BMP_LAUNCH_DLL_SIM_224)
endif

ifeq ($(HAVE_WIN32),y)

RESOURCE_TEXT = Data/XCSoar.rc
RESOURCE_BINARY = $(TARGET_OUTPUT_DIR)/$(notdir $(RESOURCE_TEXT:.rc=.rsc))
RESOURCE_FILES += $(patsubst po/%.po,$(OUT)/po/%.mo,$(wildcard po/*.po))

$(RESOURCE_BINARY): $(RESOURCE_TEXT) $(RESOURCE_FILES) | $(TARGET_OUTPUT_DIR)/%/../dirstamp
	@$(NQ)echo "  WINDRES $@"
	$(Q)$(WINDRES) $(WINDRESFLAGS) -o $@ $<

else

# no resources on UNIX
RESOURCE_BINARY =

endif
