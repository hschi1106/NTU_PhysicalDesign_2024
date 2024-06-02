#!/bin/bash

#############################################################################
# File       [ evaluator.sh ]
# Author     [ FrankChiang ]
# Synopsis   [ To evaluate the score with pd23 ]
# Usage      [ bash evaluator.sh <input.aux> <HPWL> <runtime> ]
# Date       [ Ver. 1.0 started 2023/05/06 ]
#############################################################################

HPWL=0
RUNTIME=0
### check usage
if [ $# -eq 3 ]; then
    HPWL=$2
    RUNTIME=$3
else
    echo "[Eval] usage1: bash evaluator.sh <input.aux> <HPWL> <runtime>"
    exit 1
fi

### check files and runtime
# check files
BASEDIR=$(dirname "$0")
FILE_LIST=("$BASEDIR/pd23_results.list")
for FILE_NAME in ${FILE_LIST[@]}; do
    if [ ! -f $FILE_NAME ]; then
        echo "[Eval] error: Please check file '$FILE_NAME' exist."
        echo "(ps. pd23_result.list should be in the same dir with evaluator.sh)"
        exit 1
    fi
done
# check HPWL and RUNTIME
re='^([0-9]+.?[0-9]+?)$'
if [[ ! $HPWL =~ $re ]] ; then
    echo "[Eval] error: Given HPWL is not legal"; exit 1
fi
if [[ ! $RUNTIME =~ $re ]] ; then
    echo "[Eval] error: Given RUNTIME is not legal"; exit 1
fi

echo "[Eval] Your HPWL: $HPWL"
echo "[Eval] Your runtime: $RUNTIME"

### evaluater
## make table
CASENAME=$(basename "$1" .aux)
if ! grep -Fxq "$CASENAME" "${FILE_LIST[0]}"; then
    echo "[Eval] No case named '$CASENAME'"
    exit 1
fi
HPWL_LIST=(`grep -A2 -P "^$CASENAME$" ${FILE_LIST[0]} | sed -n '2 p'`)
RUNTIME_LIST=(`grep -A2 -P "^$CASENAME$" ${FILE_LIST[0]} | sed -n '3 p'`)

HPWL_MIN=${HPWL_LIST[0]}
HPWL_MAX=${HPWL_LIST[-1]}

## evaluate
TOP_SCORE=9.5
BOT_SCORE=4
QUALITY_SCORE=0
RUNTIME_SCORE=0

# quality
TOTAL_SCORE_DIFF=`echo $TOP_SCORE-$BOT_SCORE | bc`
SCORE_DIFF=`awk "BEGIN {print $TOTAL_SCORE_DIFF/${#HPWL_LIST[@]}}"`
SCORE1=`echo $TOP_SCORE+$SCORE_DIFF | bc`
SCORE2=$TOP_SCORE
for idx in ${!HPWL_LIST[@]}; do
    if (( $(echo "${HPWL_LIST[$idx]} >= $HPWL" |bc -l) )); then
        if [ $idx -eq 0 ]; then
            # case1: HPWL < HPWL_list[0]
            SCORE1=$TOP_SCORE
            SCORE2=`echo $TOP_SCORE-$SCORE_DIFF | bc`
            idx2=$(($idx+2))
            SCORE_DIFF=`awk "BEGIN {print $TOTAL_SCORE_DIFF/${#HPWL_LIST[@]}*2}"`
            # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
            X_DIFF=`echo ${HPWL_LIST[$idx]}-$HPWL | bc`
            X_TOTAL_DIFF=`echo ${HPWL_LIST[$idx2]}-${HPWL_LIST[$idx]} | bc`
            if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l) )); then
                X_TOTAL_DIFF=1
            fi
            Y_DIFF=$SCORE_DIFF
            X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
            Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
            QUALITY_SCORE=`echo $SCORE1+$Y_RATIO | bc`
            #echo "[Eval] QUALITY_SCORE = $SCORE1 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
            break
        fi
        # case 2: HPWL_list[0] < HPWL < HPWL_list[n-1]
        pre_idx=$(($idx-1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo ${HPWL_LIST[$idx]}-$HPWL | bc`
        X_TOTAL_DIFF=`echo ${HPWL_LIST[$idx]}-${HPWL_LIST[$pre_idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l) )); then
            X_TOTAL_DIFF=1
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        QUALITY_SCORE=`echo $SCORE2+$Y_RATIO | bc`
        #echo "[Eval] QUALITY_SCORE = $SCORE2 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
        break
    fi
    SCORE1=`echo $SCORE1-$SCORE_DIFF | bc`
    SCORE2=`echo $SCORE2-$SCORE_DIFF | bc`
    # case 3: HPWL > HPWL_list[n-1]
    if [ $idx -eq $((${#HPWL_LIST[@]}-1)) ]; then
        SCORE1=`echo $BOT_SCORE+$SCORE_DIFF | bc`
        SCORE2=$BOT_SCORE
        idx=$((${#HPWL_LIST[@]}-2))
        idx2=$(($idx+1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo $HPWL-${HPWL_LIST[$idx2]} | bc`
        X_TOTAL_DIFF=`echo ${HPWL_LIST[$idx2]}-${HPWL_LIST[$idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l) )); then
            X_TOTAL_DIFF=1 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        QUALITY_SCORE=`echo $SCORE2-$Y_RATIO | bc`
        #echo "[Eval] QUALITY_SCORE = $SCORE2 - $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
        break
    fi
done

# runtime
TOTAL_SCORE_DIFF=`echo $TOP_SCORE-$BOT_SCORE | bc`
SCORE_DIFF=`awk "BEGIN {print $TOTAL_SCORE_DIFF/${#RUNTIME_LIST[@]}}"`
SCORE1=`echo $TOP_SCORE+$SCORE_DIFF | bc`
SCORE2=$TOP_SCORE
for idx in ${!RUNTIME_LIST[@]}; do
    if (( $(echo "${RUNTIME_LIST[$idx]} >= $RUNTIME" |bc -l)  )); then
        if [ $idx -eq 0 ]; then
            # case1: runtime < runtime_list[0]
            SCORE1=$TOP_SCORE
            SCORE2=`echo $TOP_SCORE-$SCORE_DIFF | bc`
            idx2=$(($idx+1))
            # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
            X_DIFF=`echo ${RUNTIME_LIST[$idx]}-$RUNTIME | bc`
            X_TOTAL_DIFF=`echo ${RUNTIME_LIST[$idx2]}-${RUNTIME_LIST[$idx]} | bc`
            if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l)  )); then
                X_TOTAL_DIFF=0.01 
            fi
            Y_DIFF=$SCORE_DIFF
            X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
            Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
            RUNTIME_SCORE=`echo $SCORE1+$Y_RATIO | bc`
            #echo "[Eval] RUNTIME_SCORE = $SCORE1 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $RUNTIME_SCORE"
            break
        fi
        # case 2: runtime_list[0] < runtime < runtime_list[n-1]
        pre_idx=$(($idx-1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo ${RUNTIME_LIST[$idx]}-$RUNTIME | bc`
        X_TOTAL_DIFF=`echo ${RUNTIME_LIST[$idx]}-${RUNTIME_LIST[$pre_idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l)  )); then
            X_TOTAL_DIFF=0.01 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        RUNTIME_SCORE=`echo $SCORE2+$Y_RATIO | bc`
        #echo "[Eval] RUNTIME_SCORE = $SCORE2 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $RUNTIME_SCORE"
        break
    fi
    SCORE1=`echo $SCORE1-$SCORE_DIFF | bc`
    SCORE2=`echo $SCORE2-$SCORE_DIFF | bc`
    # case 3: runtime > runtime_list[n-1]
    if [ $idx -eq $((${#HPWL_LIST[@]}-1)) ]; then
        SCORE1=`echo $BOT_SCORE+$SCORE_DIFF | bc`
        SCORE2=$BOT_SCORE
        idx=$((${#RUNTIME_LIST[@]}-2))
        idx2=$(($idx+1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo $RUNTIME-${RUNTIME_LIST[$idx2]} | bc`
        X_TOTAL_DIFF=`echo ${RUNTIME_LIST[$idx2]}-${RUNTIME_LIST[$idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l)  )); then
            X_TOTAL_DIFF=0.01 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        RUNTIME_SCORE=`echo $SCORE2-$Y_RATIO | bc`
        #echo "[Eval] RUNTIME_SCORE = $SCORE2 - $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $RUNTIME_SCORE"
        break
    fi
done

# case score
Q_W_SCORE=`awk "BEGIN {print $QUALITY_SCORE*0.8}"`
T_W_SCORE=`awk "BEGIN {print $RUNTIME_SCORE*0.2}"`
CASE_SCORE=`echo $Q_W_SCORE+$T_W_SCORE | bc`
echo "[Eval] SCORE = QUALITY_SCORE*0.8 + RUNTIME_SCORE*0.2"
echo "             = $QUALITY_SCORE*0.8 + $RUNTIME_SCORE*0.2"
echo "             = $CASE_SCORE"