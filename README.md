# SeqEyes
Display Pulseq sequence diagram and k-space trajectory, modified from [PulseqViewer](https://github.com/xpjiang/PulseqViewer)

![image](./doc/ui.png)

## Install
Download the compiled windows exe from [github releases](https://github.com/xingwangyong/seqeyes/releases), or from [artifacts in github actions](https://github.com/xingwangyong/seqeyes/actions). The latter is more frequently updated.

## Usage
- Open GUI, load .seq file
- Use the command line interface 
```bash
seqeyes filename.seq
```
for more options, see `seqeyes --help`
- Use the matlab wrapper `seqeyes.m`
```matlab
seqeyes('path/to/sequence.seq');
```
or
```matlab
seqeyes(seq);
```

## Build Instructions
Qt6 libraries and cmake are required to build the project.
### Linux
Use the build.sh script to build the project.
### Windows
```
cmake -S . -B out/build/x64-Release
cmake --build out/build/x64-Release --config Release
```
After compilation, run the following command to deploy Qt libraries:
```bash
C:\Qt\6.5.3\msvc2019_64\bin\windeployqt.exe .\seqeyes.exe
```

**Note**: Please use the full path to run windeployqt.exe, as the system may have multiple versions of Qt installed.

## Python Package

SeqEyes also provides a Python package (`seqeyes`) that exposes the Pulseq reader as a native extension module built with [pybind11](https://pybind11.readthedocs.io). It has no Qt dependency — only NumPy is required at runtime.

### Install

```bash
pip install seqeyes
```

Or install directly from the repository:

```bash
pip install .
```

Requirements: Python ≥ 3.9, NumPy, a C++17 compiler, and CMake ≥ 3.20.

### Usage

```python
from seqeyes import Sequence, read_version

# Read only the version header (fast, no full parse)
major, minor = read_version("path/to/sequence.seq")
print(f"Pulseq v{major}.{minor}")

# Load a full sequence
seq = Sequence("path/to/sequence.seq")
print(f"{seq.num_blocks} blocks, version {seq.version}")
print(seq.definitions)          # dict from [DEFINITIONS] section

# Decode a block
blk = seq.get_block(0)
print(f"Duration: {blk.duration_us} µs")

if blk.rf_amp is not None:      # RF event present
    print("RF amplitude (Hz):", blk.rf_amp)

if blk.gx_wave is not None:     # Gx gradient present
    print("Gx waveform (Hz/m):", blk.gx_wave)

if blk.adc_num_samples is not None:   # ADC event present
    print(f"ADC: {blk.adc_num_samples} samples, dwell {blk.adc_dwell} ns")
```

### Run Tests

```bash
pip install ".[test]"
pytest python/tests/
```

## Known Issues

Please see [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for a list of known issues and limitations.



