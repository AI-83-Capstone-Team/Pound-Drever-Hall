SIM    ?= verilator
PYTHON ?= python3

.PHONY: all clean

all:
	$(PYTHON) run.py --sim $(SIM)

clean:
	rm -rf build/ __pycache__ results.xml *.vcd sine_qtr_4096_16b.mem
