#### No IndexJoinPlacementRule vs IndexJoinPlacementRule + Additional PredicateReorderingRule

commit id: `95b5d33a963cb625dfd5497dda0c0bdda097b607`  

How often was an IndexJoin used for a query?
- Q 01: -
- Q 02: 2
- Q 03: 1
- Q 04: -
- Q 05: 1
- Q 06: -
- Q 07: -
- Q 08: 1
- Q 09: 1
- Q 10: -
- Q 11: -
- Q 12: -
- Q 13: -
- Q 14: 1
- Q 15: -
- Q 16: -
- Q 17: 1
- Q 18: -
- Q 19: 1
- Q 20: -
- Q 21: -
- Q 22: -


```
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| TPC-H 1        | 0.21275626123  | 13   | 0.213594436646 | 13   | +0%        |                          0.3233 |
| TPC-H 2        | 4.32471132278  | 260  | 3.25851345062  | 196  | -25%       |                          0.0000 |
| TPC-H 3        | 3.05668640137  | 184  | 2.08375716209  | 126  | -32%       |                          0.0000 |
| TPC-H 4        | 1.09300518036  | 66   | 1.0379884243   | 63   | -5%        |                          0.0000 |
| TPC-H 5        | 2.06358098984  | 124  | 2.01935076714  | 122  | -2%        |                          0.0000 |
| TPC-H 6        | 3.89955830574  | 234  | 3.80540108681  | 229  | -2%        |                          0.0001 |
| TPC-H 7        | 0.629360556602 | 38   | 0.628725111485 | 38   | -0%        |                          0.7107 |
| TPC-H 8        | 2.29891943932  | 138  | 1.26149916649  | 76   | -45%       |                          0.0000 |
| TPC-H 9        | 0.710052847862 | 43   | 0.419112503529 | 26   | -41%       |                          0.0000 |
| TPC-H 10       | 1.36736500263  | 83   | 1.36793327332  | 83   | +0%        |                          0.8201 |
| TPC-H 11       | 13.5454950333  | 813  | 13.5193748474  | 812  | -0%        |                          0.3934 |
| TPC-H 12       | 3.47885560989  | 209  | 3.55911684036  | 214  | +2%        |                          0.0000 |
| TPC-H 13       | 1.20316195488  | 73   | 1.18467319012  | 72   | -2%        |                          0.0314 |
| TPC-H 14       | 11.7121486664  | 703  | 1.38437581062  | 84   | -88%       |                          0.0000 |
| TPC-H 15       | 7.85988664627  | 472  | 7.82859277725  | 470  | -0%        |                          0.0104 |
| TPC-H 16       | 3.39795780182  | 204  | 3.43517804146  | 207  | +1%        |                          0.0000 |
| TPC-H 17       | 0.675080537796 | 41   | 0.543406546116 | 33   | -20%       |                          0.0000 |
| TPC-H 18       | 0.746687352657 | 45   | 0.732461571693 | 44   | -2%        |                          0.0074 |
| TPC-H 19       | 2.6254491806   | 158  | 1.0183069706   | 62   | -61%       |                          0.0000 |
| TPC-H 20       | 1.0215395689   | 62   | 0.981803119183 | 59   | -4%        |                          0.0000 |
| TPC-H 21       | 0.223747879267 | 14   | 0.225249275565 | 14   | +1%        |                          0.0121 |
| TPC-H 22       | 5.24645805359  | 315  | 5.25656890869  | 316  | +0%        |                          0.6276 |
| geometric mean |                |      |                |      | -21%       |                                 |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
```

#### No IndexJoinPlacementRule vs IndexJoinPlacementRule

commit id: `505b12d78ee9fd953ae1152459c4df3d120fb821`  

- Q 01: -
- Q 02: 2
- Q 03: 1
- Q 04: -
- Q 05: 1
- Q 06: -
- Q 07: -
- Q 08: 1
- Q 09: 1
- Q 10: -
- Q 11: -
- Q 12: -
- Q 13: -
- Q 14: 1
- Q 15: -
- Q 16: -
- Q 17: 1
- Q 18: -
- Q 19: 1
- Q 20: -
- Q 21: -
- Q 22: -

```
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| TPC-H 1        | 0.21275626123  | 13   | 0.177822008729 | 11   | -16%       |                          0.0000 |
| TPC-H 2        | 4.32471132278  | 260  | 2.75368785858  | 166  | -36%       |                          0.0000 |
| TPC-H 3        | 3.05668640137  | 184  | 2.02779865265  | 122  | -34%       |                          0.0000 |
| TPC-H 4        | 1.09300518036  | 66   | 1.07735061646  | 65   | -1%        |                          0.0000 |
| TPC-H 5        | 2.06358098984  | 124  | 2.06545615196  | 124  | +0%        |                          0.8530 |
| TPC-H 6        | 3.89955830574  | 234  | 3.86253786087  | 232  | -1%        |                          0.1224 |
| TPC-H 7        | 0.629360556602 | 38   | 0.641739308834 | 39   | +2%        |                          0.0000 |
| TPC-H 8        | 2.29891943932  | 138  | 1.28869831562  | 78   | -44%       |                          0.0000 |
| TPC-H 9        | 0.710052847862 | 43   | 0.436373323202 | 27   | -39%       |                          0.0000 |
| TPC-H 10       | 1.36736500263  | 83   | 1.37905740738  | 83   | +1%        |                          0.0027 |
| TPC-H 11       | 13.5454950333  | 813  | 13.3932495117  | 804  | -1%        |                          0.0000 |
| TPC-H 12       | 3.47885560989  | 209  | 3.55863380432  | 214  | +2%        |                          0.0000 |
| TPC-H 13       | 1.20316195488  | 73   | 1.19653761387  | 72   | -1%        |                          0.4008 |
| TPC-H 14       | 11.7121486664  | 703  | 1.41830122471  | 86   | -88%       |                          0.0000 |
| TPC-H 15       | 7.85988664627  | 472  | 7.93642759323  | 477  | +1%        |                          0.0000 |
| TPC-H 16       | 3.39795780182  | 204  | 3.40516662598  | 205  | +0%        |                          0.3266 |
| TPC-H 17       | 0.675080537796 | 41   | 0.543012738228 | 33   | -20%       |                          0.0000 |
| TPC-H 18       | 0.746687352657 | 45   | 0.742865085602 | 45   | -1%        |                          0.5226 |
| TPC-H 19       | 2.6254491806   | 158  | 0.741265654564 | 45   | -72%       |                          0.0000 |
| TPC-H 20       | 1.0215395689   | 62   | 1.01655244827  | 61   | -0%        |                          0.0453 |
| TPC-H 21       | 0.223747879267 | 14   | 0.222683742642 | 14   | -0%        |                          0.1661 |
| TPC-H 22       | 5.24645805359  | 315  | 5.2547659874   | 316  | +0%        |                          0.7329 |
| geometric mean |                |      |                |      | -23%       |                                 |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
```

#### TPCH JoinHash Size Study

The following table shows information about tables used for hash joins in processing the TPCH query set.
Used Scale Factor: 1.0

https://docs.google.com/spreadsheets/d/1PuGSHS9GFtzB29SbFe6wESE8arqGQXr4LUQY4j_8vpQ

#### Experiment 8

commit id: `a0ece74fb23bc27322c520aecaf905b21d422d54`  
SF: 10.0

Same experiment as in experment 7 with SF 10.

Joining first is **389,79776534** times faster than scanning first.

```
2019-05-03 10:33:36
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 10 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (3 min 2 s)
- Encoding tables if necessary
-  Encoding 'region' - encoding applied (801 µs 571 ns)
-  Encoding 'nation' - encoding applied (456 µs 767 ns)
-  Encoding 'supplier' - encoding applied (568 ms 954 µs)
-  Encoding 'orders' - encoding applied (2 s 248 ms)
-  Encoding 'partsupp' - encoding applied (1 s 130 ms)
-  Encoding 'customer' - encoding applied (1 s 421 ms)
-  Encoding 'lineitem' - encoding applied (11 s 296 ms)
-  Encoding 'part' - encoding applied (1 s 154 ms)
- Encoding tables done (17 s 819 ms)
- Adding tables to StorageManager and generating statistics 
-  Adding 'region' (205 µs 892 ns)
-  Adding 'nation' (47 µs 838 ns)
-  Adding 'supplier' (533 ms 189 µs)
-  Adding 'orders' (36 s 398 ms)
-  Adding 'partsupp' (14 s 382 ms)
-  Adding 'customer' (6 s 254 ms)
-  Adding 'lineitem' (2 min 23 s)
-  Adding 'part' (4 s 628 ms)
- Adding tables to StorageManager and generating statistics done (3 min 25 s)
--------------------------------------------------------------------------------------------------
Benchmark                                                           Time           CPU Iterations
--------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lScan_lScan_plJoin    671186686 ns  670693831 ns          1
TPCHDataMicroBenchmarkFixture/BM_TPCH_plJoin_plScan_plScan    1721498 ns    1720620 ns        392

```

#### Experiment 7

commit id: `074ebabcef298d6cde4263a55066cedd9d392941`  
SF: 1.0

In this experiment the execution time using different orders of the operators is measured.
The first benchmark measures the execution times of the two table scans on the lineitem table followed by the index join of a pre-scanned part table and the twice-scanned lineitem table.
The second benchmark measures the execution time of the index join of a pre-scanned part table and two scans of the result table afterwards.

Joining first is **169,213611404** times faster than scanning first.

```
2019-05-03 10:17:46
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (23 s 210 ms)
- Encoding tables if necessary
-  Encoding 'region' - encoding applied (684 µs 27 ns)
-  Encoding 'nation' - encoding applied (515 µs 323 ns)
-  Encoding 'supplier' - encoding applied (44 ms 104 µs)
-  Encoding 'orders' - encoding applied (795 ms 528 µs)
-  Encoding 'partsupp' - encoding applied (380 ms 385 µs)
-  Encoding 'customer' - encoding applied (627 ms 730 µs)
-  Encoding 'lineitem' - encoding applied (1 s 297 ms)
-  Encoding 'part' - encoding applied (593 ms 464 µs)
- Encoding tables done (3 s 740 ms)
- Adding tables to StorageManager and generating statistics 
-  Adding 'region' (211 µs 991 ns)
-  Adding 'nation' (49 µs 100 ns)
-  Adding 'supplier' (33 ms 332 µs)
-  Adding 'orders' (3 s 24 ms)
-  Adding 'partsupp' (1 s 385 ms)
-  Adding 'customer' (480 ms 365 µs)
-  Adding 'lineitem' (13 s 309 ms)
-  Adding 'part' (360 ms 918 µs)
- Adding tables to StorageManager and generating statistics done (18 s 593 ms)
--------------------------------------------------------------------------------------------------------
Benchmark                                                                 Time           CPU Iterations
--------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_pScan_lScan_lScan_plJoin     54044522 ns   53988618 ns         13
TPCHDataMicroBenchmarkFixture/BM_TPCH_pScan_plJoin_plScan_plScan     319268 ns     319056 ns       2242
```

#### Experiment 6

commit id: `c684fa97779aed151b9344857e626a575a8719a1`  
Same setup as in experiment 5 but with **Scale Factor 10.0**.   
The JoinIndex is **4,776724478** times faster than the JoinHash.

```
2019-04-27 11:08:51
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 10 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (3 min 1 s)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (4 ms 193 µs)
-  Encoding 'orders' - encoding applied (2 s 633 ms)
-  Encoding 'region' - encoding applied (9 ms 306 µs)
-  Encoding 'part' - encoding applied (965 ms 449 µs)
-  Encoding 'lineitem' - encoding applied (14 s 115 ms)
-  Encoding 'partsupp' - encoding applied (2 s 77 ms)
-  Encoding 'customer' - encoding applied (1 s 222 ms)
-  Encoding 'supplier' - encoding applied (767 ms 475 µs)
- Encoding tables done (21 s 794 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (280 µs 422 ns)
-  Adding 'orders' (40 s 60 ms)
-  Adding 'region' (80 µs 433 ns)
-  Adding 'part' (4 s 879 ms)
-  Adding 'lineitem' (2 min 28 s)
-  Adding 'partsupp' (17 s 322 ms)
-  Adding 'customer' (6 s 813 ms)
-  Adding 'supplier' (319 ms 144 µs)
- Adding Tables to StorageManager and generating statistics done (3 min 38 s)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join   131053162 ns  130965017 ns          5
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join   27433147 ns   27417327 ns         25
```

#### Experiment 5

commit id: `6126e95aa103596e9c06466a45952ef4df74a2f2`

In this experiment, a reduce PART table (reference table) and a reduced LINEITEM table (reference table) are joined using the JoinIndex operator. The JoinIndex operator builds one large PosList using all PosLists of the ReferenceSegments of the reduced LINEITEM table. Let this PosList be `possible_lineitem_positions`, it gets sorted afterwards.
Additionally the join column of the reduced PART table is iterated. For each value, a **partial** IndexScan is executed on the original LINEITEM table that the reduced LINEITEM table references. A partial IndexScan means, that only the matching positions are calculated and a output table with reference chunks are not created. Therefore partial IndexScan returns a PosList as result. Let this PosList be `lineitem_matches`. After getting this PosList, it gets sorted and intersected with the previously sorted `possible_lineitem_positions`. The result PosList of this intersection contains the RowIDs of the reduced LINEITEM table that matches the IndexScan predicate.

reduced part table (left join input): 3 rows  
reduced lineitem table (right join input): 75'983 rows  
join result table: 1 row

Scale Factor 1.0  
The JoinIndex is **1,264768186** times faster than the JoinHash.

```
2019-04-27 10:37:30
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (24 s 721 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (943 µs 660 ns)
-  Encoding 'orders' - encoding applied (754 ms 214 µs)
-  Encoding 'region' - encoding applied (820 µs 305 ns)
-  Encoding 'part' - encoding applied (594 ms 388 µs)
-  Encoding 'lineitem' - encoding applied (1 s 830 ms)
-  Encoding 'partsupp' - encoding applied (449 ms 890 µs)
-  Encoding 'customer' - encoding applied (764 ms 758 µs)
-  Encoding 'supplier' - encoding applied (56 ms 689 µs)
- Encoding tables done (4 s 453 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (257 µs 802 ns)
-  Adding 'orders' (3 s 138 ms)
-  Adding 'region' (52 µs 971 ns)
-  Adding 'part' (421 ms 774 µs)
-  Adding 'lineitem' (13 s 695 ms)
-  Adding 'partsupp' (1 s 260 ms)
-  Adding 'customer' (463 ms 208 µs)
-  Adding 'supplier' (25 ms 316 µs)
- Adding Tables to StorageManager and generating statistics done (19 s 4 ms)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join     3120240 ns    3118439 ns        223
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join    2466743 ns    2465621 ns        283

```

#### Experiment 4

commit id: `e0b9a287d5eb3ac16916f50c63746cac4b97d832`  
Scale Factor 1.0  
The JoinIndex is **2,6892** times faster than the JoinHash.

Modifications since the previous experiment:
In this version, the concatinated PosList for the reduced lineitem table and the concatineted PosList for the matches in the lineitem table are not sorted, since they are already sorted when the execution is single threaded.

```
2019-04-26 17:57:26
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (24 s 995 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 98 µs)
-  Encoding 'orders' - encoding applied (842 ms 301 µs)
-  Encoding 'region' - encoding applied (853 µs 439 ns)
-  Encoding 'part' - encoding applied (593 ms 697 µs)
-  Encoding 'lineitem' - encoding applied (2 s 47 ms)
-  Encoding 'partsupp' - encoding applied (410 ms 145 µs)
-  Encoding 'customer' - encoding applied (764 ms 733 µs)
-  Encoding 'supplier' - encoding applied (50 ms 147 µs)
- Encoding tables done (4 s 710 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (217 µs 624 ns)
-  Adding 'orders' (2 s 956 ms)
-  Adding 'region' (45 µs 485 ns)
-  Adding 'part' (403 ms 715 µs)
-  Adding 'lineitem' (13 s 563 ms)
-  Adding 'partsupp' (1 s 258 ms)
-  Adding 'customer' (433 ms 556 µs)
-  Adding 'supplier' (23 ms 647 µs)
- Adding Tables to StorageManager and generating statistics done (18 s 639 ms)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join     3068475 ns    3067048 ns        222
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join    1141085 ns    1140485 ns        611

```

#### Join: reduced PART with reduced LINEITEM: Experiment 3

commit id: `87ee079d5df8934701d0d82ab6d892148fe28882`  
Scale Factor 10.0  
The JoinIndex is **4,074700484** times faster than the JoinHash.

```
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 10 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (3 min 6 s)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 29 µs)
-  Encoding 'orders' - encoding applied (2 s 849 ms)
-  Encoding 'region' - encoding applied (5 ms 3 µs)
-  Encoding 'part' - encoding applied (923 ms 311 µs)
-  Encoding 'lineitem' - encoding applied (13 s 524 ms)
-  Encoding 'partsupp' - encoding applied (2 s 138 ms)
-  Encoding 'customer' - encoding applied (1 s 279 ms)
-  Encoding 'supplier' - encoding applied (879 ms 904 µs)
- Encoding tables done (21 s 599 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (259 µs 239 ns)
-  Adding 'orders' (42 s 286 ms)
-  Adding 'region' (116 µs 375 ns)
-  Adding 'part' (4 s 796 ms)
-  Adding 'lineitem' (2 min 32 s)
-  Adding 'partsupp' (17 s 943 ms)
-  Adding 'customer' (7 s 106 ms)
-  Adding 'supplier' (300 ms 995 µs)
- Adding Tables to StorageManager and generating statistics done (3 min 45 s)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join   136170268 ns  136105460 ns          4
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join   33417554 ns   33402568 ns         21
```

#### Join: reduced PART with reduced LINEITEM: Experiment 2

commit id: `67b7dae1450a66a85a804bb4dbebdddf9589b0e8`  
Scale Factor 1.0  
The JoinIndex is **0,253471597** times faster than the JoinHash.

```
2019-04-25 12:01:09
Running ./hyriseMicroBenchmarks
Run on (80 X 2395 MHz CPU s)
CPU Caches:
  L1 Data 32K (x40)
  L1 Instruction 32K (x40)
  L2 Unified 256K (x40)
  L3 Unified 30720K (x4)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables 
- Loading/Generating tables done (1 min 37 s)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 499 µs)
-  Encoding 'orders' - encoding applied (3 s 3 ms)
-  Encoding 'region' - encoding applied (1 ms 49 µs)
-  Encoding 'part' - encoding applied (2 s 971 ms)
-  Encoding 'lineitem' - encoding applied (6 s 323 ms)
-  Encoding 'partsupp' - encoding applied (1 s 467 ms)
-  Encoding 'customer' - encoding applied (3 s 58 ms)
-  Encoding 'supplier' - encoding applied (234 ms 844 µs)
- Encoding tables done (17 s 60 ms)
- Adding Tables to StorageManager and generating statistics 
-  Adding 'nation' (571 µs 77 ns)
-  Adding 'orders' (13 s 624 ms)
-  Adding 'region' (231 µs 903 ns)
-  Adding 'part' (1 s 847 ms)
-  Adding 'lineitem' (1 min 9 s)
-  Adding 'partsupp' (4 s 84 ms)
-  Adding 'customer' (1 s 698 ms)
-  Adding 'supplier' (99 ms 984 µs)
- Adding Tables to StorageManager and generating statistics done (1 min 31 s)
[PERF] Using type-erased accessor as the ReferenceSegmentIterable is type-erased itself at src/lib/storage/reference_segment/reference_segment_iterable.hpp:57
        Performance can be affected. This warning is only shown once.

------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join    48209320 ns   48187412 ns         11
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join  190173864 ns  190109711 ns          4
```

#### Join: reduced PART with reduced LINEITEM: Experiment 1

In these benchmarks, first table PART is reduced by a scan. This leads to an intermediate result table with only 3 rows (filter predicate: p_partkey <= 3). Let this intermediate table be REDUCED_PART. Additionally, let REDUCED_LINEITEM be a reduced LINEITEM table. It only contains tuples of the original table where l_shipdate >= '1995-09-01' AND l_shipdate < '1995-10-01'. REDUCED_PART is then joined with table REDUCED_LINEITEM using the join predicate p_partkey = l_partkey. REDUCED_LINEITEM has indices for column l_shipdate.

The `data_table`-benchmarks are using REDUCED_LINEITEM as data table (the result table of the second scan is stored as data table),
the `reference_table`-benchmarks are using the REDUCED_LINEITEM as reference table (the output table of the second scan).

The result below shows that the JoinIndex is a factor of **116,2488** faster than the JoinHash if REDUCED_LINEITEM is a data table.
If the REDUCED_LINEITEM is the output of the second scan and thus a reference table,
the JoinIndex is a factor of **1,0997** faster than the JoinHash.

reduced part table (left join input): 3 rows  
reduced lineitem table (right join input): 75'983 rows  
join result table: 1 row  

**We should investigate the message** `[PERF] Only 0 of 61 chunks scanned using an index at src/lib/operators/join_index.cpp:173`.

```
Running ./hyriseMicroBenchmarks
Run on (32 X 2500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x32)
  L1 Instruction 32K (x32)
  L2 Unified 256K (x32)
  L3 Unified 25600K (x32)
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables
- Loading/Generating tables done (20 s 246 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (1 ms 315 µs)
-  Encoding 'orders' - encoding applied (786 ms 808 µs)
-  Encoding 'region' - encoding applied (576 µs 357 ns)
-  Encoding 'part' - encoding applied (490 ms 186 µs)
-  Encoding 'lineitem' - encoding applied (1 s 718 ms)
-  Encoding 'partsupp' - encoding applied (437 ms 163 µs)
-  Encoding 'customer' - encoding applied (615 ms 865 µs)
-  Encoding 'supplier' - encoding applied (42 ms 725 µs)
- Encoding tables done (4 s 94 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (339 µs 20 ns)
-  Adding 'orders' (2 s 509 ms)
-  Adding 'region' (33 µs 424 ns)
-  Adding 'part' (319 ms 975 µs)
-  Adding 'lineitem' (11 s 971 ms)
-  Adding 'partsupp' (1 s 441 ms)
-  Adding 'customer' (402 ms 856 µs)
-  Adding 'supplier' (19 ms 863 µs)
- Adding Tables to StorageManager and generating statistics done (16 s 664 ms)
[PERF] get_value() used at src/lib/storage/table.hpp:129
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/reference_segment.cpp:31
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/dictionary_segment.cpp:26
        Performance can be affected. This warning is only shown once.

[PERF] get_value() used at src/lib/storage/table.hpp:129
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/dictionary_segment.cpp:26
        Performance can be affected. This warning is only shown once.

[PERF] get_value() used at src/lib/storage/table.hpp:129
        Performance can be affected. This warning is only shown once.

[PERF] operator[] used at src/lib/storage/dictionary_segment.cpp:26
        Performance can be affected. This warning is only shown once.

------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_data_table_hash_join           933131 ns     933129 ns        649
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_data_table_index_join            8027 ns       8027 ns      80806
TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_hash_join     2898362 ns    2898320 ns        235
[PERF] Only 0 of 61 chunks scanned using an index at src/lib/operators/join_index.cpp:173
        Performance can be affected. This warning is only shown once.

TPCHDataMicroBenchmarkFixture/BM_TPCH_reduced_part_and_reduced_lineitem_reference_table_index_join    2635585 ns    2635556 ns        259

```

#### Table Scan and Index Scan

In these benchmarks, first table `LINEITEM` is scanned using a TableScan with the predicate `l_shipdate >= '1995-09-01'` to get a reference table.
After that the resulting table is scanned with the predicate `l_shipdate < '1995-10-01'`.
Only the execution of the second scan is measured.

Scan input table: 2817779 rows
Scan output table: 75983 rows

The IndexScan is **0,017540869** times faster than the table scan.

```
2019-04-18 06:09:16
Running ./hyriseMicroBenchmarks
Run on (32 X 2500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x32)
  L1 Instruction 32K (x32)
  L2 Unified 256K (x32)
  L3 Unified 25600K (x32)
Generating TPC-H data set with scale factor 1 and Dictionary encoding:
- Loading/Generating tables
- Loading/Generating tables done (20 s 193 ms)
- Encoding tables if necessary
-  Encoding 'nation' - encoding applied (717 µs 924 ns)
-  Encoding 'orders' - encoding applied (476 ms 636 µs)
-  Encoding 'region' - encoding applied (458 µs 507 ns)
-  Encoding 'part' - encoding applied (481 ms 541 µs)
-  Encoding 'lineitem' - encoding applied (1 s 727 ms)
-  Encoding 'partsupp' - encoding applied (534 ms 971 µs)
-  Encoding 'customer' - encoding applied (609 ms 858 µs)
-  Encoding 'supplier' - encoding applied (45 ms 496 µs)
- Encoding tables done (3 s 878 ms)
- Adding Tables to StorageManager and generating statistics
-  Adding 'nation' (165 µs 237 ns)
-  Adding 'orders' (2 s 643 ms)
-  Adding 'region' (38 µs 28 ns)
-  Adding 'part' (321 ms 161 µs)
-  Adding 'lineitem' (12 s 803 ms)
-  Adding 'partsupp' (1 s 512 ms)
-  Adding 'customer' (417 ms 463 µs)
-  Adding 'supplier' (27 ms 446 µs)
- Adding Tables to StorageManager and generating statistics done (17 s 725 ms)
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                                   Time           CPU Iterations
------------------------------------------------------------------------------------------------------------------------------------------
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_reference_table_table_scan                             6392396 ns    6391687 ns        115
TPCHDataMicroBenchmarkFixture/BM_TPCH_lineitem_reference_table_index_scan                           364399910 ns  364388269 ns          2
```
