filterReads
===========

I am currently working on a (rather simple) program that filters short reads using a set of defined sequences which shall be ignored (the motivation behind this was the observation that some RNA-Seq samples have an obscene number of reads aligning to ribosomal DNA - which messes up Rcount-multireads as it results in a huge RAM requirement for the multireads). I uploaded an early version. The filter based on sequences seems to work. The other filters are not tested yet. Note that one requires the qtiocompressor to compile it (more info in future or on request). I recommend running this version in any case from the command line (to see what happens in the background if you are using the GUI).

[Binary for Ubuntu-like linux](https://github.com/MWSchmid/filterReads/blob/master/p502filterReads_linux_64bit?raw=true)

To start a GUI:

```shell
./p502filterReads
```

To run on command-line:

```shell
./p502filterReads -c arguments
# check the arguments with
./p502filterReads --help
```

**TODO**: it's currently assuming PHRED 64 quality scores. Change this to PHRED 33 (all data on SRA is generally in 33).
