#### 2019-06-11 #2

SF 1  

version without new rule: `fb3300d8a35ec18ba565022420434421839e906c`
version with new rule: `fd507508ffce336a809ab702f3f26fb732950cc9`

```
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| TPC-H 2        | 4.49600219727  | 270  | 5.10012340546  | 307  | +13%       |                          0.0000 |
| TPC-H 7        | 0.633461356163 | 39   | 0.629884541035 | 38   | -1%        |                          0.0788 |
| TPC-H 8        | 2.67597937584  | 161  | 4.72242021561  | 284  | +76%       |                          0.0000 |
| TPC-H 9        | 0.773651301861 | 47   | 0.831459701061 | 50   | +7%        |                          0.0000 |
| TPC-H 10       | 1.35946428776  | 82   | 1.3601142168   | 82   | +0%        |                          0.8995 |
| TPC-H 11       | 18.5842666626  | 1116 | 18.726852417   | 1124 | +1%        |                          0.0004 |
| TPC-H 17       | 0.807760834694 | 49   | 0.951073527336 | 58   | +18%       |                          0.0000 |
| TPC-H 21       | 0.226015150547 | 14   | 0.216356471181 | 13   | -4%        |                          0.0000 |
| geometric mean |                |      |                |      | +12%       |                                 |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
```

#### 2019-06-11 #1

The reason for the wrong execution of iteration 20 was related with the physical query plan cache: In iteration 20 the used sql string is available in the cache so the PQP is taken from this PQP cache. When the PQP is stored in the cache, `_on_deep_copy` is calles for each operator. Since the swap mechanism was implemented for the JoinIndex, the JoinIndex has a new member variable `_tables_swapped`.
This variable was previously not set for `_on_deep_copy`. Therefore the default value (`false`) was used. 

In this benchmark comparison, this bug is fixed.

SF 1  

version without new rule: `fb3300d8a35ec18ba565022420434421839e906c`
version with new rule: `fd507508ffce336a809ab702f3f26fb732950cc9`

```
+----------------+----------------+------+---------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s   | runs | new iter/s    | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+---------------+------+------------+---------------------------------+
| TPC-H 17       | 0.798545598984 | 48   | 0.94625300169 | 57   | +18%       |                          0.0000 |
| geometric mean |                |      |               |      | +18%       |                                 |
+----------------+----------------+------+---------------+------+------------+---------------------------------+
```

#### 2019-06-09 #4

SF 5, Query 17
Comparison: no additional rule vs IndexJoinPl'Rule + additional Pred'Reord'Rule.  
Same versions as in the previous experiment.  

```
+----------------+-----------------+------+-----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s    | runs | new iter/s      | runs | change [%] | p-value (significant if <0.001) |
+----------------+-----------------+------+-----------------+------+------------+---------------------------------+
| TPC-H 17       | 0.0781910046935 | 5    | 0.0837142467499 | 6    | +7%        |        (not enough runs) 0.0000 |
| geometric mean |                 |      |                 |      | +7%        |                                 |
+----------------+-----------------+------+-----------------+------+------------+---------------------------------+
```

#### 2019-06-09 #3

SF 10, Query 17
Comparison: no additional rule vs IndexJoinPl'Rule + additional Pred'Reord'Rule.  
Same versions as in the previous experiment.  

```
+----------------+-----------------+------+-----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s    | runs | new iter/s      | runs | change [%] | p-value (significant if <0.001) |
+----------------+-----------------+------+-----------------+------+------------+---------------------------------+
| TPC-H 17       | 0.0288852453232 | 2    | 0.0300217363983 | 2    | +4%        |        (not enough runs) 0.0924 |
| geometric mean |                 |      |                 |      | +4%        |                                 |
+----------------+-----------------+------+-----------------+------+------------+---------------------------------+
```

#### 2019-06-09 #2

SF 1, **runs 19**, Query 17
Comparison: no additional rule vs IndexJoinPl'Rule + additional Pred'Reord'Rule.  
Same versions as in the previous experiment.  

This comparison result is much better than result `2019-06-09 #1` because in iteration 20, a failure manipulates the result.
Even if the number of runs is not satisfactory, the result indicates that the JoinIndex improves the performance of query 17.

```
+----------------+--------------+------+----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+--------------+------+----------------+------+------------+---------------------------------+
| TPC-H 17       | 0.769967556  | 19   | 0.953152835369 | 19   | +24%       |     (run time too short) 0.0000 |
| geometric mean |              |      |                |      | +24%       |                                 |
+----------------+--------------+------+----------------+------+------------+---------------------------------+
```

#### 2019-06-09 #1

SF 1, **runs 20**, Query 17  
Comparison: no additional rule vs IndexJoinPl'Rule + additional Pred'Reord'Rule.  
Same versions as in the previous experiment.  

```
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| TPC-H 17       | 0.794617414474 | 20   | 0.572405636311 | 20   | -28%       |     (run time too short) 0.4595 |
| geometric mean |                |      |                |      | -28%       |                                 |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
```
Same versions as in the previous benchmark.

#### 2019-06-04: Effect of additional IndexJoinPlacementRule + additional PredicateReorderingRule

SF 1.0

version without new rules: `7e25d2bd84e1a489364bb443c891de934cf68cb8`  
version with new rules: `26507806bf5a5aa58e4f4f56fab40f3cebe09464` 

 ```
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| TPC-H 2        | 4.53714179993  | 273  | 5.07598209381  | 305  | +12%       |                          0.0000 |
| TPC-H 7        | 0.633953988552 | 39   | 0.637969911098 | 39   | +1%        |                          0.3975 |
| TPC-H 8        | 2.65545225143  | 160  | 4.74879598618  | 285  | +79%       |                          0.0000 |
| TPC-H 9        | 0.781233668327 | 47   | 0.840946435928 | 51   | +8%        |                          0.0000 |
| TPC-H 10       | 1.35322785378  | 82   | 1.34014475346  | 81   | -1%        |                          0.0473 |
| TPC-H 11       | 18.3708400726  | 1103 | 18.0039863586  | 1081 | -2%        |                          0.0000 |
| TPC-H 17       | 0.80807030201  | 49   | 0.384566485882 | 24   | -52%       |                          0.0253 |
| TPC-H 21       | 0.220683857799 | 14   | 0.22509726882  | 14   | +2%        |                          0.0417 |
| geometric mean |                |      |                |      | +0%        |                                 |
+----------------+----------------+------+----------------+------+------------+---------------------------------+

 ```

#### 2019-05-20: Effect of additional IndexJoinPlacementRule + additional PredicateReorderingRule

In this version, the join index table ratio has to be less than 0.1.  

SF 1.0  

version without new rules: `2416724991fc3e3e97bda19793853bf95d69dd84`  
version with new rules: `79afd515ee5b25d6971d1905f8433796fd9446f0`  

| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
|----------------|----------------|------|----------------|------|------------|---------------------------------|
| TPC-H 8        | 2.5950717926   | 156  | 4.74868106842  | 285  | +83%       |                          0.0000 |
| TPC-H 9        | 0.784150958061 | 48   | 0.844296753407 | 51   | +8%        |                          0.0000 |
| geometric mean |                |      |                |      | +40%       |                                 |


|Q ID|LQP left rows|LQP right rows|LQP smaller rows / larger rows|PQP left rows|PQP right rows|PQP smaller rows / larger rows|
|----|-------------|--------------|------------------------------|-------------|--------------|----------------------------|
|8	 |1,333.3  		 |6,001,215			|0,000222172									 |1,362			   |6,001,215			|0,000226954								 |
|9   |20,000		 	 |6,001,215			|0,003332658	 								 |10,922 			 |6,001,215			|0,001819965 								 |


#### 2019-05-20: Effect of additional IndexJoinPlacementRule + additional PredicateReorderingRule

Replaced GroupKeyIndex by BTree Index.  

version without new rules: `8383313322f64030f62e6b325bae618cd8042026`  
version with new rules: `c57809b85283eb110346685e1abba89b34193723`  

SF 1.0   

| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
|----------------|----------------|------|----------------|------|------------|---------------------------------|
| TPC-H 8        | 2.73871946335  | 165  | 0.323936700821 | 20   | -88%       |                          0.0000 |
| TPC-H 9        | 0.771609425545 | 47   | 0.254928290844 | 16   | -67%       |                          0.0000 |
| TPC-H 14       | 12.3342638016  | 741  | 0.248107999563 | 15   | -98%       |                          0.0000 |
| TPC-H 17       | 0.804330229759 | 49   | 0.19298478961  | 12   | -76%       |                          0.0000 |
| TPC-H 19       | 2.84414768219  | 171  | 0.226619407535 | 14   | -92%       |                          0.0000 |
| geometric mean |                |      |                |      | -89%       |                                 |

#### 2019-05-20: Effect of additional IndexJoinPlacementRule + additional PredicateReorderingRule

Now the JoinIndexPlacementRule checks the row counts of the original data tables. Previously the row count of the output tables of a join node's inputs nodes were used.  

version without new rules: `7a7327fe461900155bbf4c24e85d3efb2d2857a8`  
version with new rules: `c57809b85283eb110346685e1abba89b34193723`  

SF 1.0  

##### Query Plan Evaluation

|Query ID|LQP left row count|LQP right row count|LQP smaller row count / larger row count|PQP left row count|PQP right row count|PQP smaller row count / larger row count|
|--------|------------------|-------------------|----------------------------------------|------------------|-------------------|----------------------------------------|
|8			 |200,000					  |6,001,215					|0.0333265847														 |200,000						|6,001,215					|0.0333265847														 |
|9			 |200,000						|6,001,215					|0.0333265847														 |200,000						|6,001,215					|0.0333265847														 |
|14			 |6,001,215					|200,000						|0.0333265847														 |6,001,215					|200,000						|0.0333265847														 |
|17			 |6,001,215					|200,000						|0.0333265847														 |6,001,215					|200,000						|0.0333265847														 |
|19			 |6,001,215					|200,000						|0.0333265847														 |6,001,215					|200,000						|0.0333265847 												|
  

##### Benchmark Results

| Benchmark      | prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
|----------------|----------------|------|----------------|------|------------|---------------------------------|
| TPC-H 8        | 2.73871946335  | 165  | 0.41283261776  | 25   | -85%       |                          0.0000 |
| TPC-H 9        | 0.771609425545 | 47   | 0.312071979046 | 19   | -60%       |                          0.0000 |
| TPC-H 14       | 12.3342638016  | 741  | 0.270524173975 | 17   | -98%       |                          0.0000 |
| TPC-H 17       | 0.804330229759 | 49   | 0.21412576735  | 13   | -73%       |                          0.0000 |
| TPC-H 19       | 2.84414768219  | 171  | 0.246438637376 | 15   | -91%       |                          0.0000 |
| geometric mean |                |      |                |      | -87%       |                                 |

#### Effect of additional IndexJoinPlacementRule + Additional PredicateReorderingRule

SF 10.0

The same versions as in the previous experiment were used.  
version without IndexJoinPlacementRule: `535c3391ef191f899d5a9d0be889c191ba1a9da8`  
version with IndexJoinPlacementRule: `036c713367b0d3e2e2b67098b1c7187e6c9bd860`  

Benchmark runner limitation: time 120  

```
+----------------+-----------------+------+------------------+------+------------+---------------------------------+
| Benchmark  (JI)| prev. iter/s    | runs | new iter/s       | runs | change [%] | p-value (significant if <0.001) |
+----------------+-----------------+------+------------------+------+------------+---------------------------------+
| TPC-H 1        | 0.0217794291675 | 3    | 0.0219399034977  | 3    | +1%        |        (not enough runs) 0.7375 |
| TPC-H 2      Y | 0.281032115221  | 34   | 0.0379721522331  | 5    | -86%       |        (not enough runs) 0.0000 |
| TPC-H 3      Y | 0.214108929038  | 26   | 0.0263014603406  | 4    | -88%       |        (not enough runs) 0.0000 |
| TPC-H 4        | 0.102194160223  | 13   | 0.102906867862   | 13   | +1%        |                          0.5527 |
| TPC-H 5      Y | 0.123629011214  | 15   | 0.123487263918   | 15   | -0%        |                          0.8346 |
| TPC-H 6        | 0.382442891598  | 46   | 0.40026023984    | 49   | +5%        |                          0.0007 |
| TPC-H 7        | 0.0415439866483 | 5    | 0.0413809865713  | 5    | -0%        |        (not enough runs) 0.6523 |
| TPC-H 8      Y | 0.241899907589  | 30   | 0.00558691937476 | 1    | -98%       |           (not enough runs) nan |
| TPC-H 9      Y | 0.0534801147878 | 7    | 0.0050798994489  | 1    | -91%       |           (not enough runs) nan |
| TPC-H 10       | 0.0999271124601 | 12   | 0.0918122753501  | 12   | -8%        |                          0.0000 |
| TPC-H 11       | 1.32582592964   | 160  | 1.3329526186     | 160  | +1%        |                          0.2013 |
| TPC-H 12       | 0.362900882959  | 44   | 0.334512799978   | 41   | -8%        |                          0.0000 |
| TPC-H 13       | 0.0735384523869 | 9    | 0.0742614120245  | 9    | +1%        |        (not enough runs) 0.6456 |
| TPC-H 14     Y | 0.772601246834  | 93   | 0.00435410346836 | 1    | -99%       |           (not enough runs) nan |
| TPC-H 15       | 0.622954368591  | 75   | 0.615468323231   | 74   | -1%        |                          0.0007 |
| TPC-H 16       | 0.360707849264  | 44   | 0.361891001463   | 44   | +0%        |                          0.5046 |
| TPC-H 17     Y | 0.0295911300927 | 4    | 0.00386895751581 | 1    | -87%       |           (not enough runs) nan |
| TPC-H 18       | 0.081690967083  | 10   | 0.0846856981516  | 11   | +4%        |                          0.0335 |
| TPC-H 19       | 0.236484691501  | 29   | 0.237277761102   | 29   | +0%        |                          0.2455 |
| TPC-H 20       | 0.0644487515092 | 8    | 0.0648381859064  | 8    | +1%        |        (not enough runs) 0.1189 |
| TPC-H 21       | 0.0136448312551 | 2    | 0.0139385713264  | 2    | +2%        |        (not enough runs) 0.4019 |
| TPC-H 22       | 0.43804743886   | 53   | 0.438911646605   | 53   | +0%        |                          0.6113 |
| geometric mean |                 |      |                  |      | -55%       |                                 |
+----------------+-----------------+------+------------------+------+------------+---------------------------------+
```

#### Effect of additional IndexJoinPlacementRule + Additional PredicateReorderingRule

SF 1.0

In this version, the projection operator was modified. Now a projection forwards single-column indices.  

version without IndexJoinPlacementRule: `535c3391ef191f899d5a9d0be889c191ba1a9da8`  
version with IndexJoinPlacementRule: `036c713367b0d3e2e2b67098b1c7187e6c9bd860`  

```
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| Benchmark  (IJ)| prev. iter/s   | runs | new iter/s     | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+----------------+------+------------+---------------------------------+
| TPC-H 1        | 0.223961099982 | 12   | 0.225101903081 | 12   | +1%        |     (run time too short) 0.0824 |
| TPC-H 2      Y | 4.76732110977  | 239  | 2.261033535    | 114  | -53%       |     (run time too short) 0.0000 |
| TPC-H 3      Y | 3.11760139465  | 156  | 1.44704437256  | 73   | -54%       |     (run time too short) 0.0000 |
| TPC-H 4        | 1.13301408291  | 57   | 1.15646612644  | 58   | +2%        |     (run time too short) 0.0000 |
| TPC-H 5      Y | 2.41769242287  | 121  | 2.3869125843   | 120  | -1%        |     (run time too short) 0.0414 |
| TPC-H 6        | 3.95523047447  | 198  | 4.08140563965  | 205  | +3%        |     (run time too short) 0.0000 |
| TPC-H 7        | 0.652304768562 | 33   | 0.65274065733  | 33   | +0%        |     (run time too short) 0.8440 |
| TPC-H 8      Y | 2.77189207077  | 139  | 0.433254539967 | 22   | -84%       |     (run time too short) 0.0000 |
| TPC-H 9      Y | 0.784137785435 | 40   | 0.303733110428 | 16   | -61%       |     (run time too short) 0.0000 |
| TPC-H 10       | 1.4014621973   | 71   | 1.39813327789  | 70   | -0%        |     (run time too short) 0.4125 |
| TPC-H 11       | 18.8957977295  | 945  | 18.8761768341  | 944  | -0%        |     (run time too short) 0.7094 |
| TPC-H 12       | 3.99219942093  | 200  | 4.03843784332  | 202  | +1%        |     (run time too short) 0.0003 |
| TPC-H 13       | 1.24678170681  | 63   | 1.24492871761  | 63   | -0%        |     (run time too short) 0.8566 |
| TPC-H 14     Y | 12.4258356094  | 622  | 0.270688325167 | 14   | -98%       |     (run time too short) 0.0000 |
| TPC-H 15       | 8.15819835663  | 408  | 8.31071376801  | 416  | +2%        |     (run time too short) 0.0000 |
| TPC-H 16       | 3.65528178215  | 183  | 3.6699488163   | 184  | +0%        |     (run time too short) 0.1626 |
| TPC-H 17     Y | 0.812508285046 | 41   | 0.212832629681 | 11   | -74%       |     (run time too short) 0.0000 |
| TPC-H 18       | 0.885608375072 | 45   | 0.874071121216 | 44   | -1%        |     (run time too short) 0.1693 |
| TPC-H 19       | 2.80154371262  | 141  | 2.83348155022  | 142  | +1%        |     (run time too short) 0.0000 |
| TPC-H 20       | 1.04819071293  | 53   | 1.04829239845  | 53   | +0%        |     (run time too short) 0.9428 |
| TPC-H 21       | 0.228599146008 | 12   | 0.231115072966 | 12   | +1%        |     (run time too short) 0.0010 |
| TPC-H 22       | 5.37617111206  | 269  | 5.3461432457   | 268  | -1%        |     (run time too short) 0.0201 |
| geometric mean |                |      |                |      | -35%       |                                 |
+----------------+----------------+------+----------------+------+------------+---------------------------------+

```

#### Further experiments for IndexJoin queries (2,3,5,8,9,14,17)

All queries fall back to the nested loop join.  
Queries for which two projections are the JoinIndex inputs:  
2,3,5,8,9,14,17 --> All JoinIndex-relevant queries have projections as input. Therefore
the original data tables are not accessed --> the accessed tables don't have indices.

#### No IndexJoinPlacementRule vs IndexJoinPlacementRule + Additional PredicateReorderingRule V2

commit id: `46d6a6a85e64c6767478479af0fdef1f4926b7fd`

```
+----------------+----------------+------+-------------------+------+------------+---------------------------------+
| Benchmark  (IJ)| prev. iter/s   | runs | new iter/s        | runs | change [%] | p-value (significant if <0.001) |
+----------------+----------------+------+-------------------+------+------------+---------------------------------+
| TPC-H 1        | 0.21275626123  | 13   | 0.222574949265    | 14   | +5%        |                          0.0000 |
| TPC-H 2     Y  | 4.32471132278  | 260  | 0.00283059524372  | 1    | -100%      |           (not enough runs) nan |
| TPC-H 3     Y  | 3.05668640137  | 184  | 0.00144365651067  | 1    | -100%      |           (not enough runs) nan |
| TPC-H 4        | 1.09300518036  | 66   | 1.14908874035     | 69   | +5%        |                          0.0000 |
| TPC-H 5     Y  | 2.06358098984  | 124  | 2.44435358047     | 147  | +18%       |                          0.0000 |
| TPC-H 6        | 3.89955830574  | 234  | 3.92650437355     | 236  | +1%        |                          0.2612 |
| TPC-H 7        | 0.629360556602 | 38   | 0.648810565472    | 39   | +3%        |                          0.0000 |
| TPC-H 8     Y  | 2.29891943932  | 138  | 0.000246844399953 | 1    | -100%      |           (not enough runs) nan |
| TPC-H 9     Y  | 0.710052847862 | 43   | 0.00024558705627  | 1    | -100%      |           (not enough runs) nan |
| TPC-H 10       | 1.36736500263  | 83   | 1.39022612572     | 84   | +2%        |                          0.0000 |
| TPC-H 11       | 13.5454950333  | 813  | 18.7914962769     | 1128 | +39%       |                          0.0000 |
| TPC-H 12       | 3.47885560989  | 209  | 3.9256939888      | 236  | +13%       |                          0.0000 |
| TPC-H 13       | 1.20316195488  | 73   | 1.22741651535     | 74   | +2%        |                          0.0062 |
| TPC-H 14    Y  | 11.7121486664  | 703  | 0.000374269176973 | 1    | -100%      |           (not enough runs) nan |
| TPC-H 15       | 7.85988664627  | 472  | 8.01616764069     | 481  | +2%        |                          0.0000 |
| TPC-H 16       | 3.39795780182  | 204  | 3.72948122025     | 224  | +10%       |                          0.0000 |
| TPC-H 17    Y  | 0.675080537796 | 41   | 0.000373923248844 | 1    | -100%      |           (not enough runs) nan |
| TPC-H 18       | 0.746687352657 | 45   | 0.86581158638     | 52   | +16%       |                          0.0000 |
| TPC-H 19       | 2.6254491806   | 158  | 2.8420484066      | 171  | +8%        |                          0.0000 |
| TPC-H 20       | 1.0215395689   | 62   | 1.03491866589     | 63   | +1%        |                          0.0000 |
| TPC-H 21       | 0.223747879267 | 14   | 0.229144528508    | 14   | +2%        |                          0.0000 |
| TPC-H 22       | 5.24645805359  | 315  | 5.3048620224      | 319  | +1%        |                          0.0000 |
| geometric mean |                |      |                   |      | -89%       |                                 |
+----------------+----------------+------+-------------------+------+------------+---------------------------------+
```

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
