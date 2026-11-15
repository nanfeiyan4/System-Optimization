# CPU-bound Python workload: dict churn, string ops, list comprehension.
d = {}
for i in range(300000):
    k = i % 5000
    d[k] = d.get(k, 0) + (i % 97)
s = 0
for k, v in d.items():
    s += v
lst = [str(x) for x in range(60000)]
joined = ",".join(lst)
parts = joined.split(",")
words = [p for p in parts if len(p) % 2 == 0]
total = sum(len(w) for w in words)
print(s, total)
