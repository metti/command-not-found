#include "similar.h"

#include <catch2/catch.hpp>

template <typename... Args>
std::set<std::string> make_set(Args&&... args) {
    return {args...};
}

TEST_CASE("similar::empty_string") {
    std::set<std::string> result;
    similar_words("", result);
    CHECK(result.empty());
}

TEST_CASE("similar::single_char_string") {
    std::set<std::string> result;
    similar_words("x", result);
    CHECK(!result.empty());
    CHECK(result ==
          make_set("-", "-x", "0", "0x", "1", "1x", "2", "2x", "3", "3x", "4",
                   "4x", "5", "5x", "6", "6x", "7", "7x", "8", "8x", "9", "9x",
                   "_", "_x", "a", "ax", "b", "bx", "c", "cx", "d", "dx", "e",
                   "ex", "f", "fx", "g", "gx", "h", "hx", "i", "ix", "j", "jx",
                   "k", "kx", "l", "lx", "m", "mx", "n", "nx", "o", "ox", "p",
                   "px", "q", "qx", "r", "rx", "s", "sx", "t", "tx", "u", "ux",
                   "v", "vx", "w", "wx", "x", "xx", "y", "yx", "z", "zx"));
    CHECK(result.size() == 76);
}

TEST_CASE("similar::multi_char_string") {
    std::set<std::string> result;
    similar_words("xyz", result);
    CHECK(!result.empty());
    CHECK(
        result ==
        make_set(
            "-xyz", "-yz", "0xyz", "0yz", "1xyz", "1yz", "2xyz", "2yz", "3xyz",
            "3yz", "4xyz", "4yz", "5xyz", "5yz", "6xyz", "6yz", "7xyz", "7yz",
            "8xyz", "8yz", "9xyz", "9yz", "_xyz", "_yz", "axyz", "ayz", "bxyz",
            "byz", "cxyz", "cyz", "dxyz", "dyz", "exyz", "eyz", "fxyz", "fyz",
            "gxyz", "gyz", "hxyz", "hyz", "ixyz", "iyz", "jxyz", "jyz", "kxyz",
            "kyz", "lxyz", "lyz", "mxyz", "myz", "nxyz", "nyz", "oxyz", "oyz",
            "pxyz", "pyz", "qxyz", "qyz", "rxyz", "ryz", "sxyz", "syz", "txyz",
            "tyz", "uxyz", "uyz", "vxyz", "vyz", "wxyz", "wyz", "x-yz", "x-z",
            "x0yz", "x0z", "x1yz", "x1z", "x2yz", "x2z", "x3yz", "x3z", "x4yz",
            "x4z", "x5yz", "x5z", "x6yz", "x6z", "x7yz", "x7z", "x8yz", "x8z",
            "x9yz", "x9z", "x_yz", "x_z", "xayz", "xaz", "xbyz", "xbz", "xcyz",
            "xcz", "xdyz", "xdz", "xeyz", "xez", "xfyz", "xfz", "xgyz", "xgz",
            "xhyz", "xhz", "xiyz", "xiz", "xjyz", "xjz", "xkyz", "xkz", "xlyz",
            "xlz", "xmyz", "xmz", "xnyz", "xnz", "xoyz", "xoz", "xpyz", "xpz",
            "xqyz", "xqz", "xryz", "xrz", "xsyz", "xsz", "xtyz", "xtz", "xuyz",
            "xuz", "xvyz", "xvz", "xwyz", "xwz", "xxyz", "xxz", "xy-", "xy-z",
            "xy0", "xy0z", "xy1", "xy1z", "xy2", "xy2z", "xy3", "xy3z", "xy4",
            "xy4z", "xy5", "xy5z", "xy6", "xy6z", "xy7", "xy7z", "xy8", "xy8z",
            "xy9", "xy9z", "xy_", "xy_z", "xya", "xyaz", "xyb", "xybz", "xyc",
            "xycz", "xyd", "xydz", "xye", "xyez", "xyf", "xyfz", "xyg", "xygz",
            "xyh", "xyhz", "xyi", "xyiz", "xyj", "xyjz", "xyk", "xykz", "xyl",
            "xylz", "xym", "xymz", "xyn", "xynz", "xyo", "xyoz", "xyp", "xypz",
            "xyq", "xyqz", "xyr", "xyrz", "xys", "xysz", "xyt", "xytz", "xyu",
            "xyuz", "xyv", "xyvz", "xyw", "xywz", "xyx", "xyxz", "xyy", "xyyz",
            "xyz", "xyzz", "xz", "xzy", "xzyz", "xzz", "yxyz", "yxz", "yyz",
            "yz", "zxyz", "zyz"));
    CHECK(result.size() == 228);
}

TEST_CASE("similar::long_string") {
    std::set<std::string> result;
    similar_words(std::string(512, 'x'), result);
    CHECK(!result.empty());
    CHECK(result.size() == 37891);
}

