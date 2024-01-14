#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>


#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

// THE FOLLOWING CODE IS EXTRACTED FROM UPSTREAM REPO'S README

int main ()
{
  {
    // create an empty structure (null)
    json j;

    // add a number that is stored as double (note the implicit conversion of j to an object)
    j["pi"] = 3.141;

    // add a Boolean that is stored as bool
    j["happy"] = true;

    // add a string that is stored as std::string
    j["name"] = "Niels";

    // add another null object by passing nullptr
    j["nothing"] = nullptr;

    // add an object inside the object
    j["answer"]["everything"] = 42;

    // add an array that is stored as std::vector (using an initializer list)
    j["list"] = { 1, 0, 2 };

    // add another object (using an initializer list of pairs)
    j["object"] = { {"currency", "USD"}, {"value", 42.99} };
  }

  {
    // instead, you could also write (which looks very similar to the JSON above)
    json j2 = {
      {"pi", 3.141},
      {"happy", true},
      {"name", "Niels"},
      {"nothing", nullptr},
      {"answer", {
        {"everything", 42}
      }},
      {"list", {1, 0, 2}},
      {"object", {
        {"currency", "USD"},
        {"value", 42.99}
      }}
    };

  }
  
  {
    // a way to express the empty array []
    json empty_array_explicit = json::array();

    // ways to express the empty object {}
    json empty_object_implicit = json({});
    json empty_object_explicit = json::object();

    // a way to express an _array_ of key/value pairs [["currency", "USD"], ["value", 42.99]]
    json array_not_object = json::array({ {"currency", "USD"}, {"value", 42.99} });
  }

  {
    // create object from string literal
    json j = "{ \"happy\": true, \"pi\": 3.141 }"_json;

    // or even nicer with a raw string literal
    auto j2 = R"(
      {
        "happy": true,
        "pi": 3.141
      }
    )"_json;

    // parse explicitly
    auto j3 = json::parse("{ \"happy\": true, \"pi\": 3.141 }");
    
    // explicit conversion to string
    std::string s = j.dump();    // {\"happy\":true,\"pi\":3.141}

    // serialization with pretty printing
    // pass in the amount of spaces to indent
    std::cout << j.dump(4) << std::endl;
    // {
    //     "happy": true,
    //     "pi": 3.141
    // }
  }
  
  {
    // store a string in a JSON value
    json j_string = "this is a string";

    // retrieve the string value
    auto cpp_string = j_string.get<std::string>();
    // retrieve the string value (alternative when an variable already exists)
    std::string cpp_string2;
    j_string.get_to(cpp_string2);

    // retrieve the serialized value (explicit JSON serialization)
    std::string serialized_string = j_string.dump();

    // output of original string
    std::cout << cpp_string << " == " << cpp_string2 << " == " << j_string.get<std::string>() << '\n';
    // output of serialized value
    std::cout << j_string << " == " << serialized_string << std::endl;
  }

  // {
  //   // deserialize from standard input
  //   json j;
  //   std::cin >> j;

  //   // serialize to standard output
  //   std::cout << j;

  //   // the setw manipulator was overloaded to set the indentation for pretty printing
  //   std::cout << std::setw(4) << j << std::endl;
  // }

  // {
  //   // read a JSON file
  //   std::ifstream i("file.json");
  //   json j;
  //   i >> j;

  //   // write prettified JSON to another file
  //   std::ofstream o("pretty.json");
  //   o << std::setw(4) << j << std::endl;
  // }

  {
    // create an array using push_back
    json j;
    j.push_back("foo");
    j.push_back(1);
    j.push_back(true);

    // also use emplace_back
    j.emplace_back(1.78);

    // iterate the array
    for (json::iterator it = j.begin(); it != j.end(); ++it) {
      std::cout << *it << '\n';
    }

    // range-based for
    for (auto& element : j) {
      std::cout << element << '\n';
    }

    // getter/setter
    const auto tmp = j[0].get<std::string>();
    j[1] = 42;
    bool foo = j.at(2);

    // comparison
    j == "[\"foo\", 1, true]"_json;  // true

    // other stuff
    j.size();     // 3 entries
    j.empty();    // false
    j.type();     // json::value_t::array
    j.clear();    // the array is empty again

    // convenience type checkers
    j.is_null();
    j.is_boolean();
    j.is_number();
    j.is_object();
    j.is_array();
    j.is_string();

    // create an object
    json o;
    o["foo"] = 23;
    o["bar"] = false;
    o["baz"] = 3.141;

    // also use emplace
    o.emplace("weather", "sunny");

    // special iterator member functions for objects
    for (json::iterator it = o.begin(); it != o.end(); ++it) {
      std::cout << it.key() << " : " << it.value() << "\n";
    }

    // the same code as range for
    for (auto& el : o.items()) {
      std::cout << el.key() << " : " << el.value() << "\n";
    }

    // even easier with structured bindings (C++17)
    for (auto& [key, value] : o.items()) {
      std::cout << key << " : " << value << "\n";
    }

    // find an entry
    if (o.find("foo") != o.end()) {
      // there is an entry with key "foo"
    }

    // or simpler using count()
    int foo_present = o.count("foo"); // 1
    int fob_present = o.count("fob"); // 0

    // delete an entry
    o.erase("foo");
  }

  {
    std::vector<int> c_vector {1, 2, 3, 4};
    json j_vec(c_vector);
    // [1, 2, 3, 4]

    std::deque<double> c_deque {1.2, 2.3, 3.4, 5.6};
    json j_deque(c_deque);
    // [1.2, 2.3, 3.4, 5.6]

    std::list<bool> c_list {true, true, false, true};
    json j_list(c_list);
    // [true, true, false, true]

    std::forward_list<int64_t> c_flist {12345678909876, 23456789098765, 34567890987654, 45678909876543};
    json j_flist(c_flist);
    // [12345678909876, 23456789098765, 34567890987654, 45678909876543]

    std::array<unsigned long, 4> c_array {{1, 2, 3, 4}};
    json j_array(c_array);
    // [1, 2, 3, 4]

    std::set<std::string> c_set {"one", "two", "three", "four", "one"};
    json j_set(c_set); // only one entry for "one" is used
    // ["four", "one", "three", "two"]

    std::unordered_set<std::string> c_uset {"one", "two", "three", "four", "one"};
    json j_uset(c_uset); // only one entry for "one" is used
    // maybe ["two", "three", "four", "one"]

    std::multiset<std::string> c_mset {"one", "two", "one", "four"};
    json j_mset(c_mset); // both entries for "one" are used
    // maybe ["one", "two", "one", "four"]

    std::unordered_multiset<std::string> c_umset {"one", "two", "one", "four"};
    json j_umset(c_umset); // both entries for "one" are used
    // maybe ["one", "two", "one", "four"]
  }

  {
    std::map<std::string, int> c_map { {"one", 1}, {"two", 2}, {"three", 3} };
    json j_map(c_map);
    // {"one": 1, "three": 3, "two": 2 }

    std::unordered_map<const char*, double> c_umap { {"one", 1.2}, {"two", 2.3}, {"three", 3.4} };
    json j_umap(c_umap);
    // {"one": 1.2, "two": 2.3, "three": 3.4}

    std::multimap<std::string, bool> c_mmap { {"one", true}, {"two", true}, {"three", false}, {"three", true} };
    json j_mmap(c_mmap); // only one entry for key "three" is used
    // maybe {"one": true, "two": true, "three": true}

    std::unordered_multimap<std::string, bool> c_ummap { {"one", true}, {"two", true}, {"three", false}, {"three", true} };
    json j_ummap(c_ummap); // only one entry for key "three" is used
    // maybe {"one": true, "two": true, "three": true}
  }
  
  {
    // a JSON value
    json j_original = R"({
      "baz": ["one", "two", "three"],
      "foo": "bar"
    })"_json;

    // access members with a JSON pointer (RFC 6901)
    j_original["/baz/1"_json_pointer];
    // "two"

    // a JSON patch (RFC 6902)
    json j_patch = R"([
      { "op": "replace", "path": "/baz", "value": "boo" },
      { "op": "add", "path": "/hello", "value": ["world"] },
      { "op": "remove", "path": "/foo"}
    ])"_json;

    // apply the patch
    json j_result = j_original.patch(j_patch);
    // {
    //    "baz": "boo",
    //    "hello": ["world"]
    // }

    // calculate a JSON patch from two JSON values
    json::diff(j_result, j_original);
    // [
    //   { "op":" replace", "path": "/baz", "value": ["one", "two", "three"] },
    //   { "op": "remove","path": "/hello" },
    //   { "op": "add", "path": "/foo", "value": "bar" }
    // ]
  }

  {
    // a JSON value
    json j_document = R"({
      "a": "b",
      "c": {
        "d": "e",
        "f": "g"
      }
    })"_json;

    // a patch
    json j_patch = R"({
      "a":"z",
      "c": {
        "f": null
      }
    })"_json;

    // apply the patch
    j_document.merge_patch(j_patch);
    // {
    //  "a": "z",
    //  "c": {
    //    "d": "e"
    //  }
    // }
  }

}
