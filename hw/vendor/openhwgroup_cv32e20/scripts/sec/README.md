# Sequential Equivalence Checking (SEC)

This folder contains a SEC script that runs on
Cadence Jaspergold

### Running the script

From a bash shell, please execute:

```
./sec.sh -t cadence
```

Use `sh ./sec.sh -t {cadence}` if you run it from a tcsh shell.

The script clones the `cve2` `main` branch of the core as a golden reference, and uses the current repository's `rtl` as revised version.

If you want to use another golden reference rtl, Set the `GOLDEN_RTL` enviromental variable to the new rtl before calling the `sec.sh` script.

```
export GOLDEN_RTL=YOUR_GOLDEN_CORE_RTL_PATH
```
or

```
setenv GOLDEN_RTL YOUR_GOLDEN_CORE_RTL_PATH
```
If the script succeeds, it returns 0, otherwise 1.

The `sec.tcl` scripts in the tool specific folders are executed on the tools to perform `RTL to RTL` SEC.
