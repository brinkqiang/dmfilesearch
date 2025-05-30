#include <string>
#include <vector>

#include "gtest.h"
#include "ini/ini.h"
#include "dmutil.h"

using namespace inih;

TEST(INIReader, get_sections) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    const std::set<std::string> ans = {"section1", "section2"};

    EXPECT_EQ(r.Sections(), ans);
}

TEST(INIReader, get_keys) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    const std::set<std::string> ans = {"any", "any2", "not_int", "not_int_arr"};

    EXPECT_EQ(r.Keys("section1"), ans);
}

TEST(INIReader, get_single_value) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    EXPECT_EQ(r.Get<>("section1", "any"), std::string("1"));
    EXPECT_EQ(r.Get<float>("section1", "any"), float(1.0));
    EXPECT_EQ(r.Get<double>("section1", "any"), double(1.0));
    EXPECT_EQ(r.Get<long>("section1", "any"), long(1));
    EXPECT_EQ(r.Get<unsigned long>("section1", "any"), (unsigned long)(1));

    EXPECT_EQ(r.Get("section1", "any2"), std::string("true"));
    EXPECT_EQ(r.Get<bool>("section1", "any2"), true);
}

TEST(INIReader, get_vector) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    const std::vector<int> ans1{1, 2, 3};
    const std::vector<std::string> ans2{"1", "2", "3"};

    ASSERT_EQ(r.GetVector<int>("section2", "any_vec"), ans1);
    ASSERT_EQ(r.GetVector<>("section2", "any_vec"), ans2);
}

TEST(INIReader, get_single_value_with_default) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    EXPECT_EQ(r.Get<std::string>("section1", "not_exist", "1"),
              std::string("1"));
    EXPECT_EQ(r.Get<int>("section1", "not_exist", 1), int(1));
    EXPECT_EQ(r.Get<float>("section1", "not_exist", 1), float(1.0));
    EXPECT_EQ(r.Get<double>("section1", "not_exist", 1), double(1.0));
    EXPECT_EQ(r.Get<long>("section1", "not_exist", 1), long(1));
    EXPECT_EQ(r.Get<unsigned long>("section1", "not_exist", 1),
              (unsigned long)(1));
    EXPECT_EQ(r.Get<bool>("section1", "not_exist", true), true);

    EXPECT_EQ(r.Get<bool>("section1", "any2", false), true);
}

TEST(INIReader, get_vector_with_default) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    const std::vector<int> ans1{1, 2, 3};
    const std::vector<std::string> ans2{"1", "2", "3", "4"};
    const std::vector<double> ans3{1.23, 4.56};

    const auto& vec1 = r.GetVector<int>("section2", "not_exist", ans1);
    const auto& vec2 = r.GetVector<std::string>(
        "section2", "not_exist", std::vector<std::string>{"1", "2", "3", "4"});
    const auto& vec3 =
        r.GetVector<double>("section2", "doubles", std::vector<double>{0});

    ASSERT_EQ(vec1, ans1);
    ASSERT_EQ(vec2, ans2);
    ASSERT_EQ(vec3, ans3);
}

TEST(INIReader, exception) {
    EXPECT_THROW(INIReader{"QQ"}, std::runtime_error);  // file not found
    EXPECT_THROW(INIReader{DMGetRootPath() + "/../../config/fixtures/bad_file.ini"},
                 std::runtime_error);  // parse error

    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    // section not found error
    EXPECT_THROW(r.Get("section3"), std::runtime_error);

    // key not found error
    EXPECT_THROW(r.Get<int>("section1", "not_exist"), std::runtime_error);
    EXPECT_THROW(r.GetVector<int>("section1", "not_exist"), std::runtime_error);

    // parse error
    EXPECT_THROW(r.Get<int>("section1", "not_int"), std::runtime_error);
    EXPECT_THROW(r.Get<bool>("section1", "not_int"), std::runtime_error);
    EXPECT_THROW(r.GetVector<int>("section1", "not_int_arr"),
                 std::runtime_error);
}

TEST(INIReader, read_big_file) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/bigfile.ini"};

    for (int i = 1; i <= 1000; ++i) {
        const auto& v = r.Get<int>("section", "key" + std::to_string(i));
        EXPECT_EQ(v, i);
    }
}

TEST(INIReader, dulicate_keys) {
    EXPECT_THROW(INIReader{DMGetRootPath() + "/../../config/fixtures/duplicate_keys.ini"},
                 std::runtime_error);
}

TEST(INIReader, InsertEntry) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};

    // section exist, key not exist
    r.InsertEntry("section1", "my_custom_key", "hello world");

    // section&key not exist
    r.InsertEntry("new_section", "key1", 5);

    EXPECT_EQ("hello world", r.Get("section1", "my_custom_key"));
    EXPECT_EQ(5, r.Get<int>("new_section", "key1"));
}

TEST(INIReader, UpdateEntry) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};
    r.InsertEntry("section1", "my_custom_key", "hello world");

    r.UpdateEntry("section1", "my_custom_key", 123);
    EXPECT_EQ(123, r.Get<int>("section1", "my_custom_key"));

    std::vector<double> ans1{0.1, 0.2, 0.3};
    r.UpdateEntry("section1", "my_custom_key", ans1);
    for (size_t i = 0; i < ans1.size(); ++i) {
        EXPECT_EQ(ans1[i], r.GetVector<double>("section1", "my_custom_key")[i]);
    }
}

TEST(INIWriter, write) {
    std::string ini_name = DMGetRootPath() + "/../../config/fixtures/config.ini";
    INIReader r{ ini_name };
    r.InsertEntry("new_section", "key1", "123");
    r.InsertEntry("new_section", "key2", 5.5);
    r.InsertEntry("new_section", "key3", std::vector<double>{0.1, 0.2, 0.3});
    r.InsertEntry("new_section", "key4", std::vector<std::string>{"a", "b"});

    std::string new_ini_name = DMGetRootPath() + "/output.ini";

	unlink(new_ini_name.data());
    INIWriter::write(new_ini_name, r);

    INIReader r2{ new_ini_name };
    for (const auto& section : r.Sections()) {
        for (const auto& key : r.Keys(section)) {
            EXPECT_EQ(r.Get(section, key), r2.Get(section, key));
        }
    }
    unlink(new_ini_name.data());
}

TEST(INIWriter, exception) {
    INIReader r{DMGetRootPath() + "/../../config/fixtures/config.ini"};
    EXPECT_THROW(INIWriter::write(DMGetRootPath() + "/../../config/fixtures/config.ini", r), std::runtime_error);
}