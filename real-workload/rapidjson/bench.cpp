// Real workload: parse + serialize a JSON document repeatedly with RapidJSON.
// Template-heavy: every value type instantiates its own handler code, so the
// hot region is made of many similarly sized functions.
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace rapidjson;

static std::string makeDoc(int n) {
  std::string s = "{\"items\":[";
  char buf[256];
  for (int i = 0; i < n; i++) {
    snprintf(buf, sizeof(buf),
             "%s{\"id\":%d,\"name\":\"item_%d\",\"price\":%.2f,\"tags\":[\"a\",\"b\",\"c\"],"
             "\"active\":%s,\"nested\":{\"x\":%d,\"y\":%d}}",
             i ? "," : "", i, i, (i % 1000) / 7.0, (i % 2) ? "true" : "false", i, i * 2);
    s += buf;
  }
  s += "],\"total\":" + std::to_string(n) + "}";
  return s;
}

int main(int argc, char **argv) {
  int n = argc > 1 ? atoi(argv[1]) : 2000;
  int rounds = argc > 2 ? atoi(argv[2]) : 40;
  std::string doc = makeDoc(n);
  long acc = 0;
  for (int r = 0; r < rounds; r++) {
    Document d;
    d.Parse(doc.c_str(), doc.size());
    if (d.HasParseError()) { fprintf(stderr, "parse error\n"); return 1; }
    // walk values: templated GetInt/GetDouble/String paths
    for (auto &it : d["items"].GetArray()) {
      acc += it["id"].GetInt();
      acc += (long)it["price"].GetDouble();
      acc += (long)it["name"].GetStringLength();
      acc += (long)it["nested"]["y"].GetInt();
      if (it["active"].GetBool()) acc += 3;
    }
    // serialize back
    StringBuffer sb;
    Writer<StringBuffer> w(sb);
    d.Accept(w);
    acc += (long)sb.GetSize();
  }
  printf("%ld\n", acc);
  return 0;
}
