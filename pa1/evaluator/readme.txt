For Evaluator:
    Usage1: 
    ./evaluator.sh [input_file_name] [output_file_name] [runtime]
    Usage2: 
    ./evaluator.sh [input_file_name] [output_file_name]
    Please input your runtime(s):
    [runtime]

    Ex: bash evaluator.sh input_0.dat result_0.dat 0.14

    Results:
    [Eval] ./checker input_0.dat result_0.dat
    [Check] Cut size = 3400 matched!
    [Check] Balance passed:: 60300(min) < 76453(G1), 74297(G2) < 90450(max)
    =================================
    Congratulations! Legal Solution!!
    =================================

    [Eval] Your cutsize: 3400
    [Eval] Your runtime: 0.14
    [Eval] SCORE = QUALITY_SCORE*0.8 + RUNTIME_SCORE*0.2
                = 8.8567874*0.8 + 19.03331*0.2
                = 10.89209

    --
    Contact: Yan-Jen Chen <yjchen@eda.ee.ntu.edu.tw>


For Checker:
    Usage: 
    chmod 744 checker
    ./checker [input_file_name] [output_file_name]


    Ex: ./checker case1.dat result1.dat

    Results:
    [Check] Cut size = 3400 matched!
    [Check] Balance passed:: 60300(min) < 76453(G1), 74297(G2) < 90450(max)
    =================================
    Congratulations! Legal Solution!!
    =================================
