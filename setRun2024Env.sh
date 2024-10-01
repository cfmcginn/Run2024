#!/bin/bash

#check script is sourced so RUN2024DIR is properly set in environment
if [[ $0 == $BASH_SOURCE ]]
then
    echo "Usage: source setRUN2024Env.sh. do not use 'bash setRUN2024Env.sh'. exit 1"
    exit 1
fi

CURRRUN2024DIR=${RUN2024DIR}
RUN2024DIRTOSET=$PWD

#For Debug, uncomment  below
#echo "CURRRUN2024DIR: '$CURRRUN2024DIR'"
#echo "RUN2024DIRTOSET: '$RUN2024DIRTOSET'"

#Check that you are sourcing from top level
if [[ $RUN2024DIRTOSET != *"/Run2024" ]]
then
    echo "Please 'source setRUN2024env.sh' from top level of Run2024 repo - RUN2024DIR environment var not set. return 1"
    return 1    
fi

#Check RUN2024DIR already set
if [[ $CURRRUN2024DIR == $RUN2024DIRTOSET ]]
then
    echo "RUN2024DIR '$RUN2024DIRTOSET' already set. return 1"
    return 1
fi

echo "Setting environment var RUN2024DIR='$RUN2024DIRTOSET'"

export RUN2024DIR=$RUN2024DIRTOSET
