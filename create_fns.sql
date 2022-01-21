DROP FUNCTION py_eval;
DROP FUNCTION py_hex_to_dec;
DROP FUNCTION py_hex_with_dec;
DROP FUNCTION py_hex_sum;
DROP FUNCTION py_hex_sum_with_dec;

CREATE FUNCTION py_eval RETURNS STRING SONAME 'py_eval.so';
CREATE FUNCTION py_hex_to_dec RETURNS STRING SONAME 'py_eval.so';
CREATE FUNCTION py_hex_with_dec RETURNS STRING SONAME 'py_eval.so';
CREATE AGGREGATE FUNCTION py_hex_sum RETURNS STRING SONAME 'py_eval.so';
CREATE AGGREGATE FUNCTION py_hex_sum_with_dec RETURNS STRING SONAME 'py_eval.so';
