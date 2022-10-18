# minisquel - A minimalistic plaintext sql interpreter

This implements a simple SQL:ish interpreter. It's more of a toy/proof-of-concept doing an SQL-style interpreter from first principles. It takes the same naive/simple approach to interpretation as early BASICs did: It re-parses and interprets the source code *every* time. Is it efficient? No. Was it easy to write? Yes! So far...

## GOALS

- "minimal" in code/complexity (<1000 lines code)
- fast hack for fun
- do the simplest for the moment to add
- "not for professional use" :-D
- no depencies other than linux/posix
- limited sql
        [FORMAT (CSV|TAB|BAR)]
	SELECT count(money) xxx AS x, ...
	[FROM fil.csv AS fil
 	 [JOIN foo.csv AS foo ON fil.id=foo.id]]
	[WHERE ...
          AND ...
	  OR NOT ...
	  GROUP BY 1,2]
- sql *INTERPRETER* (means no internal structures/parse-tree/interpreter internal representation)
- parse text files (csv, tab, plain, json, xml)
- limited aggregation (full/group by on sorted data)
- some variant of merge-join (on sorted data)

## NON-goals
- NO: full standard SQL (we go for subset)
- NO:high optimial efficency (go download DuckDB instead!)
- NO: tokenizer/parser-tree/internal represenation
- NO: optimizer
- NO: more datatypes (but funtions on strings: xml/json etc)
- NO: ODBC, NO: JDBC - no fxxing way!
- NO: stored prepared queries

## Working Examples

Calculations

    select 1+3*4   => 13
    
Naming and using columns (EXTENSION!)

    select 42 as ft, ft+7, ft*ft

complex where clause

    select 42 where 0=3*4 or 1=1 and 2=2
    
Integer iterator (EXTENSION!):

    select i, i*i from int(1,10) i where i*i>10

CSV file querying (EXTENSION!):

    select a,b,c from foo.csv foo

Function calling

    select "abba" as f, char(ascii(upper(f)))

Joins!

    select int.a, b, a*int.b
    from int(1,10) a, int(1,10) b
    where a=b
    
select int.a, b, a*int.b from int(1,10) a, int(1,10) b where a=b
    
## Performance?
- actually, not too bad!
- 22MB of csv w 108K records take < 1s to scan!
- 1000x1000 iterations <1s (1.4M "where"-evals)
- opening/close same file 100,000 times (in nested loop) almost no overhead!

## Current Features
- row by row processing
- refer to columns by name or table.col
- plain-text CSV/TAB-file querying
- cross-product join (no JOIN ... ON)
- undefined/not present variables are NULL
- NULL is always null if not set, LOL ("feature")
- double as only numeric type (print %lg)
- string as "other type"
- $lineno of generated out-lines
- generic iterator/generator in from-clause
- built in INT iterator
- no optimization
- order of data in kept in result
- (only?) nested loop evaluation
- WHERE ... AND ... OR NOT ...
- variable stats data used for aggregates:
- COUNT(), SUM(), MIN(), MAX(), AVG(), DEV(), 
- log queries/statistics
- aggregators only work on variables, not computed values:
       doesn't work (currently):

         select sum(1) from int(1,10) i

       workaround:
       
         select 1 as foo, sumfoo) fror int(1,10) i


## functions
- mod div
- ascii char lower upper
- count sum min max avg stdev
- type

## TODO:
- LIKE/REGEXP
- select *,tab.*
- CREATEF FUNCTION Bigtable UDF:s - https://cloud.google.com/bigquery/docs/reference/standard-sql/user-defined-functions
- val with altname/num/table.col
- upper/lower-case for "sql"
- where not/and/or...
- functions in expressions
  - date/time functions (on strings)
  - json/xml extract val from string
  - more math?
- xml querying
- json querying
- yaml querying
- flatfile querying
- APPEND (not UPDATE?) == SELECT ... INTO ... ?
- BEGIN...END transactional over several files?
- index files=="create view" (auto-invalidate on update)
- consider connecting with user pipes and external programs like sql_orderby? or for nested queries, at least in "from"
- tab JOIN tab USING(c, ...)
- Query 22 Mb external URL
      SELECT ...
      FROM  https://raw.githubusercontent.com/megagonlabs/HappyDB/master/happydb/data/cleaned_hm.csv AS happy
      download, cache

      Possibly an specific

      IMPORT TABLE happy FROM https://...

## NOT todo
- optimizer (reordering)
- row values/tuples
- more datatypes (just keep as string)
- Ordering and/or Aggregation with GROUP BY. Basically this would either have to use memory or temporary files. Not such good idea. Maybe generate set and then call unix "sort" with right arguments/types as ORDER BY, then could do GROUP BY.
- GROUP BY (unless sorted input?)
- ORDER BY
