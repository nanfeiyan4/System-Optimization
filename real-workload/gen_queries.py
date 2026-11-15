#!/usr/bin/env python3
"""Generate the mixed SQL workload used for the real-workload check."""
import sys

N = int(sys.argv[1]) if len(sys.argv) > 1 else 300
out = sys.argv[2] if len(sys.argv) > 2 else "queries.sql"

stmts = []
for i in range(N):
    k = i % 6
    if k == 0:
        stmts.append("SELECT count(*) FROM t WHERE c BETWEEN %d AND %d;" % (i % 100, i % 100 + 10))
    elif k == 1:
        stmts.append("SELECT a,b FROM t WHERE a = %d;" % (i * 13 % 40000 + 1))
    elif k == 2:
        stmts.append("SELECT sum(length(b)) FROM t WHERE c > %d;" % (i % 120))
    elif k == 3:
        stmts.append("SELECT count(*) FROM t JOIN u ON u.id = (t.a %% 8000) WHERE t.c < %d;" % (i % 140))
    elif k == 4:
        stmts.append("SELECT max(a), min(a), avg(c) FROM t WHERE b LIKE 'name_%d%%';" % (i % 900))
    else:
        stmts.append("SELECT u.s, count(*) FROM t JOIN u ON u.id = (t.a % 8000) GROUP BY u.s HAVING count(*) > 3 LIMIT 5;")

open(out, "w").write("\n".join(stmts) + "\n")
print("wrote %d statements to %s" % (len(stmts), out))
