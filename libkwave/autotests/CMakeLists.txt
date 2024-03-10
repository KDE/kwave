# SPDX-FileCopyrightText: 2023 Mark Penner <mrpenner@mailbox.org>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_test(
    ../Utils.cpp
    Utils.test.cpp
    LINK_LIBRARIES
    Qt::Test
    KF6::I18n
    TEST_NAME test_Utils
)
