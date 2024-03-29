#!/bin/bash

#############################################################################
# File       [ evaluator.sh ]
# Author     [ FrankChiang ]
# Synopsis   [ To evaluate the score with pd21 ]
# Usage      [ bash evaluator.sh <input_file_name> <output_file_name> ]
# Date       [ Ver. 1.0 started 2023/03/10 ]
#############################################################################

RUNTIME=0
### check usage
if [ $# -eq 2 ]; then
    echo "Please input your runtime(s): "
    read -r RUNTIME
elif [ $# -eq 3 ]; then
    RUNTIME=$3
else
    echo "[Eval] usage1: bash evaluator.sh <input_file_name> <output_file_name> <runtime(s)>"
    echo "[Eval] usage2: bash evaluator.sh <input_file_name> <output_file_name>"
    exit 1
fi

### check files and runtime
# check files
BASEDIR=$(dirname "$0")
FILE_LIST=("$BASEDIR/checker" "$BASEDIR/pd23_results.list" $1 $2)
for FILE_NAME in ${FILE_LIST[@]}; do
    if [ ! -f $FILE_NAME ]; then
        echo "[Eval] error: Please check file '$FILE_NAME' exist."
        echo "(ps. checker and pd23_result.list should be in the same dir with evaluator.sh)"
        exit 1
    fi
done
# check runtime
re='^[0-9]+(.[0-9]+)?$'
if [[ ! $RUNTIME =~ $re ]] ; then
    echo "[Eval] error: Given runtime not a number" >&2; exit 1
fi

### run checker 
# run
chmod 744 ${FILE_LIST[0]}
echo "[Eval] Run cmd: ${FILE_LIST[0]} $1 $2"
${FILE_LIST[0]} $1 $2 |tee .check.log
# check
if ! grep -xq "Congratulations! Legal Solution!!" ".check.log"; then
    exit 1
fi
CUTSIZE=$(awk '/Cut size/{print $5}' .check.log)
rm -f .check.log 

echo "[Eval] Your cutsize: $CUTSIZE"
echo "[Eval] Your runtime: $RUNTIME"

### evaluater
## make table
CASENAME=$(basename "$1")
if ! grep -xq "$CASENAME" "${FILE_LIST[1]}"; then
    echo "[Eval] No case named '$CASENAME'"
    exit 1
fi
CUTSIZE_LIST=(`grep -A2 -P "^$CASENAME$" ${FILE_LIST[1]} | sed -n '2 p'`)
RUNTIME_LIST=(`grep -A2 -P "^$CASENAME$" ${FILE_LIST[1]} | sed -n '3 p'`)
## evaluate
TOP_SCORE=9.5
BOT_SCORE=4
QUALITY_SCORE=0
RUNTIME_SCORE=0

# quality
TOTAL_SCORE_DIFF=`echo $TOP_SCORE-$BOT_SCORE | bc`
SCORE_DIFF=`awk "BEGIN {print $TOTAL_SCORE_DIFF/${#CUTSIZE_LIST[@]}}"`
SCORE1=`echo $TOP_SCORE+$SCORE_DIFF | bc`
SCORE2=$TOP_SCORE
for idx in ${!CUTSIZE_LIST[@]}; do
    if [ ${CUTSIZE_LIST[$idx]} -ge $CUTSIZE ]; then
        if [ $idx -eq 0 ]; then
            # case1: cutsize < cutsize_list[0]
            SCORE1=$TOP_SCORE
            SCORE2=`echo $TOP_SCORE-$SCORE_DIFF | bc`
            idx2=$(($idx+1))
            # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
            X_DIFF=`echo ${CUTSIZE_LIST[$idx]}-$CUTSIZE | bc`
            X_TOTAL_DIFF=`echo ${CUTSIZE_LIST[$idx2]}-${CUTSIZE_LIST[$idx]} | bc`
            if [ $X_TOTAL_DIFF -eq 0 ]; then 
                X_TOTAL_DIFF=1 
            fi
            Y_DIFF=$SCORE_DIFF
            X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
            Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
            QUALITY_SCORE=`echo $SCORE1+$Y_RATIO | bc`
            #echo "[Eval] QUALITY_SCORE = $SCORE1 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
            break
        fi
        # case 2: cutsize_list[0] < cutsize < cutsize_list[n-1]
        pre_idx=$(($idx-1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo ${CUTSIZE_LIST[$idx]}-$CUTSIZE | bc`
        X_TOTAL_DIFF=`echo ${CUTSIZE_LIST[$idx]}-${CUTSIZE_LIST[$pre_idx]} | bc`
        if [ $X_TOTAL_DIFF -eq 0 ]; then 
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
    # case 3: cutsize > cutsize_list[n-1]
    if [ $idx -eq $((${#CUTSIZE_LIST[@]}-1)) ]; then
        SCORE1=`echo $BOT_SCORE+$SCORE_DIFF | bc`
        SCORE2=$BOT_SCORE
        idx=$((${#CUTSIZE_LIST[@]}-2))
        idx2=$(($idx+1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo $CUTSIZE-${CUTSIZE_LIST[$idx2]} | bc`
        X_TOTAL_DIFF=`echo ${CUTSIZE_LIST[$idx2]}-${CUTSIZE_LIST[$idx]} | bc`
        if [ $X_TOTAL_DIFF -eq 0 ]; then 
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
    if [ $idx -eq $((${#CUTSIZE_LIST[@]}-1)) ]; then
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