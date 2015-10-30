# networkRobustnessToolbox

This toolbox contains tools needed to find the robustness of metabolic
networks. I have written this toolbox during my PhD time. 

## Table of Contents

[Installation](#Installation)

[Examples](#Examples)

[Calculate robustness](#calculate robustness)

* failureProbabilityByMcs
* recalcFailureProbability
* robustnessByEfms
* overallRobustnessByEfms

[Additional tools](#additional tools)

* convertFailureProbOut2csv.pl 

## <a name="Installation"></a>Installation

C-programs can be compiled by 

```
cd robustToolbox
make
```

Perl scripts are located in folder scripts and can be executed without
compilation.

## <a name="Examples"></a>Examples

Examples for all programs and scripts are located in the examples folder.

## <a name="calculate robustness"></a>Calculate robustness

This section describes tools that calculates the robustness of metabolic
networks. Help for all C tools is called if the program is started without any
parameter.

**failureProbabilityByMcs**

```
This C tool calculates the failure probability by a given minimal cutsets. The
provided file needs to be in following format.
```

**recalcFailureProbability**

```
This C tool recalculates the failure probability with a different lambda value
using a previously calculated result by failureProbabilityByMcs.
```

**robustnessByEfms**

```
This C tool calculates the robustness by a given set of EFMs, as described by
Behre et al., 2008
```

**robustnessByEfms**

```
This C tool calculates the the overall robustness by a given set of EFMs, as
described by Behre et al., 2008
```

## <a name="additional tools"></a>Additional tools

This section describes further tools needed for calculation of robustness

**convertFailureProbOut2csv.pl**

```
This perl script converts the formatted output of failureProbabilityByMcs and
recalcFailureProbability to csv format
```
