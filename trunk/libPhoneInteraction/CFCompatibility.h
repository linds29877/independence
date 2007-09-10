/*
 *  CFCompatibility.h
 *  libPhoneInteraction
 *
 *  Created by The Operator on 05/09/07.
 *  Copyright 2007 The Operator. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 2 for more details
 */
#include <CoreFoundation/CoreFoundation.h>


// Used to read a .plist file into a CFDictionaryRef since the QuickTime SDK
// on Windows doesn't contain anything for this.
CFDictionaryRef PICreateDictionaryFromPlistFile(const char *file);
