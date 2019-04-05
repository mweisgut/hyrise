### Performance analysis of scan and join operators with index structures

- `BM_TPCH_Q6_lineitem_shipdate_less_predicate_table_scan`  
uses a `TableScan` operator to scan TPC-H table `lineitem` with predicate `l_shipdate < '1995-01-01'`
- `BM_TPCH_Q6_lineitem_shipdate_less_predicate_groupKeyIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `lineitem` with predicate `l_shipdate < '1995-01-01'`.  
Only indices of type `GroupKeyIndex` exist for column `l_shipdate` of table `lineitem`.
- `BM_TPCH_Q6_lineitem_shipdate_less_predicate_bTreeIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `lineitem` with predicate `l_shipdate < '1995-01-01'`.  
Only indices of type `BTreeIndex` exist for column `l_shipdate` of table `lineitem`.
- `BM_TPCH_Q3_orders_orderdate_predicate_table_scan`  
uses a `TableScan` operator to scan TPC-H table `orders` with predicate `o_orderdate < '1995-03-15'`
- `BM_TPCH_Q3_orders_orderdate_predicate_groupKeyIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `orders` with predicate `o_orderdate < '1995-03-15'`.  
Only indices of type `GroupKeyIndex` exist for column `o_orderdate` of table `orders`.
- `BM_TPCH_Q3_orders_orderdate_predicate_bTreeIndex_scan`  
uses an `IndexScan` operator to scan TPC-H table `orders` with predicate `o_orderdate < '1995-03-15'`.  
Only indices of type `BTreeIndex` exist for column `o_orderdate` of table `orders`.
- `BM_TPCH_Q14_part_partkey_predicate_hash_join`  
uses a `JoinHash` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.
- `BM_TPCH_Q14_part_partkey_predicate_nestedLoop_join`  
uses a `JoinNestedLoop` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.
- `BM_TPCH_Q14_part_partkey_predicate_groupKeyIndex_join`  
uses a `JoinIndex` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.  
Only indices of type `GroupKeyIndex` exist for column `l_partkey` of table `lineitem` and for column `p_partkey` of table `part`.
- `BM_TPCH_Q14_part_partkey_predicate_bTreeIndex_join`  
uses a `JoinIndex` operator to join tables `part` and `lineitem` with join predicate `l_partkey = p_partkey`.  
Only indices of type `BTreeIndex` exist for column `l_partkey` of table `lineitem` and for column `p_partkey` of table `part`.
