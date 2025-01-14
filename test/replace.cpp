/**
 *   Copyright (C) 2024 T. Zachary Laine
 *
 *   Distributed under the Boost Software License, Version 1.0. (See
 *   accompanying file LICENSE_1_0.txt or copy at
 *   http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/parser/replace.hpp>

#include <gtest/gtest.h>

#include "ill_formed.hpp"

#include <list>

#if !defined(_MSC_VER) || BOOST_PARSER_USE_CONCEPTS

namespace bp = boost::parser;

#if BOOST_PARSER_USE_CONCEPTS
namespace deduction {
    using namespace std::literals;
    std::string str;
    auto const parser = bp::char_;
    auto const skip = bp::ws;

    auto deduced_1 = bp::replace_view(str, parser, skip, "foo", bp::trace::on);
    auto deduced_2 = bp::replace_view(str, parser, skip, "foo");
    auto deduced_3 = bp::replace_view(str, parser, "foo", bp::trace::on);
    auto deduced_4 = bp::replace_view(str, parser, "foo");
}
#endif

static_assert(
    bp::detail::range_utf_format<char const *&>() == bp::detail::no_format);

TEST(replace, either_iterator)
{
    {
        std::list<int> l({1, 2, 3});
        std::vector<int> v({4, 5, 6});
        bp::detail::either_iterator<std::list<int>, std::vector<int>>
            either_l_begin(l.begin());
        bp::detail::either_iterator<std::list<int>, std::vector<int>>
            either_l_end(l.end());
        bp::detail::either_iterator<std::list<int>, std::vector<int>>
            either_v_begin(v.begin());
        bp::detail::either_iterator<std::list<int>, std::vector<int>>
            either_v_end(v.end());

        int const l_array[] = {1, 2, 3};
        auto l_array_curr = l_array;
        for (auto it = either_l_begin; it != either_l_end;
             ++it, ++l_array_curr) {
            EXPECT_EQ(*it, *l_array_curr);
        }

        int const v_array[] = {4, 5, 6};
        auto v_array_curr = v_array;
        for (auto it = either_v_begin; it != either_v_end;
             ++it, ++v_array_curr) {
            EXPECT_EQ(*it, *v_array_curr);
        }
    }
    {
        auto r1 = bp::detail::to_range<decltype("")>::call("");
        auto r2 = bp::detail::to_range<decltype("foo")>::call("foo");

        bp::detail::either_iterator<decltype(r1), decltype(r2)> either_r1_begin(
            r1.begin());
        bp::detail::either_iterator<decltype(r1), decltype(r2)> either_r1_end(
            r1.end());
        bp::detail::either_iterator<decltype(r1), decltype(r2)> either_r2_begin(
            r2.begin());
        bp::detail::either_iterator<decltype(r1), decltype(r2)> either_r2_end(
            r2.end());

        EXPECT_EQ(either_r1_begin, either_r1_end);
        std::string copy;
        for (auto it = either_r2_begin; it != either_r2_end; ++it) {
            copy.push_back(*it);
        }
        EXPECT_EQ(copy, "foo");
    }
}

TEST(replace, replace)
{
    {
        auto r = bp::replace("", bp::lit("XYZ"), bp::ws, "foo");
        int count = 0;
        for (auto subrange : r) {
            (void)subrange;
            ++count;
        }
        EXPECT_EQ(count, 0);
    }
    {
        char const str[] = "aaXYZb";
        auto r = bp::replace(str, bp::lit("XYZ"), bp::ws, "foo");
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "b"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 3);
    }
    {
        char const str[] = "a a XYZ baa ba XYZ";
        auto r =
            str | bp::replace(bp::lit("XYZ"), bp::ws, "foo", bp::trace::off);
        int count = 0;
        std::string_view const strs[] = {"a a ", "foo", " baa ba ", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 4);
    }
#if !defined(__GNUC__) || 12 <= __GNUC__
    // Older GCCs don't like the use of temporaries like the
    // std::string("foo") below.
    {
        char const str[] = "aaXYZbaabaXYZ";
        auto r = str | bp::replace(
                           bp::lit("XYZ"), std::string("foo"), bp::trace::off);
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 4);
    }
#endif
    {
        char const str[] = "aaXYZbaabaXYZ";
        const auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 4);
    }
    {
        char const str[] = "aaXYZbaabaXYZXYZ";
        auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 5);
    }
    {
        char const str[] = "XYZaaXYZbaabaXYZXYZ";
        auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {
            "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 6);
    }
    {
        char const str[] = "XYZXYZaaXYZbaabaXYZXYZ";
        auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {
            "foo", "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 7);
    }
    {
        char const * str = "XYZXYZaaXYZbaabaXYZXYZ";
        char const * replacement = "foo";
        auto r = str | bp::replace(bp::lit("XYZ"), replacement);
        int count = 0;
        std::string_view const strs[] = {
            "foo", "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 7);
    }
    {
        char const * str = "XYZXYZaaXYZbaabaXYZXYZ";
        char const * replacement = "foo";
        auto const r = str | bp::replace(bp::lit("XYZ"), replacement);
        int count = 0;
        std::string_view const strs[] = {
            "foo", "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 7);
    }
}

// MSVC produces hard errors here, so ill_formed does not work.
#if defined(__cpp_char8_t) && !defined(_MSC_VER)
char const empty_str[] = "";

template<typename T>
using char_str_utf8_replacement =
    decltype(std::declval<T>() | bp::replace(bp::lit("XYZ"), std::declval<T>() | bp::as_utf8));
static_assert(ill_formed<char_str_utf8_replacement, decltype(empty_str)>{});

template<typename T>
using char_str_utf16_replacement =
    decltype(std::declval<T>() | bp::replace(bp::lit("XYZ"), std::declval<T>() | bp::as_utf16));
static_assert(ill_formed<char_str_utf16_replacement, decltype(empty_str)>{});

template<typename T>
using utf8_str_char_replacement =
    decltype(std::declval<T>() | bp::as_utf8 | bp::replace(bp::lit("XYZ"), std::declval<T>()));
static_assert(ill_formed<utf8_str_char_replacement, decltype(empty_str)>{});
#endif

TEST(replace, replace_unicode)
{
    {
        char const str_[] = "";
        auto str = str_ | bp::as_utf8;
        auto r = bp::replace(str, bp::lit("XYZ"), bp::ws, "foo" | bp::as_utf8);
        int count = 0;
        for (auto subrange : r) {
            (void)subrange;
            ++count;
        }
        EXPECT_EQ(count, 0);
    }
    {
        char const * str_ = "aaXYZb";
        auto str = str_ | bp::as_utf16;
        auto r = bp::replace(str, bp::lit("XYZ"), bp::ws, "foo" | bp::as_utf16);
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "b"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 3);
    }
    {
        char const str_[] = "aaXYZbaabaXYZ";
        auto str = str_ | bp::as_utf32;
        auto r =
            str |
            bp::replace(
                bp::lit("XYZ"), bp::ws, "foo" | bp::as_utf32, bp::trace::off);
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 4);
    }
    {
        char const str_[] = "aaXYZbaabaXYZ";
        auto str = str_ | bp::as_utf8;
        auto r = str | bp::replace(
                           bp::lit("XYZ"), "foo" | bp::as_utf8, bp::trace::off);
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 4);
    }
    {
        char const str_[] = "aaXYZbaabaXYZ";
        auto str = str_ | bp::as_utf16;
        auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 4);
    }
    {
        char const str_[] = "aaXYZbaabaXYZXYZ";
        auto str = str_ | bp::as_utf32;
        auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {"aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 5);
    }
    {
        char const str_[] = "XYZaaXYZbaabaXYZXYZ";
        auto str = str_ | bp::as_utf8;
        auto r = str | bp::replace(bp::lit("XYZ"), "foo" | bp::as_utf8);
        int count = 0;
        std::string_view const strs[] = {
            "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 6);
    }
    {
        char const str_[] = "XYZXYZaaXYZbaabaXYZXYZ";
        auto str = str_ | bp::as_utf16;
        auto r = str | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        std::string_view const strs[] = {
            "foo", "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 7);
    }
    {
        char const str_[] = "XYZXYZaaXYZbaabaXYZXYZ";
        auto str = str_ | bp::as_utf16;
        auto r = str | bp::replace(bp::lit("XYZ"), "foo" | bp::as_utf8);
        int count = 0;
        std::string_view const strs[] = {
            "foo", "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 7);
    }
    {
        char const str_[] = "XYZXYZaaXYZbaabaXYZXYZ";
        auto str = str_ | bp::as_utf16;
        auto r = str | bp::replace(bp::lit("XYZ"), "foo" | bp::as_utf32);
        int count = 0;
        std::string_view const strs[] = {
            "foo", "foo", "aa", "foo", "baaba", "foo", "foo"};
        for (auto subrange : r) {
            std::string str(subrange.begin(), subrange.end());
            EXPECT_EQ(str, strs[count]);
            ++count;
        }
        EXPECT_EQ(count, 7);
    }
}

#if BOOST_PARSER_USE_CONCEPTS && (!defined(__GNUC__) || 12 <= __GNUC__)
// Older GCCs don't like the use of temporaries like the std::string("foo")
// below.  This causes | join to break.
TEST(replace, join_compat)
{
    {
        char const str[] = "XYZXYZaaXYZbaabaXYZXYZ";
        auto rng = str | bp::as_utf32 |
                   bp::replace(bp::lit("XYZ"), "foo" | bp::as_utf8) |
                   std::views::join;
        std::string replace_result;
        for (auto ch : rng) {
            static_assert(std::is_same_v<decltype(ch), char32_t>);
            replace_result.push_back(ch);
        }
        EXPECT_EQ(replace_result, "foofooaafoobaabafoofoo");
    }

    {
        char const str[] = "XYZXYZaaXYZbaabaXYZXYZ";
        auto rng = str | bp::replace(bp::lit("XYZ"), "foo") | std::views::join;
        std::string replace_result;
        for (auto ch : rng) {
            replace_result.push_back(ch);
        }
        EXPECT_EQ(replace_result, "foofooaafoobaabafoofoo");
    }
    {
        std::string str = "XYZXYZaaXYZbaabaXYZXYZ";
        auto rng = str | bp::replace(bp::lit("XYZ"), "foo") | std::views::join;
        std::string replace_result;
        for (auto ch : rng) {
            replace_result.push_back(ch);
        }
        EXPECT_EQ(replace_result, "foofooaafoobaabafoofoo");
    }
    {
        std::string const str = "XYZXYZaaXYZbaabaXYZXYZ";
        auto rng = str | bp::replace(bp::lit("XYZ"), "foo") | std::views::join;
        std::string replace_result;
        for (auto ch : rng) {
            replace_result.push_back(ch);
        }
        EXPECT_EQ(replace_result, "foofooaafoobaabafoofoo");
    }
    {
        auto rng = std::string("XYZXYZaaXYZbaabaXYZXYZ") |
                   bp::replace(bp::lit("XYZ"), "foo") | std::views::join;
        std::string replace_result;
        for (auto ch : rng) {
            replace_result.push_back(ch);
        }
        EXPECT_EQ(replace_result, "foofooaafoobaabafoofoo");
    }
}
#endif

TEST(replace, doc_examples)
{
    {
        auto rng = "XYZaaXYZbaabaXYZXYZ" | bp::replace(bp::lit("XYZ"), "foo");
        int count = 0;
        // Prints foo aa foo baaba foo foo.
        for (auto subrange : rng) {
            std::cout << std::string_view(subrange.begin(), subrange.end() - subrange.begin()) << " ";
            ++count;
        }
        std::cout << "\n";
        assert(count == 6);
    }
}

#endif
