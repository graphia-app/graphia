/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick
import QtTest

import Graphia.Utils

TestCase
{
    name: "Utils"

    function test_isNumeric_data()
    {
        return [
            {tag: "int", value: 1, expected: true},
            {tag: "float", value: -1.5, expected: true},
            {tag: "numeric string", value: "2.5e3", expected: true},
            {tag: "empty string", value: "", expected: false},
            {tag: "whitespace", value: " ", expected: false},
            {tag: "text", value: "abc", expected: false},
            {tag: "trailing text", value: "1abc", expected: false},
            {tag: "NaN", value: NaN, expected: false},
            {tag: "infinity", value: Infinity, expected: false},
            {tag: "null", value: null, expected: false},
            {tag: "undefined", value: undefined, expected: false}
        ];
    }

    function test_isNumeric(data)
    {
        compare(Utils.isNumeric(data.value), data.expected);
    }

    function test_isInt_data()
    {
        return [
            {tag: "int", value: 3, expected: true},
            {tag: "negative", value: -3, expected: true},
            {tag: "int as float", value: 3.0, expected: true},
            {tag: "int string", value: "3", expected: true},
            {tag: "float", value: 3.5, expected: false},
            {tag: "float string", value: "3.5", expected: false},
            {tag: "text", value: "abc", expected: false},
            {tag: "NaN", value: NaN, expected: false}
        ];
    }

    function test_isInt(data)
    {
        compare(Utils.isInt(data.value), data.expected);
    }

    function test_clamp()
    {
        compare(Utils.clamp(5, 0, 10), 5);
        compare(Utils.clamp(-5, 0, 10), 0);
        compare(Utils.clamp(15, 0, 10), 10);
    }

    function test_castToBool_data()
    {
        return [
            {tag: "true", value: true, expected: true},
            {tag: "false", value: false, expected: false},
            {tag: "true string", value: "true", expected: true},
            {tag: "true string mixed case", value: "True", expected: true},
            {tag: "false string", value: "false", expected: false},
            {tag: "other string", value: "yes", expected: false},
            {tag: "zero", value: 0, expected: false},
            {tag: "non-zero", value: 2, expected: true},
            {tag: "object", value: {}, expected: false},
            {tag: "undefined", value: undefined, expected: false}
        ];
    }

    function test_castToBool(data)
    {
        compare(Utils.castToBool(data.value), data.expected);
    }

    function test_normaliseWhitespace()
    {
        compare(Utils.normaliseWhitespace("a \t\n b  c"), "a b c");
    }

    function test_escapeQuotes_data()
    {
        return [
            {tag: "none", value: "abc", escaped: "abc"},
            {tag: "quotes", value: "say \"hi\"", escaped: "say \\\"hi\\\""},
            {tag: "only quotes", value: "\"\"", escaped: "\\\"\\\""},
            {tag: "backslashes", value: "a\\b\\", escaped: "a\\\\b\\\\"}
        ];
    }

    function test_escapeQuotes(data)
    {
        compare(Utils.escapeQuotes(data.value), data.escaped);
        compare(Utils.unescapeQuotes(data.escaped), data.value);
    }

    function test_unescapeQuotes_data()
    {
        return [
            {tag: "enclosing quotes", value: "\"abc\"", expected: "abc"},
            {tag: "enclosing quotes and whitespace", value: "  \"abc\"\n", expected: "abc"},
            {tag: "enclosing quotes and newlines", value: "\"a\nb\"", expected: "a\nb"},
            {tag: "escaped quotes inside", value: "\"a \\\"b\\\"\"", expected: "a \"b\""},
            {tag: "no enclosing quotes", value: "a \\\"b\\\"", expected: "a \"b\""},
            {tag: "trailing backslash", value: "\"C:\\\\\"", expected: "C:\\"},
            {tag: "other backslashes", value: "\"\\d\"", expected: "\\d"}
        ];
    }

    function test_unescapeQuotes(data)
    {
        compare(Utils.unescapeQuotes(data.value), data.expected);
    }

    function test_addSlashes_data()
    {
        return [
            {tag: "plain", value: "abc", expected: "abc"},
            {tag: "quotes", value: "a\"b", expected: "a\\\"b"},
            {tag: "backslash", value: "a\\b", expected: "a\\\\b"},
            {tag: "newline", value: "a\nb", expected: "a\\nb"},
            {tag: "number", value: 5, expected: "5"}
        ];
    }

    function test_addSlashes(data)
    {
        compare(Utils.addSlashes(data.value), data.expected);
    }

    function test_sanitiseJson_data()
    {
        return [
            {tag: "object", value: "{ \"a\" : [1, 2] }", expected: "{\"a\":[1,2]}"},
            {tag: "number", value: " 1.50 ", expected: "1.5"},
            {tag: "not JSON", value: "{abc", expected: "{abc"},
            {tag: "empty", value: "", expected: ""}
        ];
    }

    function test_sanitiseJson(data)
    {
        compare(Utils.sanitiseJson(data.value), data.expected);
    }

    function test_decimalPointsForValue_data()
    {
        return [
            {tag: "0", value: 0, expected: 5},
            {tag: "0.001", value: 0.001, expected: 5},
            {tag: "0.005", value: 0.005, expected: 4},
            {tag: "0.5", value: 0.5, expected: 3},
            {tag: "-0.5", value: -0.5, expected: 3},
            {tag: "1", value: 1, expected: 3},
            {tag: "50", value: 50, expected: 2},
            {tag: "500", value: 500, expected: 1},
            {tag: "5000", value: 5000, expected: 0}
        ];
    }

    function test_decimalPointsForValue(data)
    {
        compare(Utils.decimalPointsForValue(data.value), data.expected);
    }

    function test_decimalPointsForRange()
    {
        compare(Utils.decimalPointsForRange(0, 0.5), 3);
        compare(Utils.decimalPointsForRange(100, 150), 2);
        compare(Utils.decimalPointsForRange(1, 0), 0);
        compare(Utils.decimalPointsForRange("a", 1), 0);
    }

    function test_incrementForRange_data()
    {
        return [
            {tag: "0 to 1", min: 0, max: 1, expected: 0.001},
            {tag: "0 to 10", min: 0, max: 10, expected: 0.01},
            {tag: "0 to 11", min: 0, max: 11, expected: 0.1},
            {tag: "reversed", min: 10, max: 0, expected: 0.01},
            {tag: "-50 to 50", min: -50, max: 50, expected: 0.1}
        ];
    }

    function test_incrementForRange(data)
    {
        fuzzyCompare(Utils.incrementForRange(data.min, data.max), data.expected, 1e-12);
    }

    function test_format()
    {
        compare(Utils.format("{0} of {1}", 3, 4), "3 of 4");
        compare(Utils.format("{1} {0} {1}", "a", "b"), "b a b");
        compare(Utils.format("{0} {1}", "a"), "a {1}");

        // Substituted text isn't itself substituted
        compare(Utils.format("{0}{1}", "{1}", "x"), "{1}x");
    }

    function test_sets()
    {
        let set = [1, 2];

        verify(Utils.setContains(set, 1));
        verify(!Utils.setContains(set, 3));
        verify(!Utils.setContains(undefined, 1));

        let added = Utils.setAdd(set, 3);
        compare(added, [1, 2, 3]);
        compare(set, [1, 2], "setAdd changed its argument");
        verify(added !== set, "setAdd didn't return a new array");
        compare(Utils.setAdd(set, 1), [1, 2]);

        let removed = Utils.setRemove(added, 2);
        compare(removed, [1, 3]);
        compare(added, [1, 2, 3], "setRemove changed its argument");
        compare(Utils.setRemove(set, 5), [1, 2]);

        compare(Utils.setIntersection([1, 2, 3], [3, 2, 4]), [2, 3]);
        compare(Utils.setIntersection([1], []), []);
    }

    function test_regexEscape_data()
    {
        return [
            {tag: "plain", value: "abc"},
            {tag: "special characters", value: "-/\\^$*+?.()|[]{}"},
            {tag: "attribute like", value: "a.b (c) [d]"}
        ];
    }

    function test_regexEscape(data)
    {
        let re = new RegExp("^" + Utils.regexEscape(data.value) + "$");
        verify(re.test(data.value));

        // Not a match for anything else
        verify(!re.test(data.value + "x"));
        verify(!re.test(data.value.replace(/./g, "x")));
    }

    function test_arraysMatch_data()
    {
        return [
            {tag: "same", a: [1, 2, 3], b: [1, 2, 3], expected: true},
            {tag: "different order", a: [3, 1, 2], b: [1, 2, 3], expected: true},
            {tag: "numbers and strings", a: ["1", "2"], b: [2, 1], expected: true},
            {tag: "different lengths", a: [1, 2], b: [1, 2, 3], expected: false},
            {tag: "different values", a: [1, 2], b: [1, 3], expected: false},
            {tag: "both empty", a: [], b: [], expected: true},
            {tag: "null", a: null, b: [], expected: false},
            {tag: "undefined", a: [], b: undefined, expected: false}
        ];
    }

    function test_arraysMatch(data)
    {
        let a = data.a !== null && data.a !== undefined ? data.a.slice() : data.a;
        compare(Utils.arraysMatch(data.a, data.b), data.expected);

        // The arguments aren't changed
        compare(data.a, a);
    }

    function test_arrayIntersection_data()
    {
        return [
            {tag: "overlapping", a: [5, 1, 3], b: [3, 4, 5], expected: [3, 5]},
            {tag: "disjoint", a: [1, 2], b: [3, 4], expected: []},
            {tag: "empty", a: [], b: [1], expected: []},
            {tag: "duplicates", a: [1, 1, 2], b: [1, 2, 2], expected: [1, 2]},
            {tag: "numeric sort", a: [10, 9, 100], b: [100, 9], expected: [9, 100]}
        ];
    }

    function test_arrayIntersection(data)
    {
        let a = data.a.slice();
        compare([...Utils.arrayIntersection(data.a, data.b)], data.expected);
        compare(data.a, a);
    }

    function test_floatCompare()
    {
        compare(Utils.floatCompare(1.0, 1.0), 0);
        compare(Utils.floatCompare(1.0, 1.000001), 0);
        compare(Utils.floatCompare(1.0, 1.1), -1);
        compare(Utils.floatCompare(1.1, 1.0), 1);
    }

    function test_lerpAndNormalise()
    {
        compare(Utils.lerp(10, 20, 0.0), 10);
        compare(Utils.lerp(10, 20, 0.25), 12.5);
        compare(Utils.lerp(10, 20, 1.0), 20);
        compare(Utils.normalise(10, 20, 12.5), 0.25);

        for(let value of [-5, 0, 3.3, 7])
            fuzzyCompare(Utils.lerp(-5, 7, Utils.normalise(-5, 7, value)), value, 1e-12);
    }

    function test_objectsEquivalent()
    {
        verify(Utils.objectsEquivalent({a: 1, b: "x"}, {b: "x", a: 1}));
        verify(!Utils.objectsEquivalent({a: 1}, {a: 2}));
        verify(!Utils.objectsEquivalent({a: 1}, {a: 1, b: 2}));
        verify(!Utils.objectsEquivalent({a: 1}, {b: 1}));
    }

    function test_generateColorFrom()
    {
        let red = Qt.color("#ff0000");
        let next = Qt.color(Utils.generateColorFrom("#ff0000"));

        // The hue moves on by the golden angle, keeping saturation and value
        fuzzyCompare(next.hsvHue, 137.5 / 360.0, 0.01);
        fuzzyCompare(next.hsvSaturation, red.hsvSaturation, 0.01);
        fuzzyCompare(next.hsvValue, red.hsvValue, 0.01);

        // It wraps around
        let wrapped = Qt.color(Utils.generateColorFrom(next.toString()));
        wrapped = Qt.color(Utils.generateColorFrom(wrapped.toString()));
        fuzzyCompare(wrapped.hsvHue, ((137.5 * 3) / 360.0) % 1.0, 0.01);

        // Achromatic colours have no hue to move on from
        compare(Utils.generateColorFrom("#808080"), "#FF0000");
    }

    function test_desaturate()
    {
        let c = Qt.color(Utils.desaturate("#ff0000", 0.5));
        fuzzyCompare(c.hslSaturation, 0.5, 0.01);
        fuzzyCompare(c.hslHue, 0.0, 0.01);
        fuzzyCompare(c.hslLightness, 0.5, 0.01);
    }
}
