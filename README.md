# Stock Price Value Simulation

This project serves as a tool for visualization and Simulation
of stock price valuation using different stochastic processes like geometric brownian motion, black scholes and monte carlo.

## Requirements:

This project relies on qt5 and cmake in order to run.

for Debian based distributions:

```bash
sudo apt-get install build-essential libgl1-mesa-dev cmake
```

for Fedora/RHEL/Centos:

```bash
sudo yum groupinstall "C Development Tools and Libraries"
sudo yum install mesa-libGL-devel
```

for OpenSUSE
```bash
sudo zypper install -t pattern devel_basis
```

You can also build it from source by looking at the qt5 documentation.

## Compiling the code:

1. Clone the repository
2. Enter the base directory
```bash
mkdir -p build
```
4. Build the project
```bash
cmake -B build --parallel
```
