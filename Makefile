# Whack A Mole Attack -- build and lint targets.
#
# Requires arduino-cli with the Teensy core installed:
#   brew install arduino-cli
#   arduino-cli core install teensy:avr --additional-urls \
#     https://www.pjrc.com/teensy/package_teensy_index.json
#
# Optional lint tools:
#   brew install clang-format llvm ruff

FQBN    := teensy:avr:teensy41
SKETCH  := firmware/WhackAMole
BUILD   := build
CHROME  := /Applications/Google Chrome.app/Contents/MacOS/Google Chrome
SOURCES := $(wildcard firmware/WhackAMole/*.cpp firmware/WhackAMole/*.h) $(SKETCH)/WhackAMole.ino

.PHONY: all build matrix upload format format-check tidy lint-py lint stl wiring-pdf clean

all: build

## Compile the default hardware configuration.
build:
	arduino-cli compile --warnings all --fqbn $(FQBN) --build-path $(BUILD) $(SKETCH)

## Compile every combination of input and lamp hardware. This is what keeps the
## resistor-ladder path from bit-rotting while the GPIO rewire is in progress,
## and vice versa.
##
## The Teensy platform.txt has no build.extra_flags hook -- its recipes only
## interpolate build.flags.defs -- so the base value is read back and appended
## to rather than replaced. Overriding a property the platform does not
## reference is silently ignored, which makes every variant build the default
## config and pass for the wrong reason.
matrix:
	@defs=`arduino-cli board details -b $(FQBN) --show-properties | sed -n 's/^build\.flags\.defs=//p'`; \
	if [ -z "$$defs" ]; then echo "could not read build.flags.defs for $(FQBN)"; exit 1; fi; \
	rc=0; \
	for gpio in 0 1; do \
	  for pixels in 0 1; do \
	    printf 'WAM_INPUT_GPIO=%s WAM_LIGHTS_PIXELS=%s ... ' $$gpio $$pixels; \
	    if arduino-cli compile --clean --warnings all --fqbn $(FQBN) \
	        --build-property "build.flags.defs=$$defs -DWAM_INPUT_GPIO=$$gpio -DWAM_LIGHTS_PIXELS=$$pixels" \
	        $(SKETCH) > /tmp/wam-build.log 2>&1; then \
	      echo OK; \
	    else \
	      echo FAIL; cat /tmp/wam-build.log; rc=1; \
	    fi; \
	  done; \
	done; \
	exit $$rc

## Compile and flash a connected Teensy.
upload: build
	arduino-cli upload --fqbn $(FQBN) --input-dir $(BUILD) $(SKETCH)

format:
	clang-format -i $(SOURCES)

format-check:
	clang-format --dry-run --Werror $(SOURCES)

## clang-tidy needs a compile database; arduino-cli emits one into the build dir.
tidy:
	arduino-cli compile --fqbn $(FQBN) --build-path $(BUILD) --only-compilation-database $(SKETCH)
	clang-tidy -p $(BUILD) firmware/WhackAMole/*.cpp

lint-py:
	ruff check display/
	ruff format --check display/

lint: format-check lint-py

## Render the printed parts. Needs OpenSCAD: `brew install --cask openscad@snapshot`
## (the stable cask is currently disabled). The .scad asserts on the measured
## dimensions, so a failed render here means a part that would not fit.
stl:
	mkdir -p $(BUILD)/stl
	openscad -D 'part="seat"' -o $(BUILD)/stl/button-ring-seat.stl hardware/button-led-carrier.scad
	openscad -D 'part="plug"' -o $(BUILD)/stl/button-bore-plug.stl hardware/button-led-carrier.scad

## Render docs/wiring.html to a PDF. Needs Google Chrome.
## Chrome ignores --print-to-pdf-no-header; --no-pdf-header-footer is the flag
## that actually suppresses the date/title/URL stamps.
wiring-pdf:
	python3 docs/make-print-html.py docs/wiring.html $(BUILD)/wiring-print.html
	"$(CHROME)" --headless --disable-gpu --no-sandbox \
	  --virtual-time-budget=12000 --no-pdf-header-footer \
	  --print-to-pdf="$(PWD)/docs/mole-table-wiring.pdf" \
	  "file://$(PWD)/$(BUILD)/wiring-print.html" 2>/dev/null || true
	@test -f docs/mole-table-wiring.pdf && echo "docs/mole-table-wiring.pdf" || \
	  (echo "PDF not produced -- is Chrome at $(CHROME)?" && exit 1)

clean:
	rm -rf $(BUILD)
