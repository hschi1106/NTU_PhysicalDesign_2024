For Evaluator:
    Usage: 
    ./evaluator.sh [input.block] [input.net] [output.rpt] [alpha]
    ## Note that we only support alpha=0.5 for public evaluation ##

    Ex: bash evaluator.sh ami33.block ami33.nets ami33.rpt 0.5
    Results: 
    [Eval] Run cmd: ./checker ../input_pa2/ami33.block ../input_pa2/ami33.nets ami33.rpt 0.5

    Checking Report...
    cost:       actual  662927.500000/reported 662928.000000
    wirelength: actual  71602.000000/reported 71602.000000
    area:       actual  1254253.000000/reported 1254253.000000
    width:      outline 1326/actual 1253/reported 1253.000000
    height:     outline 1205/actual 1001/reported 1001.000000
    runtime: 0.001531

    Congradulations, your results are correct

    [Eval] Your area & wirelength: 1254253.000000 & 71602.000000
    [Eval] Your runtime: 0.001531
    [Eval] Your normalized cost = 0.5*(A-minA)/(maxA-minA) + 0.5*(WL-minWL)/(maxWL-minWL) = 0.0794171
    [Eval] SCORE = QUALITY_SCORE*0.8 + RUNTIME_SCORE*0.2
                = 6.687530*0.8 + 9.103054*0.2
                = 7.17063

    --
    Contact: Yan-Jen Chen <yjchen@eda.ee.ntu.edu.tw>


For Checker:
	Usage:
	./checker [input block] [input net] [output] [alpha]

    Ex: ./checker ami33.block ami33.nets ami33.rpt 0.5

    Results:
	Checking Report...
    cost:       actual  662927.500000/reported 662928.000000
    wirelength: actual  71602.000000/reported 71602.000000
    area:       actual  1254253.000000/reported 1254253.000000
    width:      outline 1326/actual 1253/reported 1253.000000
    height:     outline 1205/actual 1001/reported 1001.000000
    runtime: 0.001531

    Congradulations, your results are correct
	--
