#include <sp2/string.h>
#include "doctest.h"

using sp::string;

TEST_CASE("strings")
{
    CHECK(string("hello") == string("hello").substr());
    CHECK(string("hel") == string("hello").substr(0, 3));
    CHECK(string("he") == string("hello").substr(0, -3));
    CHECK(string("llo") == string("hello").substr(-3));
    
    //test_capitalize
    CHECK(string(" hello ") == string(" hello ").capitalize());
    CHECK(string("Hello ") == string("Hello ").capitalize());
    CHECK(string("Hello ") == string("hello ").capitalize());
    CHECK(string("Aaaa") == string("aaaa").capitalize());
    CHECK(string("Aaaa") == string("AaAa").capitalize());

    //test_count
    CHECK(3 == string("aaa").count("a"));
    CHECK(0 == string("aaa").count("b"));
    CHECK(3 == string("aaa").count("a"));
    CHECK(0 == string("aaa").count("b"));
    CHECK(3 == string("aaa").count("a"));
    CHECK(0 == string("aaa").count("b"));
    CHECK(0 == string("aaa").count("b"));

    CHECK(1 == string("").count(""));

    CHECK(0 == string("").count("xx"));

    //test_find
    CHECK(0 == string("abcdefghiabc").find("abc"));
    CHECK(9 == string("abcdefghiabc").find("abc", 1));
    CHECK(-1 == string("abcdefghiabc").find("def", 4));

    CHECK(0 == string("abc").find("", 0));
    CHECK(3 == string("abc").find("", 3));
    CHECK(-1 == string("abc").find("", 4));

    CHECK(0 == string("").find(""));
    CHECK(-1 == string("").find("xx"));

    //test_rfind
    CHECK(9 == string("abcdefghiabc").rfind("abc"));
    CHECK(12 == string("abcdefghiabc").rfind(""));
    CHECK(0 ==  string("abcdefghiabc").rfind("abcd"));
    CHECK(-1 == string("abcdefghiabc").rfind("abcz"));

    CHECK(3 == string("abc").rfind("", 0));
    CHECK(3 == string("abc").rfind("", 3));
    CHECK(-1 == string("abc").rfind("", 4));

    //test_lower
    CHECK(string("hello") == string("HeLLo").lower());
    CHECK(string("hello") == string("hello").lower());

    //test_upper
    CHECK(string("HELLO") == string("HeLLo").upper());
    CHECK(string("HELLO") == string("HELLO").upper());

    //test_expandtabs
    CHECK(string("a       bc") == string("a\tbc").expandtabs());
    CHECK(string("abc\rab      def") == string("abc\rab\tdef").expandtabs());
    CHECK(string("abc\rab      def\ng       hi") == string("abc\rab\tdef\ng\thi").expandtabs());
    CHECK(string("abc\rab      def\ng       hi") == string("abc\rab\tdef\ng\thi").expandtabs(8));
    CHECK(string("abc\rab  def\ng   hi") == string("abc\rab\tdef\ng\thi").expandtabs(4));
    CHECK(string("abc\r\nab  def\ng   hi") == string("abc\r\nab\tdef\ng\thi").expandtabs(4));
    CHECK(string("abc\rab      def\ng       hi") == string("abc\rab\tdef\ng\thi").expandtabs());
    CHECK(string("abc\rab      def\ng       hi") == string("abc\rab\tdef\ng\thi").expandtabs(8));
    CHECK(string("abc\r\nab\r\ndef\ng\r\nhi") == string("abc\r\nab\r\ndef\ng\r\nhi").expandtabs(4));
    CHECK(string("  a\n b") == string(" \ta\n\tb").expandtabs(1));
/*
    //test_split
    CHECK(["this", "is", "the", "split", "function"], "this is the split function", "split")

        # by whitespace
    CHECK(["a", "b", "c", "d"], "a b c d ", "split")
    CHECK(["a", "b c d"], "a b c d", "split", None, 1)
    CHECK(["a", "b", "c d"], "a b c d", "split", None, 2)
    CHECK(["a", "b", "c", "d"], "a b c d", "split", None, 3)
    CHECK(["a", "b", "c", "d"], "a b c d", "split", None, 4)
    CHECK(["a", "b", "c", "d"], "a b c d", "split", None,
                        std::numeric_limits<int>::max()-1)
    CHECK(["a b c d"], "a b c d", "split", None, 0)
    CHECK(["a b c d"], "  a b c d", "split", None, 0)
    CHECK(["a", "b", "c  d"], "a  b  c  d", "split", None, 2)

    CHECK([], "         ", "split")
    CHECK(["a"], "  a    ", "split")
    CHECK(["a", "b"], "  a    b   ", "split")
    CHECK(["a", "b   "], "  a    b   ", "split", None, 1)
    CHECK(["a", "b   c   "], "  a    b   c   ", "split", None, 1)
    CHECK(["a", "b", "c   "], "  a    b   c   ", "split", None, 2)
    CHECK(["a", "b"], "\n\ta \t\r b \v ", "split")
        aaa = " a "*20
    CHECK(["a"]*20, aaa, "split")
    CHECK(["a"] + [aaa[4:]], aaa, "split", None, 1)
    CHECK(["a"]*19 + ["a "], aaa, "split", None, 19)

        # by a char
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "split", "|")
    CHECK(["a|b|c|d"], "a|b|c|d", "split", "|", 0)
    CHECK(["a", "b|c|d"], "a|b|c|d", "split", "|", 1)
    CHECK(["a", "b", "c|d"], "a|b|c|d", "split", "|", 2)
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "split", "|", 3)
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "split", "|", 4)
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "split", "|",
                        std::numeric_limits<int>::max()-2)
    CHECK(["a|b|c|d"], "a|b|c|d", "split", "|", 0)
    CHECK(["a", "", "b||c||d"], "a||b||c||d", "split", "|", 2)
    CHECK(["endcase ", ""], "endcase |", "split", "|")
    CHECK(["", " startcase"], "| startcase", "split", "|")
    CHECK(["", "bothcase", ""], "|bothcase|", "split", "|")
    CHECK(["a", "", "b\x00c\x00d"], "a\x00\x00b\x00c\x00d", "split", "\x00", 2)

    CHECK(["a"]*20, ("a|"*20)[:-1], "split", "|")
    CHECK(["a"]*15 +["a|a|a|a|a"],
                                   ("a|"*20)[:-1], "split", "|", 15)

        # by string
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "split", "//")
    CHECK(["a", "b//c//d"], "a//b//c//d", "split", "//", 1)
    CHECK(["a", "b", "c//d"], "a//b//c//d", "split", "//", 2)
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "split", "//", 3)
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "split", "//", 4)
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "split", "//",
                        std::numeric_limits<int>::max()-10)
    CHECK(["a//b//c//d"], "a//b//c//d", "split", "//", 0)
    CHECK(["a", "", "b////c////d"], "a////b////c////d", "split", "//", 2)
    CHECK(["endcase ", ""], "endcase test", "split", "test")
    CHECK(["", " begincase"], "test begincase", "split", "test")
    CHECK(["", " bothcase ", ""], "test bothcase test",
                        "split", "test")
    CHECK(["a", "bc"], "abbbc", "split", "bb")
    CHECK(["", ""], "aaa", "split", "aaa")
    CHECK(["aaa"], "aaa", "split", "aaa", 0)
    CHECK(["ab", "ab"], "abbaab", "split", "ba")
    CHECK(["aaaa"], "aaaa", "split", "aab")
    CHECK([""], "", "split", "aaa")
    CHECK(["aa"], "aa", "split", "aaa")
    CHECK(["A", "bobb"], "Abbobbbobb", "split", "bbobb")
    CHECK(["A", "B", ""], "AbbobbBbbobb", "split", "bbobb")

    CHECK(["a"]*20, ("aBLAH"*20)[:-4], "split", "BLAH")
    CHECK(["a"]*20, ("aBLAH"*20)[:-4], "split", "BLAH", 19)
    CHECK(["a"]*18 + ["aBLAHa"], ("aBLAH"*20)[:-4],
                        "split", "BLAH", 18)

    //test_rsplit
    CHECK(["this", "is", "the", "rsplit", "function"],
                         "this is the rsplit function", "rsplit")

        # by whitespace
    CHECK(["a", "b", "c", "d"], "a b c d ", "rsplit")
    CHECK(["a b c", "d"], "a b c d", "rsplit", None, 1)
    CHECK(["a b", "c", "d"], "a b c d", "rsplit", None, 2)
    CHECK(["a", "b", "c", "d"], "a b c d", "rsplit", None, 3)
    CHECK(["a", "b", "c", "d"], "a b c d", "rsplit", None, 4)
    CHECK(["a", "b", "c", "d"], "a b c d", "rsplit", None,
                        std::numeric_limits<int>::max()-20)
    CHECK(["a b c d"], "a b c d", "rsplit", None, 0)
    CHECK(["a b c d"], "a b c d  ", "rsplit", None, 0)
    CHECK(["a  b", "c", "d"], "a  b  c  d", "rsplit", None, 2)

    CHECK([], "         ", "rsplit")
    CHECK(["a"], "  a    ", "rsplit")
    CHECK(["a", "b"], "  a    b   ", "rsplit")
    CHECK(["  a", "b"], "  a    b   ", "rsplit", None, 1)
    CHECK(["  a    b","c"], "  a    b   c   ", "rsplit",
                        None, 1)
    CHECK(["  a", "b", "c"], "  a    b   c   ", "rsplit",
                        None, 2)
    CHECK(["a", "b"], "\n\ta \t\r b \v ", "rsplit", None, 88)
        aaa = " a "*20
    CHECK(["a"]*20, aaa, "rsplit")
    CHECK([aaa[:-4]] + ["a"], aaa, "rsplit", None, 1)
    CHECK([" a  a"] + ["a"]*18, aaa, "rsplit", None, 18)


        # by a char
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "rsplit", "|")
    CHECK(["a|b|c", "d"], "a|b|c|d", "rsplit", "|", 1)
    CHECK(["a|b", "c", "d"], "a|b|c|d", "rsplit", "|", 2)
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "rsplit", "|", 3)
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "rsplit", "|", 4)
    CHECK(["a", "b", "c", "d"], "a|b|c|d", "rsplit", "|",
                        std::numeric_limits<int>::max()-100)
    CHECK(["a|b|c|d"], "a|b|c|d", "rsplit", "|", 0)
    CHECK(["a||b||c", "", "d"], "a||b||c||d", "rsplit", "|", 2)
    CHECK(["", " begincase"], "| begincase", "rsplit", "|")
    CHECK(["endcase ", ""], "endcase |", "rsplit", "|")
    CHECK(["", "bothcase", ""], "|bothcase|", "rsplit", "|")

    CHECK(["a\x00\x00b", "c", "d"], "a\x00\x00b\x00c\x00d", "rsplit", "\x00", 2)

    CHECK(["a"]*20, ("a|"*20)[:-1], "rsplit", "|")
    CHECK(["a|a|a|a|a"]+["a"]*15,
                        ("a|"*20)[:-1], "rsplit", "|", 15)

        # by string
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "rsplit", "//")
    CHECK(["a//b//c", "d"], "a//b//c//d", "rsplit", "//", 1)
    CHECK(["a//b", "c", "d"], "a//b//c//d", "rsplit", "//", 2)
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "rsplit", "//", 3)
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "rsplit", "//", 4)
    CHECK(["a", "b", "c", "d"], "a//b//c//d", "rsplit", "//",
                        std::numeric_limits<int>::max()-5)
    CHECK(["a//b//c//d"], "a//b//c//d", "rsplit", "//", 0)
    CHECK(["a////b////c", "", "d"], "a////b////c////d", "rsplit", "//", 2)
    CHECK(["", " begincase"], "test begincase", "rsplit", "test")
    CHECK(["endcase ", ""], "endcase test", "rsplit", "test")
    CHECK(["", " bothcase ", ""], "test bothcase test",
                        "rsplit", "test")
    CHECK(["ab", "c"], "abbbc", "rsplit", "bb")
    CHECK(["", ""], "aaa", "rsplit", "aaa")
    CHECK(["aaa"], "aaa", "rsplit", "aaa", 0)
    CHECK(["ab", "ab"], "abbaab", "rsplit", "ba")
    CHECK(["aaaa"], "aaaa", "rsplit", "aab")
    CHECK([""], "", "rsplit", "aaa")
    CHECK(["aa"], "aa", "rsplit", "aaa")
    CHECK(["bbob", "A"], "bbobbbobbA", "rsplit", "bbobb")
    CHECK(["", "B", "A"], "bbobbBbbobbA", "rsplit", "bbobb")

    CHECK(["a"]*20, ("aBLAH"*20)[:-4], "rsplit", "BLAH")
    CHECK(["a"]*20, ("aBLAH"*20)[:-4], "rsplit", "BLAH", 19)
    CHECK(["aBLAHa"] + ["a"]*18, ("aBLAH"*20)[:-4],
                        "rsplit", "BLAH", 18)
*/
    //test_strip
    CHECK(string("hello") == string("   hello   ").strip());
    CHECK(string("hello   ") == string("   hello   ").lstrip());
    CHECK(string("   hello") == string("   hello   ").rstrip());
    CHECK(string("hello") == string("hello").strip());

    //strip/lstrip/rstrip with str arg
    CHECK(string("hello") == string("xyzzyhelloxyzzy").strip("xyz"));
    CHECK(string("helloxyzzy") == string("xyzzyhelloxyzzy").lstrip("xyz"));
    CHECK(string("xyzzyhello") == string("xyzzyhelloxyzzy").rstrip("xyz"));
    CHECK(string("hello") == string("hello").strip("xyz"));

    //test_ljust
    CHECK(string("abc       ") == string("abc").ljust(10));
    CHECK(string("abc   ") == string("abc").ljust(6));
    CHECK(string("abc") == string("abc").ljust(3));
    CHECK(string("abc") == string("abc").ljust(2));
    CHECK(string("abc*******") == string("abc").ljust(10, '*'));

    //test_rjust
    CHECK(string("       abc") == string("abc").rjust(10));
    CHECK(string("   abc") == string("abc").rjust(6));
    CHECK(string("abc") == string("abc").rjust(3));
    CHECK(string("abc") == string("abc").rjust(2));
    CHECK(string("*******abc") == string("abc").rjust(10, '*'));

    //test_center
    CHECK(string("   abc    ") == string("abc").center(10));
    CHECK(string(" abc  ") == string("abc").center(6));
    CHECK(string("abc") == string("abc").center(3));
    CHECK(string("abc") == string("abc").center(2));
    CHECK(string("***abc****") == string("abc").center(10, '*'));

    //test_swapcase
    CHECK(string("hEllO CoMPuTErS") == string("HeLLo cOmpUteRs").swapcase());

    //test_replace
    // Operations on the empty string
    CHECK(string("") == string("").replace("", ""));
    CHECK(string("A") == string("").replace("", "A"));
    CHECK(string("") == string("").replace("A", ""));
    CHECK(string("") == string("").replace("A", "A"));
    CHECK(string("") == string("").replace("", "", 100));
    CHECK(string("") == string("").replace("", "", std::numeric_limits<int>::max()));

    // interleave (from=="", "to" gets inserted everywhere)
    CHECK("A" == string("A").replace("", ""));
    CHECK("*A*" == string("A").replace("", "*"));
    CHECK("*1A*1" == string("A").replace("", "*1"));
    CHECK("*-#A*-#" == string("A").replace("", "*-#"));
    CHECK("*-A*-A*-" == string("AA").replace("", "*-"));
    CHECK("*-A*-A*-" == string("AA").replace("", "*-", -1));
    CHECK("*-A*-A*-" == string("AA").replace("", "*-", std::numeric_limits<int>::max()));
    CHECK("*-A*-A*-" == string("AA").replace("", "*-", 4));
    CHECK("*-A*-A*-" == string("AA").replace("", "*-", 3));
    CHECK("*-A*-A" == string("AA").replace("", "*-", 2));
    CHECK("*-AA" == string("AA").replace("", "*-", 1));
    CHECK("AA" == string("AA").replace("", "*-", 0));

    // single character deletion (from=="A", to=="")
    CHECK("" == string("A").replace("A", ""));
    CHECK("" == string("AAA").replace("A", ""));
    CHECK("" == string("AAA").replace("A", "", -1));
    CHECK("" == string("AAA").replace("A", "", std::numeric_limits<int>::max()));
    CHECK("" == string("AAA").replace("A", "", 4));
    CHECK("" == string("AAA").replace("A", "", 3));
    CHECK("A" == string("AAA").replace("A", "", 2));
    CHECK("AA" == string("AAA").replace("A", "", 1));
    CHECK("AAA" == string("AAA").replace("A", "", 0));
    CHECK("" == string("AAAAAAAAAA").replace("A", ""));
    CHECK("BCD" == string("ABACADA").replace("A", ""));
    CHECK("BCD" == string("ABACADA").replace("A", "", -1));
    CHECK("BCD" == string("ABACADA").replace("A", "", std::numeric_limits<int>::max()));
    CHECK("BCD" == string("ABACADA").replace("A", "", 5));
    CHECK("BCD" == string("ABACADA").replace("A", "", 4));
    CHECK("BCDA" == string("ABACADA").replace("A", "", 3));
    CHECK("BCADA" == string("ABACADA").replace("A", "", 2));
    CHECK("BACADA" == string("ABACADA").replace("A", "", 1));
    CHECK("ABACADA" == string("ABACADA").replace("A", "", 0));
    CHECK("BCD" == string("ABCAD").replace("A", ""));
    CHECK("BCD" == string("ABCADAA").replace("A", ""));
    CHECK("BCD" == string("BCD").replace("A", ""));
    CHECK("*************" == string("*************").replace("A", ""));
    CHECK("^A^" == string(string("^")+string("A")*1000+string("^")).replace("A", "", 999));

    // substring deletion (from=="the", to=="")
    CHECK("" == string("the").replace("the", ""));
    CHECK("ater" == string("theater").replace("the", ""));
    CHECK("" == string("thethe").replace("the", ""));
    CHECK("" == string("thethethethe").replace("the", ""));
    CHECK("aaaa" == string("theatheatheathea").replace("the", ""));
    CHECK("that" == string("that").replace("the", ""));
    CHECK("thaet" == string("thaet").replace("the", ""));
    CHECK("here and re" == string("here and there").replace("the", ""));
    CHECK("here and re and re" == string("here and there and there").replace("the", "", std::numeric_limits<int>::max()));
    CHECK("here and re and re" == string("here and there and there").replace("the", "", -1));
    CHECK("here and re and re" == string("here and there and there").replace("the", "", 3));
    CHECK("here and re and re" == string("here and there and there").replace("the", "", 2));
    CHECK("here and re and there" == string("here and there and there").replace("the", "", 1));
    CHECK("here and there and there" == string("here and there and there").replace("the", "", 0));
    CHECK("here and re and re" == string("here and there and there").replace("the", ""));

    CHECK("abc" == string("abc").replace("the", ""));
    CHECK("abcdefg" == string("abcdefg").replace("the", ""));

    // substring deletion (from=="bob", to=="")
    CHECK("bob" == string("bbobob").replace("bob", ""));
    CHECK("bobXbob" == string("bbobobXbbobob").replace("bob", ""));
    CHECK("aaaaaaa" == string("aaaaaaabob").replace("bob", ""));
    CHECK("aaaaaaa" == string("aaaaaaa").replace("bob", ""));

    // single character replace in place (len(from)==len(to)==1)
    CHECK("Who goes there?" == string("Who goes there?").replace("o", "o"));
    CHECK("WhO gOes there?" == string("Who goes there?").replace("o", "O"));
    CHECK("WhO gOes there?" == string("Who goes there?").replace("o", "O", std::numeric_limits<int>::max()));
    CHECK("WhO gOes there?" == string("Who goes there?").replace("o", "O", -1));
    CHECK("WhO gOes there?" == string("Who goes there?").replace("o", "O", 3));
    CHECK("WhO gOes there?" == string("Who goes there?").replace("o", "O", 2));
    CHECK("WhO goes there?" == string("Who goes there?").replace("o", "O", 1));
    CHECK("Who goes there?" == string("Who goes there?").replace("o", "O", 0));

    CHECK("Who goes there?" == string("Who goes there?").replace("a", "q"));
    CHECK("who goes there?" == string("Who goes there?").replace("W", "w"));
    CHECK("wwho goes there?ww" == string("WWho goes there?WW").replace("W", "w"));
    CHECK("Who goes there!" == string("Who goes there?").replace("?", "!"));
    CHECK("Who goes there!!" == string("Who goes there??").replace("?", "!"));

    CHECK("Who goes there?" == string("Who goes there?").replace(".", "!"));
/*
    // substring replace in place (len(from)==len(to) > 1)
    CHECK("Th** ** a t**sue", "This is a tissue").replace("is", "**")
    CHECK("Th** ** a t**sue", "This is a tissue").replace("is", "**", std::numeric_limits<int>::max())
    CHECK("Th** ** a t**sue", "This is a tissue").replace("is", "**", -1)
    CHECK("Th** ** a t**sue", "This is a tissue").replace("is", "**", 4)
    CHECK("Th** ** a t**sue", "This is a tissue").replace("is", "**", 3)
    CHECK("Th** ** a tissue", "This is a tissue").replace("is", "**", 2)
    CHECK("Th** is a tissue", "This is a tissue").replace("is", "**", 1)
    CHECK("This is a tissue", "This is a tissue").replace("is", "**", 0)
    CHECK("cobob", "bobob").replace("bob", "cob")
    CHECK("cobobXcobocob", "bobobXbobobob").replace("bob", "cob")
    CHECK("bobob", "bobob").replace("bot", "bot")

        # replace single character (len(from)==1, len(to)>1)
    CHECK("ReyKKjaviKK", "Reykjavik").replace("k", "KK")
    CHECK("ReyKKjaviKK", "Reykjavik").replace("k", "KK", -1)
    CHECK("ReyKKjaviKK", "Reykjavik").replace("k", "KK", std::numeric_limits<int>::max())
    CHECK("ReyKKjaviKK", "Reykjavik").replace("k", "KK", 2)
    CHECK("ReyKKjavik", "Reykjavik").replace("k", "KK", 1)
    CHECK("Reykjavik", "Reykjavik").replace("k", "KK", 0)
    CHECK("A----B----C----", "A.B.C.").replace(".", "----")

    CHECK("Reykjavik", "Reykjavik").replace("q", "KK")

        # replace substring (len(from)>1, len(to)!=len(from))
    CHECK("ham, ham, eggs and ham", "spam, spam, eggs and spam",
           "replace", "spam", "ham")
    CHECK("ham, ham, eggs and ham", "spam, spam, eggs and spam",
           "replace", "spam", "ham", std::numeric_limits<int>::max())
    CHECK("ham, ham, eggs and ham", "spam, spam, eggs and spam",
           "replace", "spam", "ham", -1)
    CHECK("ham, ham, eggs and ham", "spam, spam, eggs and spam",
           "replace", "spam", "ham", 4)
    CHECK("ham, ham, eggs and ham", "spam, spam, eggs and spam",
           "replace", "spam", "ham", 3)
    CHECK("ham, ham, eggs and spam", "spam, spam, eggs and spam",
           "replace", "spam", "ham", 2)
    CHECK("ham, spam, eggs and spam", "spam, spam, eggs and spam",
           "replace", "spam", "ham", 1)
    CHECK("spam, spam, eggs and spam", "spam, spam, eggs and spam",
           "replace", "spam", "ham", 0)

    CHECK("bobob", "bobobob").replace("bobob", "bob")
    CHECK("bobobXbobob", "bobobobXbobobob").replace("bobob", "bob")
    CHECK("BOBOBOB", "BOBOBOB").replace("bob", "bobby")

    CHECK("one@two!three!", "one!two!three!").replace("!", "@", 1)
    CHECK("onetwothree", "one!two!three!").replace("!", "")
    CHECK("one@two@three!", "one!two!three!").replace("!", "@", 2)
    CHECK("one@two@three@", "one!two!three!").replace("!", "@", 3)
    CHECK("one@two@three@", "one!two!three!").replace("!", "@", 4)
    CHECK("one!two!three!", "one!two!three!").replace("!", "@", 0)
    CHECK("one@two@three@", "one!two!three!").replace("!", "@")
    CHECK("one!two!three!", "one!two!three!").replace("x", "@")
    CHECK("one!two!three!", "one!two!three!").replace("x", "@", 2)
    CHECK("-a-b-c-", "abc").replace("", "-")
    CHECK("-a-b-c", "abc").replace("", "-", 3)
    CHECK("abc", "abc").replace("", "-", 0)
    CHECK("", "").replace("", "")
    CHECK("abc", "abc").replace("ab", "--", 0)
    CHECK("abc", "abc").replace("xy", "--")
        # Next three for SF bug 422088: [OSF1 alpha] string.replace(); died with
        # MemoryError due to empty result (platform malloc issue when requesting
        # 0 bytes).
    CHECK("", "123").replace("123", "")
    CHECK("", "123123").replace("123", "")
    CHECK("x", "123x123").replace("123", "")

*/
    //test_zfill
    CHECK(string("123") == string("123").zfill(2));
    CHECK(string("123") == string("123").zfill(3));
    CHECK(string("0123") == string("123").zfill(4));
    CHECK(string("+123") == string("+123").zfill(3));
    CHECK(string("+123") == string("+123").zfill(4));
    CHECK(string("+0123") == string("+123").zfill(5));
    CHECK(string("-123") == string("-123").zfill(3));
    CHECK(string("-123") == string("-123").zfill(4));
    CHECK(string("-0123") == string("-123").zfill(5));
    CHECK(string("000") == string("").zfill(3));
    CHECK(string("34") == string("34").zfill(1));
    CHECK(string("0034") == string("34").zfill(4));

    //test_islower
    CHECK(false == string("").islower());
    CHECK(true == string("a").islower());
    CHECK(false == string("A").islower());
    CHECK(false == string("\n").islower());
    CHECK(true == string("abc").islower());
    CHECK(false == string("aBc").islower());
    CHECK(true == string("abc\n").islower());

    //test_isupper
    CHECK(false == string("").isupper());
    CHECK(false == string("a").isupper());
    CHECK(true == string("A").isupper());
    CHECK(false == string("\n").isupper());
    CHECK(true == string("ABC").isupper());
    CHECK(false == string("AbC").isupper());
    CHECK(true == string("ABC\n").isupper());

    //test_istitle
    CHECK(false == string("").istitle());
    CHECK(false == string("a").istitle());
    CHECK(true == string("A").istitle());
    CHECK(false == string("\n").istitle());
    CHECK(true == string("A Titlecased Line").istitle());
    CHECK(true == string("A\nTitlecased Line").istitle());
    CHECK(true == string("A Titlecased, Line").istitle());
    CHECK(false == string("Not a capitalized String").istitle());
    CHECK(false == string("Not\ta Titlecase String").istitle());
    CHECK(false == string("Not--a Titlecase String").istitle());
    CHECK(false == string("NOT").istitle());

    //test_isspace
    CHECK(false == string("").isspace());
    CHECK(false == string("a").isspace());
    CHECK(true == string(" ").isspace());
    CHECK(true == string("\t").isspace());
    CHECK(true == string("\r").isspace());
    CHECK(true == string("\n").isspace());
    CHECK(true == string(" \t\r\n").isspace());
    CHECK(false == string(" \t\r\na").isspace());

    //test_isalpha
    CHECK(false == string("").isalpha());
    CHECK(true == string("a").isalpha());
    CHECK(true == string("A").isalpha());
    CHECK(false == string("\n").isalpha());
    CHECK(true == string("abc").isalpha());
    CHECK(false == string("aBc123").isalpha());
    CHECK(false == string("abc\n").isalpha());

    //test_isalnum
    CHECK(false == string("").isalnum());
    CHECK(true == string("a").isalnum());
    CHECK(true == string("A").isalnum());
    CHECK(false == string("\n").isalnum());
    CHECK(true == string("123abc456").isalnum());
    CHECK(true == string("a1b3c").isalnum());
    CHECK(false == string("aBc000 ").isalnum());
    CHECK(false == string("abc\n").isalnum());

    //test_isdigit
    CHECK(false == string("").isdigit());
    CHECK(false == string("a").isdigit());
    CHECK(true == string("0").isdigit());
    CHECK(true == string("0123456789").isdigit());
    CHECK(false == string("0123456789a").isdigit());

    //test_title
    CHECK(string(" Hello ") == string(" hello ").title());
    CHECK(string("Hello ") == string("hello ").title());
    CHECK(string("Hello ") == string("Hello ").title());
    CHECK(string("Format This As Title String") == string("fOrMaT thIs aS titLe String").title());
    CHECK(string("Format,This-As*Title;String") == string("fOrMaT,thIs-aS*titLe;String").title());
    CHECK(string("Getint") == string("getInt").title());
/*
    //test_splitlines
    CHECK(["abc", "def", "", "ghi"], "abc\ndef\n\rghi", "splitlines")
    CHECK(["abc", "def", "", "ghi"], "abc\ndef\n\r\nghi", "splitlines")
    CHECK(["abc", "def", "ghi"], "abc\ndef\r\nghi", "splitlines")
    CHECK(["abc", "def", "ghi"], "abc\ndef\r\nghi\n", "splitlines")
    CHECK(["abc", "def", "ghi", ""], "abc\ndef\r\nghi\n\r", "splitlines")
    CHECK(["", "abc", "def", "ghi", ""], "\nabc\ndef\r\nghi\n\r", "splitlines")
    CHECK(["\n", "abc\n", "def\r\n", "ghi\n", "\r"], "\nabc\ndef\r\nghi\n\r", "splitlines", 1)

        checkraises(TypeError, "abc", "splitlines", 42, 42)
*/
    //test_startswith
    CHECK(true == string("hello").startswith("he"));
    CHECK(true == string("hello").startswith("hello"));
    CHECK(false == string("hello").startswith("hello world"));
    CHECK(true == string("hello").startswith(""));
    CHECK(false == string("hello").startswith("ello"));
    //CHECK(true == string("hello").startswith("ello", 1));
    //CHECK(true == string("hello").startswith("o", 4));
    //CHECK(false == string("hello").startswith("o", 5));
    //CHECK(true == string("hello").startswith("", 5));
    //CHECK(false == string("hello").startswith("lo", 6));
    //CHECK(true == string("helloworld").startswith("lowo", 3));
    //CHECK(true == string("helloworld").startswith("lowo", 3, 7));
    //CHECK(false == string("helloworld").startswith("lowo", 3, 6));

    //test negative indices
    //CHECK(true == string("hello").startswith("he", 0, -1));
    //CHECK(true == string("hello").startswith("he", -53, -1));
    //CHECK(false == string("hello").startswith("hello", 0, -1));
    //CHECK(false == string("hello").startswith("hello world", -1, -10));
    //CHECK(false == string("hello").startswith("ello", -5));
    //CHECK(true == string("hello").startswith("ello", -4));
    //CHECK(false == string("hello").startswith("o", -2));
    //CHECK(true == string("hello").startswith("o", -1));
    //CHECK(true == string("hello").startswith("", -3, -3));
    //CHECK(false == string("hello").startswith("lo", -9));

    //test_endswith
    CHECK(true == string("hello").endswith("lo"));
    CHECK(false == string("hello").endswith("he"));
    CHECK(true == string("hello").endswith(""));
    CHECK(false == string("hello").endswith("hello world"));
    CHECK(false == string("helloworld").endswith("worl"));
    //CHECK(true == string("helloworld").endswith("worl", 3, 9));
    //CHECK(true == string("helloworld").endswith("world", 3, 12));
    //CHECK(true == string("helloworld").endswith("lowo", 1, 7));
    //CHECK(true == string("helloworld").endswith("lowo", 2, 7));
    //CHECK(true == string("helloworld").endswith("lowo", 3, 7));
    //CHECK(false == string("helloworld").endswith("lowo", 4, 7));
    //CHECK(false == string("helloworld").endswith("lowo", 3, 8));
    //CHECK(false == string("ab").endswith("ab", 0, 1));
    //CHECK(false == string("ab").endswith("ab", 0, 0));

    //test negative indices
    //CHECK(true == string("hello").endswith("lo", -2));
    //CHECK(false == string("hello").endswith("he", -2));
    //CHECK(true == string("hello").endswith("", -3, -3));
    //CHECK(false == string("hello").endswith("hello world", -10, -2));
    //CHECK(false == string("helloworld").endswith("worl", -6));
    //CHECK(true == string("helloworld").endswith("worl", -5, -1));
    //CHECK(true == string("helloworld").endswith("worl", -5, 9));
    //CHECK(true == string("helloworld").endswith("world", -7, 12));
    //CHECK(true == string("helloworld").endswith("lowo", -99, -3));
    //CHECK(true == string("helloworld").endswith("lowo", -8, -3));
    //CHECK(true == string("helloworld").endswith("lowo", -7, -3));
    //CHECK(false == string("helloworld").endswith("lowo", 3, -4));
    //CHECK(false == string("helloworld").endswith("lowo", -8, -2));

    //test_join
    CHECK("a b c d" == string(" ").join({"a", "b", "c", "d"}));
    CHECK("abcd" == string("").join({"a", "b", "c", "d"}));
    CHECK("bd" == string("").join({"", "b", "", "d"}));
    CHECK("ac" == string("").join({"a", "", "c", ""}));
    CHECK("abc" == string("a").join({"abc"}));
/*
    //test_formatting
    CHECK("+hello+", "+%s+", "__mod__", "hello")
    CHECK("+10+", "+%d+", "__mod__", 10)
    CHECK("a", "%c", "__mod__", "a")
    CHECK("a", "%c", "__mod__", "a")
    CHECK(""", "%c", "__mod__", 34)
    CHECK("$", "%c", "__mod__", 36)
    CHECK("10", "%d", "__mod__", 10)
    CHECK("\x7f", "%c", "__mod__", 0x7f)

        for ordinal in (-100, 0x200000):
            # unicode raises ValueError, str raises OverflowError
            checkraises((ValueError, OverflowError), "%c", "__mod__", ordinal)

        longvalue = std::numeric_limits<int>::max() + 10L
        slongvalue = str(longvalue)
        if slongvalue[-1] in ("L","l"): slongvalue = slongvalue[:-1]
    CHECK(" 42", "%3ld", "__mod__", 42)
    CHECK("42", "%d", "__mod__", 42L)
    CHECK("42", "%d", "__mod__", 42.0)
    CHECK(slongvalue, "%d", "__mod__", longvalue)
        checkcall("%d", "__mod__", float(longvalue))
    CHECK("0042.00", "%07.2f", "__mod__", 42)
    CHECK("0042.00", "%07.2F", "__mod__", 42)

        checkraises(TypeError, "abc", "__mod__")
        checkraises(TypeError, "%(foo)s", "__mod__", 42)
        checkraises(TypeError, "%s%s", "__mod__", (42,))
        checkraises(TypeError, "%c", "__mod__", (None,))
        checkraises(ValueError, "%(foo", "__mod__", {})
        checkraises(TypeError, "%(foo)s %(bar)s", "__mod__", ("foo", 42))
        checkraises(TypeError, "%d", "__mod__", "42") # not numeric
        checkraises(TypeError, "%d", "__mod__", (42+0j)) # no int/long conversion provided

        # argument names with properly nested brackets are supported
    CHECK("bar", "%((foo))s", "__mod__", {"(foo)": "bar"})

        # 100 is a magic number in PyUnicode_Format, this forces a resize
    CHECK(103*"a"+"x", "%sx", "__mod__", 103*"a")

        checkraises(TypeError, "%*s", "__mod__", ("foo", "bar"))
        checkraises(TypeError, "%10.*f", "__mod__", ("foo", 42.))
        checkraises(ValueError, "%10", "__mod__", (42,))

    //test_floatformatting
        # float formatting
        for prec in xrange(100):
            format = "%%.%if" % prec
            value = 0.01
            for x in xrange(60):
                value = value * 3.14159265359 / 3.0 * 10.0
                checkcall(format, "__mod__", value)

    //test_partition

    CHECK(("this is the par", "ti", "tion method"),
            "this is the partition method", "partition", "ti")

        # from raymond"s original specification
        S = "http://www.python.org"
    CHECK(("http", "://", "www.python.org"), S, "partition", "://")
    CHECK(("http://www.python.org", "", ""), S, "partition", "?")
    CHECK(("", "http://", "www.python.org"), S, "partition", "http://")
    CHECK(("http://www.python.", "org", ""), S, "partition", "org")

    //test_rpartition

    CHECK(("this is the rparti", "ti", "on method"),
            "this is the rpartition method", "rpartition", "ti")

        # from raymond"s original specification
        S = "http://www.python.org"
    CHECK(("http", "://", "www.python.org"), S, "rpartition", "://")
    CHECK(("", "", "http://www.python.org"), S, "rpartition", "?")
    CHECK(("", "http://", "www.python.org"), S, "rpartition", "http://")
    CHECK(("http://www.python.", "org", ""), S, "rpartition", "org")
*/
}
