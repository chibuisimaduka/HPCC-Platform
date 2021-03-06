Overview of Regression Suite usage
==================================
To use Regression Suite change directory to HPCC-Platform/testing/regress subdirectory.

Parameters of Regression Suite:
-------------------------------

Command:
 
 ./regress --help

Result:

|
|       usage: regress [-h] [--version] [--config [CONFIG]]
|                       [--loglevel [{info,debug}]] [--suiteDir [SUITEDIR]]
|                       {list,run,query} ...
| 
|       HPCC Platform Regression suite
| 
|       positional arguments:
|          {list,run}            sub-command help
|            list                list help
|            run                 run help
|            query               query help
|
|       optional arguments:
|            -h, --help            show this help message and exit
|            --version, -v         show program's version number and exit
|            --config [CONFIG]     Config file to use.
|            --loglevel [{info,debug}]
|                                  Set the log level.
|            --suiteDir [SUITEDIR], -s [SUITEDIR]
|                               suiteDir to use. Default value is the current directory and it can handle relative path.

	
Steps to run Regression Suite
=============================

1. Change directory to HPCC-Platform/testing/regress subdirectory.
------------------------------------------------------------------

2. To list all available clusters:
----------------------------------
Command:

    ./regress list

The result looks like this:

        Available Clusters: 
            - setup
            - hthor
            - thor
            - roxie



3. To run the Regression Suite setup:
-------------------------------------

Command:

        ./regress run

or

        ./regress run setup

The result:

|
|        [Action] Suite: setup
|        [Action] Queries: 3
|        [Action] 1. Test: setup_fetch.ecl
|        [Pass] Pass W20130617-095047
|        [Pass] URL `http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20130617-095047`
|        [Action] 2. Test: setupxml.ecl
|        [Pass] Pass W20130617-095049
|        [Pass] URL `http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20130617-095049`
|        [Action] 3. Test: setup.ecl
|        [Pass] Pass W20130617-095051
|        [Pass] URL `http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20130617-095051`
|        [Action]
|
|            Results
|            `-------------------------------------------------`
|            Passing: 3
|            Failure: 0
|            `-------------------------------------------------`
|            Log: /var/log/HPCCSystems/regression/setup.13-06-17-09-50.log
|            `-------------------------------------------------`


	    

4. To run Regression Suite on a selected cluster (e.g. Thor):
-------------------------------------------------------------
Command:

        ./regress run cluster [-h] [--pq threadNumber]

Positional arguments:
  cluster            Run the cluster suite (default: setup).

Optional arguments:
  -h, --help         show help message and exit
  --pq threadNumber  Parallel query execution with threadNumber threads.
                    ('-1' can be use to calculate usable thread count on a single node system)

The result is a list of test cases and their result. 

The first and last couple of lines look like this:

|
|        [Action] Suite: thor
|        [Action] Queries: 257
|        [Action]
|        [Action]   1. Test: agglist.ecl
|        [Pass]   1. Pass W20131119-173524 (2 sec)
|        [Pass]   1. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-173524
|        [Action]   2. Test: aggregate.ecl
|        [Pass]   2. Pass W20131119-173527 (1 sec)
|        [Pass]   2. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-173527
|        [Action]   3. Test: aggsq1.ecl

|        .
|        .
|        .
|        [Action] 256. Test: xmlout2.ecl
|        [Pass] Pass W20131119-182536 (1 sec)
|        [Pass] URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-182536
|        [Action] 257. Test: xmlparse.ecl
|        [Pass] Pass W20131119-182537 (1 sec)
|        [Pass] URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-182537
|        
|         Results
|         `-------------------------------------------------`
|         Passing: 257
|         Failure: 0
|         `-------------------------------------------------`
|         Log: /home/ati/HPCCSystems-regression/log/thor.13-11-19-17-52-27.log
|         `-------------------------------------------------`
|         Elapsed time: 1992 sec  (00:33:12)
|         `-------------------------------------------------`
|

If --pq option used (in this case with 16 threads) then then the content of the console log will be different like this:

|
|        [Action] Suite: thor
|        [Action] Queries: 257
|        [Action]
|        [Action]   1. Test: agglist.ecl
|        [Action]   2. Test: aggregate.ecl
|        [Action]   3. Test: aggsq1.ecl
|        [Action]   4. Test: aggsq1seq.ecl
|        [Action]   5. Test: aggsq2.ecl
|        [Action]   6. Test: aggsq2seq.ecl
|        [Action]   7. Test: aggsq4.ecl
|        [Action]   8. Test: aggsq4seq.ecl
|        [Action]   9. Test: alljoin.ecl
|        [Action]  10. Test: apply3.ecl
|        [Action]  11. Test: atmost2.ecl
|        [Action]  12. Test: bcd1.ecl
|        [Action]  13. Test: bcd2.ecl
|        [Action]  14. Test: bcd4.ecl
|        [Action]  15. Test: betweenjoin.ecl
|        [Action]  16. Test: bigrecs.ecl
|        [Pass]   2. Pass W20131119-150514 (4 sec)
|        [Pass]   2. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-150514
|        [Pass]   1. Pass W20131119-150513 (4 sec)
|        [Pass]   1. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-150513
|        [Action]  17. Test: bloom2.ecl
|        [Action]  18. Test: bug8688.ecl
|        [Pass]   3. Pass W20131119-150514-5 (5 sec)
|        [Pass]   3. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-150514-5
|        [Action]  19. Test: builtin.ecl
|        [Pass]  12. Pass W20131119-150517 (5 sec)
|        [Pass]  12. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-150517
|        [Action]  20. Test: casts.ecl
|        [Pass]  14. Pass W20131119-150517-2 (6 sec)
|        [Pass]  14. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-150517-2
|        [Action]  21. Test: catchexpr.ecl
|        .
|        .
|        .
|        [Action] 257. Test: xmlparse.ecl
|        [Pass] 240. Pass W20131119-160614 (9 sec)
|        [Pass] 240. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160614
|        [Pass] 241. Pass W20131119-160614-3 (10 sec)
|        [Pass] 241. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160614-3
|        [Pass] 254. Pass W20131119-160622-1 (2 sec)
|        [Pass] 254. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160622-1
|        [Pass] 191. Pass W20131119-160058-2 (327 sec)
|        [Pass] 191. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160058-2
|        [Pass] 245. Pass W20131119-160617-3 (9 sec)
|        [Pass] 245. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160617-3
|        [Pass] 248. Pass W20131119-160619-4 (7 sec)
|        [Pass] 248. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160619-4
|        [Pass] 249. Pass W20131119-160619-3 (9 sec)
|        [Pass] 249. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160619-3
|        [Pass] 250. Pass W20131119-160620 (10 sec)
|        [Pass] 250. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160620
|        [Pass] 252. Pass W20131119-160620-3 (10 sec)
|        [Pass] 252. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160620-3
|        [Pass] 253. Pass W20131119-160622 (8 sec)
|        [Pass] 253. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160622
|        [Pass] 255. Pass W20131119-160623 (8 sec)
|        [Pass] 255. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160623
|        [Pass] 256. Pass W20131119-160623-1 (9 sec)
|        [Pass] 256. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160623-1
|        [Pass] 257. Pass W20131119-160624 (9 sec)
|        [Pass] 257. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160624
|        [Pass] 213. Pass W20131119-160138-4 (305 sec)
|        [Pass] 213. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-160138-4
|        [Pass] 127. Pass W20131119-155918 (462 sec)
|        [Pass] 127. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-155918
|        [Pass] 100. Pass W20131119-155713 (600 sec)
|        [Pass] 100. URL http://127.0.0.1:8010/WsWorkunits/WUInfo?Wuid=W20131119-155713
|        [Action]
|        [Action]
|         Results
|         `-------------------------------------------------`
|         Passing: 257
|         Failure: 0
|         `-------------------------------------------------`
|         Log: /home/ati/HPCCSystems-regression/log/thor.13-11-19-15-55-32.log
|         `-------------------------------------------------`
|         Elapsed time: 701 sec  (00:11:41)
|         `-------------------------------------------------`
|

The logfile generated into the HPCCSystems-regression/log subfolder of the user personal folder and sorted by the test case number.


5. To run Regression Suite with selected test case on a selected cluster (e.g. Thor): 
-------------------------------------------------------------------------------------

(In this use case the default cluster is: thor)

Command:

        ./regress query [-h] [--publish] test_name [target cluster | all]

Positional arguments:
        test_name               Name of a single ECL query (mandatory).
        target cluster | all    Cluster for single query run (default: thor).
                                If cluster = 'all' then run ECL query on all clusters.
Optional arguments:
        -h, --help            Show help message and exit
        --publish             Publish compiled query instead of run.


The format of result is same as above:

6. Tags used in testcases:
--------------------------

To exclude testcase from cluster or clusters, the tag is:
//no<cluster_name>

To skip (similar to exclusion)
//skip type==<cluster> <reason>

To build and publish testcase (e.g.:for libraries)
//publish

**Important! Actually regression suite compares the test case result with xml files stored in testing/regression/ecl/key independently from the cluster.**
