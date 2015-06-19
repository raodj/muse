#ifndef XML_META_TYPE_HELPER
#define XML_META_TYPE_HELPER

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

/** \file XMLMetaTypeHelper.h
 *
 * \brief A file containing a macro to help interface QObject-based
 * types with Qt's Meta type system.
 *
 * This file provides utilities and macros to help XMLElement and its
 * derived classes to interface with Qt's meta type system to enable
 * marshalling and unmarshalling.
 */

/** \def DECLARE_METATYPE_HELPER Conveience macro to define a template
 * specialized meta data helper class.
 *
 * This macro must be used to define a template specialization for
 * Qt's QtMetaTypePrivate::QMeetaTypeFunctionHelper class. This
 * template specialization is used to avoid Qt's default helper classes
 * that require a copy-constructor.  Classes derived from QObject or
 * XMLElement (which is derived from QObject) are not meant to be
 * copyable and do not have copy constructors.
 *
 * This template specialization provides various method without using
 * copy consturctors. If a copy operation is attempted, then the methods
 * fail assertions.
 *
 * In some sense this macro "hacks" into Qt's meta type system. However,
 * the compiler will catch any inconsistencies and there should not be
 * any wierd runtime failures.
 */
#define DECLARE_METATYPE_HELPER(TYPE)                                   \
    namespace QtMetaTypePrivate {                                       \
        template <> struct QMetaTypeFunctionHelper<TYPE, true> {        \
            static void Delete(void *t) {                               \
                delete static_cast<TYPE*>(t);                           \
            }                                                           \
            static void *Create(const void *src) {                      \
                if (src) {                                              \
                    Q_ASSERT(#TYPE "cannot be copied" == NULL);         \
                }                                                       \
                return new TYPE();                                      \
            }                                                           \
            static void Destruct(void *) {                              \
                Q_ASSERT("Destruct for " #TYPE "not supported" == NULL);\
            }                                                           \
            static void *Construct(void *where, const void *src) {      \
                if (src) {                                              \
                    Q_ASSERT(#TYPE "cannot be copied" == NULL);         \
                }                                                       \
                return new (where) TYPE();                              \
            }                                                           \
            static void Save(QDataStream &, const void *) {             \
                Q_ASSERT("Save not supported for " #TYPE == NULL);      \
            }                                                           \
            static void Load(QDataStream &, void *) {                   \
                Q_ASSERT("Load not supported for " #TYPE == NULL);      \
            }                                                           \
        };                                                              \
    }

#endif // XML_META_TYPE_HELPER

