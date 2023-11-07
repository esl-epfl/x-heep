# Update the documentation

1. If you need to create a new entry, add the new document in markdown (`.md` extension) to the corresponding folder.
> Make sure the document has one single `# header`, otherwise they will be considered different documents.
2. If a new folder is added, add it to the `toctree` inside `docs/source/index.rst` (as the `peripherals` folder is)
3. Commit and push
4. Open a terminal in the docs folder and run
```bash
make clean html
```
5. Wait a few minutes and enjoy your brand new documentation in Read the Docs.
