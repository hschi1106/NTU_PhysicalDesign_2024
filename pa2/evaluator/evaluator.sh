#!/bin/bash

#############################################################################
# File       [ evaluator.sh ]
# Author     [ FrankChiang ]
# Synopsis   [ To evaluate the score with pd22 ]
# Usage      [ bash evaluator.sh <input.block> <input.net> <output.rpt> <alpha> ]
# Date       [ Ver. 1.0 started 2023/03/10 ]
#############################################################################

RUNTIME=0
### check usage
if [ $# -eq 4 ]; then
    ALPHA=$4
else
    echo "[Eval] usage1: ./evaluator.sh <input.block> <input.net> <output.rpt> <alpha>" # <runtime(s)>"
    #echo "[Eval] usage2: ./evaluator.sh <input.block> <input.net> <output.rpt> <alpha> #And input your runtime(s)."
    exit 1
fi

### check files and runtime
# check files
BASEDIR=$(dirname "$0")
FILE_LIST=("$BASEDIR/checker" "$BASEDIR/pd23_results.list" $1 $2 $3)
for FILE_NAME in ${FILE_LIST[@]}; do
    if [ ! -f $FILE_NAME ]; then
        echo "[Eval] error: Please check file '$FILE_NAME' exist."
        echo "(ps. checker and pd23_result.list should be in the same dir with evaluator.sh)"
        exit 1
    fi
done
# check alpha
re='^(0)(.[0-9]+)$'
if [[ ! $ALPHA =~ $re ]] ; then
    echo "[Eval] error: Given alpha legal"; exit 1
fi
if [[ $ALPHA != "0.5" ]] ; then
    echo "[Eval] Warning: Only sopprot alpha=0.5 for public evaluation. The temporary score would not be correct!"
fi 

### run checker 
# run
chmod 744 ${FILE_LIST[0]}
echo "[Eval] Run cmd: ${FILE_LIST[0]} $1 $2 $3 $ALPHA"
${FILE_LIST[0]} $1 $2 $3 $ALPHA |tee .check.log
# check
if ! grep -xq "Congradulations, your results are correct" ".check.log"; then
    exit 1
fi
# get wirelength, area, and runtime, and calculate normalized cost
WL=$(awk '/wirelength:/{print $3}' .check.log)
WL=${WL//\/reported/}
AREA=$(awk '/area:/{print $3}' .check.log)
AREA=${AREA//\/reported/}
RUNTIME=$(awk '/runtime:/{print $2}' .check.log)
ALPHA_OP=`awk "BEGIN {print 1-$ALPHA}"`
rm -f .check.log

# check runtime
re='^[0-9]+(.[0-9]+)?$'
if [[ ! $RUNTIME =~ $re ]] ; then
    echo "[Eval] error: Given runtime not a number"; exit 1
fi

echo "[Eval] Your area & wirelength: $AREA & $WL"
echo "[Eval] Your runtime: $RUNTIME"

### evaluater
## make table
CASENAME=$(basename "$1" .block)
if ! grep -Fxq "$CASENAME" "${FILE_LIST[1]}"; then
    echo "[Eval] No case named '$CASENAME'"
    exit 1
fi
MinMaxWLArea_LIST=(`grep -A3 -P "^$CASENAME$" ${FILE_LIST[1]} | sed -n '2 p'`)
COST_LIST=(`grep -A3 -P "^$CASENAME$" ${FILE_LIST[1]} | sed -n '3 p'`)
RUNTIME_LIST=(`grep -A3 -P "^$CASENAME$" ${FILE_LIST[1]} | sed -n '4 p'`)
#echo "${MinMaxWLArea_LIST[*]}"
COST=`awk "BEGIN {print $ALPHA*($AREA-${MinMaxWLArea_LIST[2]})/(${MinMaxWLArea_LIST[3]}-${MinMaxWLArea_LIST[2]})+$ALPHA_OP*($WL-${MinMaxWLArea_LIST[0]})/(${MinMaxWLArea_LIST[1]}-${MinMaxWLArea_LIST[0]})}"`
#echo "COST=$ALPHA*($AREA-${MinMaxWLArea_LIST[2]})/(${MinMaxWLArea_LIST[3]}-${MinMaxWLArea_LIST[2]})+$ALPHA_OP*($WL-${MinMaxWLArea_LIST[0]})/(${MinMaxWLArea_LIST[1]}-${MinMaxWLArea_LIST[0]})=$COST"
echo "[Eval] Your normalized cost = $ALPHA*(A-minA)/(maxA-minA) + $ALPHA_OP*(WL-minWL)/(maxWL-minWL) = $COST"

## evaluate
TOP_SCORE=9.5
BOT_SCORE=4
QUALITY_SCORE=0
RUNTIME_SCORE=0

# quality
TOTAL_SCORE_DIFF=`echo $TOP_SCORE - $BOT_SCORE | bc`
SCORE_DIFF=`awk "BEGIN {print $TOTAL_SCORE_DIFF/${#COST_LIST[@]}}"`
SCORE1=`echo $TOP_SCORE + $SCORE_DIFF | bc`
SCORE2=$TOP_SCORE
for idx in ${!COST_LIST[@]}; do
    if (( $(echo "${COST_LIST[$idx]} >= $COST" |bc -l) )); then
        if [ $idx -eq 0 ]; then
            # case1: cutsize < cutsize_list[0]
            SCORE1=$TOP_SCORE
            SCORE2=`echo $TOP_SCORE - $SCORE_DIFF | bc`
            idx2=$(($idx+1))
            # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
            X_DIFF=`echo ${COST_LIST[$idx]} - $COST | bc`
            X_TOTAL_DIFF=`echo ${COST_LIST[$idx2]} - ${COST_LIST[$idx]} | bc`
            if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l) )); then
                X_TOTAL_DIFF=0.001 
            fi
            Y_DIFF=$SCORE_DIFF
            X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
            Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
            QUALITY_SCORE=`echo $SCORE1 + $Y_RATIO | bc`
            #echo "[Eval] QUALITY_SCORE = $SCORE1 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
            break
        fi
        # case 2: cutsize_list[0] < cutsize < cutsize_list[n-1]
        pre_idx=$(($idx-1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo ${COST_LIST[$idx]} - $COST | bc`
        X_TOTAL_DIFF=`echo ${COST_LIST[$idx]} - ${COST_LIST[$pre_idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l) )); then
            X_TOTAL_DIFF=0.001 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        QUALITY_SCORE=`echo $SCORE2 + $Y_RATIO | bc`
        #echo "[Eval] QUALITY_SCORE = $SCORE2 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
        break
    fi
    SCORE1=`echo $SCORE1 - $SCORE_DIFF | bc`
    SCORE2=`echo $SCORE2 - $SCORE_DIFF | bc`
    # case 3: cutsize > cutsize_list[n-1]
    if [ $idx -eq $((${#COST_LIST[@]}-1)) ]; then
        SCORE1=`echo $BOT_SCORE + $SCORE_DIFF | bc`
        SCORE2=$BOT_SCORE
        idx=$((${#COST_LIST[@]}-2))
        idx2=$(($idx+1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo $COST - ${COST_LIST[$idx2]} | bc`
        X_TOTAL_DIFF=`echo ${COST_LIST[$idx2]} - ${COST_LIST[$idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l) )); then
            X_TOTAL_DIFF=1 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        QUALITY_SCORE=`echo $SCORE2 - $Y_RATIO | bc`
        #echo "[Eval] QUALITY_SCORE = $SCORE2 - $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $QUALITY_SCORE"
        break
    fi
done

# runtime
TOTAL_SCORE_DIFF=`echo $TOP_SCORE - $BOT_SCORE | bc`
SCORE_DIFF=`awk "BEGIN {print $TOTAL_SCORE_DIFF/${#RUNTIME_LIST[@]}}"`
SCORE1=`echo $TOP_SCORE + $SCORE_DIFF | bc`
SCORE2=$TOP_SCORE
for idx in ${!RUNTIME_LIST[@]}; do
    if (( $(echo "${RUNTIME_LIST[$idx]} >= $RUNTIME" |bc -l)  )); then
        if [ $idx -eq 0 ]; then
            # case1: runtime < runtime_list[0]
            SCORE1=$TOP_SCORE
            SCORE2=`echo $TOP_SCORE - $SCORE_DIFF | bc`
            idx2=$(($idx+1))
            # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
            X_DIFF=`echo ${RUNTIME_LIST[$idx]} - $RUNTIME | bc`
            X_TOTAL_DIFF=`echo ${RUNTIME_LIST[$idx2]} - ${RUNTIME_LIST[$idx]} | bc`
            if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l)  )); then
                X_TOTAL_DIFF=0.01 
            fi
            Y_DIFF=$SCORE_DIFF
            X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
            Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
            RUNTIME_SCORE=`echo $SCORE1 + $Y_RATIO | bc`
            #echo "[Eval] RUNTIME_SCORE = $SCORE1 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $RUNTIME_SCORE"
            break
        fi
        # case 2: runtime_list[0] < runtime < runtime_list[n-1]
        pre_idx=$(($idx-1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo ${RUNTIME_LIST[$idx]} - $RUNTIME | bc`
        X_TOTAL_DIFF=`echo ${RUNTIME_LIST[$idx]} - ${RUNTIME_LIST[$pre_idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l)  )); then
            X_TOTAL_DIFF=0.01 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        RUNTIME_SCORE=`echo $SCORE2 + $Y_RATIO | bc`
        #echo "[Eval] RUNTIME_SCORE = $SCORE2 + $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $RUNTIME_SCORE"
        break
    fi
    SCORE1=`echo $SCORE1 - $SCORE_DIFF | bc`
    SCORE2=`echo $SCORE2 - $SCORE_DIFF | bc`
    # case 3: runtime > runtime_list[n-1]
    if [ $idx -eq $((${#COST_LIST[@]}-1)) ]; then
        SCORE1=`echo $BOT_SCORE + $SCORE_DIFF | bc`
        SCORE2=$BOT_SCORE
        idx=$((${#RUNTIME_LIST[@]}-2))
        idx2=$(($idx+1))
        # y = y1 + (x-x1)/(x2-x1) * (y2-y1)
        X_DIFF=`echo $RUNTIME - ${RUNTIME_LIST[$idx2]} | bc`
        X_TOTAL_DIFF=`echo ${RUNTIME_LIST[$idx2]} - ${RUNTIME_LIST[$idx]} | bc`
        if (( $(echo "$X_TOTAL_DIFF == 0" |bc -l)  )); then
            X_TOTAL_DIFF=0.01 
        fi
        Y_DIFF=$SCORE_DIFF
        X_RATIO=`awk "BEGIN {print $X_DIFF/$X_TOTAL_DIFF}"`
        Y_RATIO=`awk "BEGIN {print $X_RATIO*$Y_DIFF}"`
        RUNTIME_SCORE=`echo $SCORE2 - $Y_RATIO | bc`
        #echo "[Eval] RUNTIME_SCORE = $SCORE2 - $X_DIFF/$X_TOTAL_DIFF * $Y_DIFF = $RUNTIME_SCORE"
        break
    fi
done

# case score
Q_W_SCORE=`awk "BEGIN {print $QUALITY_SCORE*0.8}"`
T_W_SCORE=`awk "BEGIN {print $RUNTIME_SCORE*0.2}"`
CASE_SCORE=`echo $Q_W_SCORE + $T_W_SCORE | bc`
echo "[Eval] SCORE = QUALITY_SCORE*0.8 + RUNTIME_SCORE*0.2"
echo "             = $QUALITY_SCORE*0.8 + $RUNTIME_SCORE*0.2"
echo "             = $CASE_SCORE"