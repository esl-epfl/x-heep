## boot_rom

1. If you haven't done it yet, install [Conda](https://phoenixnap.com/kb/how-to-install-anaconda-ubuntu-18-04-or-20-04) as described in the link,
and create the Conda enviroment with python 2.7:

```bash
conda update conda
conda env create -f environment.yml
```

Activate the environment with

```bash
conda activate boot_rom
```

If you are already in the core-v-mini-mcu conda env, deactivate it first:

```bash
conda deactivate
```

2. Install the required Python tools:

```
pip install --user -r python-requirements.txt
```

3. Generate the boot_rom:

If you modified the `boot_rom.S` file, generate it as:

```
make all
```

4. Verible:

Go back to the top folder and run verible
