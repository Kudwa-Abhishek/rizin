// SPDX-License-Identifier: LGPL-3.0-only

#include <rz_core.h>
#include "minunit.h"

bool test_core_serialize_macros(void) {
        RzCore *core = rz_core_new();
        mu_assert_notnull(core, "source core");

        RzCore *loaded_core = rz_core_new();
        mu_assert_notnull(loaded_core, "destination core");

        mu_assert_notnull(rz_core_file_open(core, "malloc://0x1000", RZ_PERM_R, 0),
                "open source core file");
        mu_assert_notnull(rz_core_file_open(loaded_core, "malloc://0x1000", RZ_PERM_R, 0),
                "open destination core file");

        const char *zero_args[] = { NULL };
        const char *two_args[] = { "a", "b", NULL };
        const char *existing_args[] = { "old", NULL };

        mu_assert_true(rz_cmd_macro_add(core->rcmd, "zero", zero_args, "zero body"),
                "add zero-argument macro");
        mu_assert_true(rz_cmd_macro_add(core->rcmd, "pair", two_args, "${a} ${b}"),
                "add two-argument macro");

        mu_assert_true(rz_cmd_macro_add(loaded_core->rcmd, "pair", existing_args, "old body"),
                "add existing macro to destination");

        Sdb *db = sdb_new0();
        mu_assert_notnull(db, "serialization database");

        rz_serialize_core_save(db, core, NULL);

        RzSerializeResultInfo *res = rz_serialize_result_info_new();
        mu_assert_notnull(res, "serialization result info");
        bool loaded = rz_serialize_core_load(db, loaded_core, false, NULL, res);
        mu_assert_true(loaded, "load serialized core");

        const RzCmdMacro *zero = rz_cmd_macro_get(loaded_core->rcmd, "zero");
        mu_assert_notnull(zero, "restore zero-argument macro");
        mu_assert_eq(zero->nargs, 0, "zero-argument macro argument count");
        mu_assert_streq(zero->code, "zero body", "zero-argument macro body");

        const RzCmdMacro *pair = rz_cmd_macro_get(loaded_core->rcmd, "pair");
        mu_assert_notnull(pair, "restore existing two-argument macro");
        mu_assert_eq(pair->nargs, 2, "two-argument macro argument count");
        mu_assert_streq(pair->args[0], "a", "first macro argument");
        mu_assert_streq(pair->args[1], "b", "second macro argument");
        mu_assert_streq(pair->code, "${a} ${b}", "two-argument macro body");

        rz_serialize_result_info_free(res);
        sdb_free(db);
        rz_core_file_close(core->file);
        rz_core_file_close(loaded_core->file);
        rz_core_free(core);
        rz_core_free(loaded_core);
        mu_end;
}

bool test_core_serialize_empty_macros(void) {
        RzCore *core = rz_core_new();
        mu_assert_notnull(core, "source core");

        RzCore *loaded_core = rz_core_new();
        mu_assert_notnull(loaded_core, "destination core");

        Sdb *db = sdb_new0();
        mu_assert_notnull(db, "serialization database");

        rz_serialize_core_save(db, core, NULL);

        RzSerializeResultInfo *res = rz_serialize_result_info_new();
        mu_assert_notnull(res, "serialization result info");

        bool loaded = rz_serialize_core_load(db, loaded_core, false, NULL, res);
        mu_assert_true(loaded, "load project without macros");

        rz_serialize_result_info_free(res);
        sdb_free(db);
        rz_core_free(core);
        rz_core_free(loaded_core);
        mu_end;
}

int all_tests() {
        mu_run_test(test_core_serialize_macros);
        mu_run_test(test_core_serialize_empty_macros);
        return tests_passed != tests_run;
}
mu_main(all_tests)
