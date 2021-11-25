# A Side-Channel Resistant Implementation of SABER

This repository contains ARM Cortex-M4 code for a first-order masked implementation of [SABER](https://github.com/KULeuven-COSIC/SABER). The implementation is described in our paper "A Side-Channel-Resistant Implementation of SABER" [[ePrint]](https://eprint.iacr.org/2020/733.pdf) that appeared in ACM Journal on Emerging Technologies in Computing Systems (JETC), Volume 17, Issue 2 [[ACM]](https://dl.acm.org/doi/abs/10.1145/3429983).

## Sources

* [saber/m4](./src/saber/m4) : SABER reference implementation files
* [saber/m4_masking](./src/saber/m4_masking): additional SABER masking implementation files

## Requirements

The code in this repository includes the [pqm4](https://github.com/mupq/pqm4) framework for testing and benchmarking on the [STM32F4 Discovery board](https://www.st.com/en/evaluation-tools/stm32f4discovery.html). We refer to the documentation of [pqm4](https://github.com/mupq/pqm4) for the required prerequisites on Setup/Installation.

After cloning or downloading this repository, it is necessary to initialize pqm4:

```bash
git submodule update --init --recursive
```

## Running Benchmarks and Tests

Before proceeding with the benchmarks and tests, the masked implementation should be added to [pqm4](https://github.com/mupq/pqm4):

```bash
ln -rs ./src/saber/m4_masking ./pqm4/crypto_kem/saber/
```

Subsequently, apply the following patches to make pqm4 work with the masked Saber API:

```bash
cd pqm4 && git apply ../pqm4.patch
cd mupq && git apply ../../mupq.patch
```

All masked Saber tests tests can be run using:

```bash
cd pqm4
[sudo -E] python3 test.py saber
[sudo -E] python3 testvectors.py saber
```

Similarly, all masked Saber benchmarks can be run using:

```bash
cd pqm4
[sudo -E] python3 benchmarks.py saber
```

The repository also includes an [example git patch](./benchmark-A2B-example.patch) on how to easily do more fine-grained benchmarking by (ab)using pqm4's `PROFILE_HASHING`.

Benchmarks can then be found in the [benchmarks](./pqm4/benchmarks/) folder.

## Bibliography

If you use or build upon the code in this repository, please cite our paper using our [citation key](./bib).
