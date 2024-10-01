# Run2024
Misc. scripting for HI LHC Run 2024

Recommend cmsenv in CMSSW_13_2_6_patch2 to ensure matched compiler/environment

To setup the area
```
source setRun2024Env.sh
```

To build
```
make
```
To clean
```
make clean
```

To create histograms for turnon
```
#insert your input list of files and modify tag as appropriate
#input should have dfinder and l1 tree

./bin/dTurnOn.exe input/dTurnOn/fileList_jetSeed1p5.txt JetSeed1p5
#will output a file w/ the tag to output dir
#above -> 'output/dTurnOn_JetSeed1p5.root'
```

To create plots
```
./bin/dTurnOnPlot.exe output/dTurnOn_JetSeed1p5.root
#will create plots under pdfDir
#above -> 'pdfDir/dTurnOn_JetSeed1p5.pdf'
```