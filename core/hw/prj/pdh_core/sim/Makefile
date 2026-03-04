SIM ?= verilator

.PHONY: all clean

all:
	python3 run.py --sim $(SIM)

clean:
	rm -rf sim_build __pycache__ results.xml *.vcd sine_qtr_4096_16b.mem
