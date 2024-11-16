# SPDX-FileCopyrightText: 2023 Mark Penner <mrp@markpenner.space>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_tests(
    test_Track.cpp
    test_Utils.cpp
    LINK_LIBRARIES
    Qt::Test
    KF6::I18n
    libkwave
)
