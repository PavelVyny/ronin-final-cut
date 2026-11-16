"""Unit tests for Tools/Quality/check_code.py: dependency ratchet, size thresholds, counters and baseline.

Run from the project root:  python -m pytest Tools/tests -q
"""

import os
import shutil
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Quality"))

import check_code as cc  # noqa: E402


class SourceTree:
    """A throwaway Source/JamGame module folder."""

    def __init__(self):
        self.root = tempfile.mkdtemp()

    def add(self, rel, text=""):
        path = os.path.join(self.root, rel)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as h:
            h.write(text)

    def remove(self):
        shutil.rmtree(self.root)


class LayoutTest(unittest.TestCase):
    def setUp(self):
        self.tree = SourceTree()

    def tearDown(self):
        self.tree.remove()

    def test_public_and_private_prefixes_are_stripped(self):
        self.tree.add("Public/Match/MatchState.h")
        self.tree.add("Private/Match/MatchState.cpp")
        self.assertEqual(sorted(rel for rel, _ in cc._walk(self.tree.root)),
                         ["Match/MatchState.cpp", "Match/MatchState.h"])

    def test_flat_module_files_are_walked_relative_to_the_module_root(self):
        self.tree.add("JamGame.cpp")
        self.tree.add("Variant_Strategy/StrategyUnit.cpp")
        self.assertEqual(sorted(rel for rel, _ in cc._walk(self.tree.root)),
                         ["JamGame.cpp", "Variant_Strategy/StrategyUnit.cpp"])

    def test_system_is_the_top_folder_and_root_files_have_none(self):
        self.assertEqual(cc.system_of("Buildings/Barracks.cpp"), "Buildings")
        self.assertIsNone(cc.system_of("JamGame.cpp"))


class DependencyTest(unittest.TestCase):
    def setUp(self):
        self.tree = SourceTree()
        self.tree.add("Public/Core/Bus.h")
        self.tree.add("Public/Match/MatchState.h")
        self.tree.add("Public/Units/Unit.h")
        self.tree.add("Public/Buildings/Barracks.h")
        patcher = mock.patch.object(cc, "FORBIDDEN", {("Core", "Match")})
        patcher.start()
        self.addCleanup(patcher.stop)

    def tearDown(self):
        self.tree.remove()

    def test_forbidden_direction_is_a_violation_counted_per_include(self):
        self.tree.add("Private/Core/Controller.cpp", '#include "Match/MatchState.h"\n')
        self.tree.add("Private/Core/Other.cpp", '#include "Match/MatchState.h"\n')
        self.assertEqual(cc.dependency_violations(self.tree.root), {"include Core -> Match": 2})

    def test_allowed_direction_is_not_a_violation(self):
        self.tree.add("Private/Match/Flow.cpp", '#include "Core/Bus.h"\n')
        self.assertEqual(cc.dependency_violations(self.tree.root), {})

    def test_mutual_systems_are_one_cycle_counting_includes_both_ways(self):
        self.tree.add("Private/Units/Unit.cpp", '#include "Buildings/Barracks.h"\n')
        self.tree.add("Private/Buildings/Barracks.cpp", '#include "Units/Unit.h"\n#include "Units/Unit.h"\n')
        self.assertEqual(cc.dependency_violations(self.tree.root), {"cycle Buildings <-> Units": 3})

    def test_shared_system_casting_to_a_concrete_actor_is_counted_per_file(self):
        with mock.patch.object(cc, "SHARED", {"Combat"}), mock.patch.object(cc, "CONCRETE_CAST", cc.concrete_cast(["AUnit"])):
            self.tree.add("Private/Combat/Damage.cpp", "Cast<AUnit>(A); Cast<AUnit>(B);\n")
            self.tree.add("Private/Units/Unit.cpp", "Cast<AUnit>(B);\n")
            self.assertEqual(cc.dependency_violations(self.tree.root), {"cast Combat/Damage.cpp": 2})

    def test_simulation_including_presentation_is_forbidden_by_default(self):
        self.assertIn(("Sim", "View"), cc.SIMULATION_FORBIDDEN)
        self.assertIn(("Core", "Audio"), cc.SIMULATION_FORBIDDEN)
        self.assertNotIn(("UI", "Sim"), cc.SIMULATION_FORBIDDEN)
        self.assertNotIn(("View", "Core"), cc.SIMULATION_FORBIDDEN)

    def test_no_concrete_actors_configured_means_no_cast_check(self):
        self.assertIsNone(cc.concrete_cast([]))


def function(name, body_lines):
    """An Allman C++ function whose body is body_lines statements long."""
    return "void %s()\n{\n%s}\n" % (name, "\tStep();\n" * body_lines)


class SizeTest(unittest.TestCase):
    def setUp(self):
        self.tree = SourceTree()

    def tearDown(self):
        self.tree.remove()

    def test_function_longer_than_limit_is_reported_with_its_length(self):
        # 2 brace lines + 61 statements + the signature line = 64 lines.
        self.tree.add("Private/Units/Unit.cpp", function("AUnit::Think", 61) + function("AUnit::Rest", 10))
        self.assertEqual(cc.size_violations(self.tree.root), {"function Units/Unit.cpp AUnit::Think": 64})

    def test_cpp_file_longer_than_limit_is_reported_with_its_line_count(self):
        # Ten 50-line functions = 500 lines, then 110 lines of declarations = 610.
        text = "".join(function("F%d" % i, 47) for i in range(10)) + "int X;\n" * 110
        self.tree.add("Private/Units/Unit.cpp", text)
        self.assertEqual(cc.size_violations(self.tree.root), {"file Units/Unit.cpp": 610})

    def test_nesting_counts_allman_blocks_and_skips_wrapped_arguments(self):
        text = (
            "void AUnit::Think()\n{\n"
            "\tif (A)\n\t{\n"
            "\t\tfor (;;)\n\t\t{\n"
            "\t\t\tif (B)\n\t\t\t{\n"
            "\t\t\t}\n\t\t\telse if (C)\n\t\t\t{\n"
            "\t\t\t\twhile (D)\n\t\t\t\t{\n"
            "\t\t\t\t\tLog(One,\n\t\t\t\t\t\t\tTwo);\n"
            "\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}\n"
            "void AUnit::Rest()\n{\n\tif (A)\n\t{\n\t\tLog(One,\n\t\t\t\t\t\t\tTwo);\n\t}\n}\n"
        )
        self.tree.add("Private/Units/Unit.cpp", text)
        self.assertEqual(cc.size_violations(self.tree.root), {"nesting Units/Unit.cpp AUnit::Think": 4})


class RatchetTest(unittest.TestCase):
    BASELINE = {"cycle Buildings <-> Units": 14, "function Units/Unit.cpp AUnit::Think": 80, "include Core -> Match": 8}

    def test_new_and_grown_violations_are_worse_shrunk_and_gone_are_better(self):
        current = {"cycle Buildings <-> Units": 15, "function Units/Unit.cpp AUnit::Think": 70, "cycle Core <-> UI": 2}
        worse, better = cc.ratchet(current, self.BASELINE)
        self.assertEqual(worse, {"cycle Buildings <-> Units": (14, 15), "cycle Core <-> UI": (0, 2)})
        self.assertEqual(better, {"function Units/Unit.cpp AUnit::Think": (80, 70), "include Core -> Match": (8, 0)})

    def test_unchanged_violations_are_neither(self):
        self.assertEqual(cc.ratchet(dict(self.BASELINE), self.BASELINE), ({}, {}))

    def test_updated_baseline_only_shrinks_and_never_adopts_a_new_violation(self):
        current = {"cycle Buildings <-> Units": 15, "function Units/Unit.cpp AUnit::Think": 70, "cycle Core <-> UI": 2}
        updated = cc.updated_baseline({"violations": self.BASELINE, "tests": 233}, current, passed_tests=230)
        self.assertEqual(updated["violations"], {"cycle Buildings <-> Units": 14, "function Units/Unit.cpp AUnit::Think": 70})
        self.assertEqual(updated["tests"], 233)

    def test_updated_baseline_raises_the_test_count(self):
        updated = cc.updated_baseline({"violations": {}, "tests": 233}, {}, passed_tests=240)
        self.assertEqual(updated["tests"], 240)


class CounterTest(unittest.TestCase):
    def test_logtemp_is_counted_per_file(self):
        tree = SourceTree()
        try:
            tree.add("Private/Units/Unit.cpp", 'UE_LOG(LogTemp, Log, TEXT("a"));\nUE_LOG(LogTemp, Warning, TEXT("b"));\n')
            tree.add("Private/Units/Rest.cpp", 'UE_LOG(LogCFUnits, Log, TEXT("c"));\n')
            self.assertEqual(cc.logtemp_violations(tree.root), {"LogTemp Units/Unit.cpp": 2})
        finally:
            tree.remove()

    def test_passed_tests_counts_successful_cf_tests_only(self):
        log = (
            "Test Completed. Result={Success} Name={A} Path={Jam.Units.A}\n"
            "Test Completed. Result={Success} Name={B} Path={Jam.Units.B}\n"
            "Test Completed. Result={Fail} Name={C} Path={Jam.Units.C}\n"
            "Test Completed. Result={Success} Name={D} Path={Project.Functional.D}\n"
        )
        self.assertEqual(cc.passed_tests(log), 2)
