/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#ifndef SCOPE_LOCALIZATION_H_
#define SCOPE_LOCALIZATION_H_

#include <libintl.h>
#include <string>

inline char * _(const char *__msgid) {
    return dgettext(GETTEXT_PACKAGE, __msgid);
}

inline std::string _(const char *__msgid1, const char *__msgid2,
                     unsigned long int __n) {
    char buffer [256];
    if (snprintf ( buffer, 256, dngettext(GETTEXT_PACKAGE, __msgid1, __msgid2, __n), __n ) >= 0) {
        return buffer;
    } else {
        return std::string();
    }
}

#endif // SCOPE_LOCALIZATION_H_


