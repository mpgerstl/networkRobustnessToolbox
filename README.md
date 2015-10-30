# networkRobustnessToolbox
This toolbox contains tools used for defining the robustness of metabolic
networks.

## NOTE
This software was developed and tested only with Linux. However it is pure C
code.  So it should also work with other operating system.

## INSTALL
1. download networkRobustnessToolbox
2. cd networkRobustnessToolbox
3. make

compiled tools are located in folder bin

Perl scripts are located in folder script and can just be started without the
need of installation.

## USAGE
* Examples are located in the folder examples
```
cd examples
sh run_examples.sh
```
To remove output files
```
sh clean_example_files.sh
```
   
* Every tool can be called without any argument to show help page
* A short documentation can be found in the folder doc
* If you use any software of this toolbox please cite: 

> Matthias P. Gerstl, Steffen Klamt, Christian Jungreuthmayer, and Jürgen Zanghellini.
> Exact quantification of cellular robustness in genome-scale metabolic networks. 
> Bioinformatics (In Press)

## LICENSE
This toolbox is published under GNU Public License V3. The license information
is contained in every src file.

