/*
 *  PhoneInteraction.h
 *  libPhoneInteraction
 *
 *  Created by The Operator on 20/07/07.
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

#pragma once

#if defined(WIN32)
#include <CoreFoundation.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "PIVersion.h"

/* Our own version of afc when in jailbreak mode */
#define AMSVC_AFC2					CFSTR("com.apple.afc2")


// enumeration of messages sent to applications that register for notifcation
// callbacks from PhoneInteraction
enum
{
	NOTIFY_INITIALIZATION_SUCCESS = 100,
	NOTIFY_INITIALIZATION_FAILED,
	NOTIFY_CONNECTION_SUCCESS,
	NOTIFY_CONNECTION_FAILED,
	NOTIFY_AFC_CONNECTION_SUCCESS,
	NOTIFY_AFC_CONNECTION_FAILED,
	NOTIFY_ACTIVATION_SUCCESS,
	NOTIFY_ACTIVATION_FAILED,
	NOTIFY_DEACTIVATION_SUCCESS,
	NOTIFY_DEACTIVATION_FAILED,
	NOTIFY_PUTFILE_SUCCESS,
	NOTIFY_PUTFILE_FAILED,
	NOTIFY_GETFILE_SUCCESS,
	NOTIFY_GETFILE_FAILED,
	NOTIFY_PUTPEM_SUCCESS,
	NOTIFY_PUTPEM_FAILED,
	NOTIFY_PUTSERVICES_SUCCESS,
	NOTIFY_PUTSERVICES_FAILED,
	NOTIFY_PUTFSTAB_SUCCESS,
	NOTIFY_PUTFSTAB_FAILED,
	NOTIFY_CREATE_DIR_SUCCESS,
	NOTIFY_CREATE_DIR_FAILED,
	NOTIFY_REMOVE_PATH_SUCCESS,
	NOTIFY_REMOVE_PATH_FAILED,
	NOTIFY_RENAME_PATH_SUCCESS,
	NOTIFY_RENAME_PATH_FAILED,
	NOTIFY_CONNECTED,
	NOTIFY_DISCONNECTED,
	NOTIFY_AFC_CONNECTED,
	NOTIFY_AFC_DISCONNECTED,
	NOTIFY_JAILBREAK_FAILED,
	NOTIFY_JAILBREAK_SUCCESS,
	NOTIFY_JAILBREAK_CANCEL,
	NOTIFY_JAILBREAK_RECOVERY_WAIT,
	NOTIFY_NEW_JAILBREAK_STAGE_ONE_WAIT,
	NOTIFY_NEW_JAILBREAK_STAGE_TWO_WAIT,
	NOTIFY_RECOVERY_CONNECTED,
	NOTIFY_RECOVERY_DISCONNECTED,
	NOTIFY_RESTORE_CONNECTED,
	NOTIFY_RESTORE_DISCONNECTED,
	NOTIFY_DFU_CONNECTED,
	NOTIFY_DFU_DISCONNECTED,
	NOTIFY_DFU_RECOVERY_WAIT,
	NOTIFY_DFU_FAILED,
	NOTIFY_DFU_SUCCESS,
	NOTIFY_RECOVERY_FAILED,
	NOTIFY_RECOVERY_SUCCESS,
	NOTIFY_JAILRETURN_FAILED,
	NOTIFY_JAILRETURN_SUCCESS,
	NOTIFY_JAILRETURN_CANCEL,
	NOTIFY_JAILRETURN_RECOVERY_WAIT,
	NOTIFY_FSCOPY_FAILED,
	NOTIFY_FSCOPY_SUCCESS,
	NOTIFY_WIN32_INITIALIZATION_FAILED,
	NOTIFY_GET_ACTIVATION_FAILED,
	NOTIFY_GET_ACTIVATION_SUCCESS
};

// data types from MobileDevice.h (can't directly include it here)
struct am_device;
struct afc_connection;
struct am_recovery_device;
struct am_restore_device;

typedef int (*cmdsend)(am_recovery_device *, const char *);


/*
 * PhoneInteraction is the main class used to perform operations on the iPhone.
 *
 * It uses the singleton design pattern since an application should only ever
 * need one instance of it.  So use getInstance() to instantiate it.
 */
class PhoneInteraction
{

public:
	~PhoneInteraction();

	// singleton pattern for object instantiation
	static PhoneInteraction* getInstance(void (*statusFunc)(const char*, bool) = NULL,
										 void (*notifyFunc)(int, const char*) = NULL,
										 bool bUsingPrivateFunctions = true);

	// connection related functions
	void connectToPhone();
	void disconnectFromPhone();
	void setConnected(bool connected);
	bool isConnected();

	// AFC connection related functions
	void connectToAFC();
	void setConnectedToAFC(bool connected);
	bool isConnectedToAFC();

	// information functions
	bool readValue(const char *key, char **value);
	char *getPhoneFirmwareVersion();
	char *getPhoneProductVersion();
	char *getPhoneBuildVersion();
	char *getPhoneBasebandVersion();

	// ringtone related functions
	char *getUserRingtoneName(const char *filename);
	bool getUserRingtoneFilenames(const char *ringtoneName, char ***filenames, int *numFiles);

	// activation related functions
	bool activate(const char* filename, const char* pemfile = NULL);
	bool factoryActivate(bool undo = false);
	bool deactivate();
	bool isPhoneActivated();

	// jailbreak related functions
	void performJailbreak(const char *firmwarePath, const char *modifiedFstabPath,
						  const char *modifiedServicesPath);
	void performNewJailbreak(const char *modifiedServicesPath);
	void newJailbreakStageTwo();
	void returnToJail(const char *servicesFile, const char *fstabFile);
	void jailbreakFinished();
	void returnToJailFinished();
	bool isPhoneJailbroken();

	// use to enable SpringBoard to recognize 3rd party applications on firmware 1.1.x
	bool enableThirdPartyApplications(bool undo = false);

	// use to enable/disable YouTube
	bool enableYouTube(bool undo = false);

	// use to enable ringtone customization on firmware 1.1.1
	bool enableCustomRingtones(bool undo = false);

	// used for notifications from MobileDevice library
	void recoveryModeStarted(struct am_recovery_device *dev);
	void recoveryModeFinished(struct am_recovery_device *dev);
	void exitRecoveryMode(am_recovery_device *dev);

	void restoreModeStarted();
	void restoreModeFinished();

	void dfuModeStarted(am_recovery_device *dev);
	void dfuModeFinished(am_recovery_device *dev);

	// used to switch phone modes
	void enterDFUMode(const char *firmwarePath);
	void enterRecoveryMode();

	// lower level file I/O
	bool putData(void *data, int len, char *dest, int failureMsg = NOTIFY_PUTFILE_FAILED,
				 int successMsg = NOTIFY_PUTFILE_SUCCESS);
	bool putFile(const char *src, char *dest, int failureMsg = NOTIFY_PUTFILE_FAILED,
				 int successMsg = NOTIFY_PUTFILE_SUCCESS);
	bool putFileRecursive(const char *filepath, char *destpath, int failureMsg = NOTIFY_PUTFILE_FAILED,
						  int successMsg = NOTIFY_PUTFILE_SUCCESS);

	int getFileSize(const char *path);
	bool getFileData(void **buf, int *size, const char *file,
					 int failureMsg = NOTIFY_GETFILE_FAILED, int successMsg = NOTIFY_GETFILE_SUCCESS);
	bool getFile(const char *src, const char *dest, int failureMsg = NOTIFY_GETFILE_FAILED,
				 int successMsg = NOTIFY_GETFILE_SUCCESS);

	bool createDirectory(const char *dir);
	bool removePath(const char *path);
	bool renamePath(const char *oldpath, const char *newpath);

	int numFilesInDirectory(const char *path);
	bool directoryFileList(const char *path, char ***list, int *length);
	bool isDirectory(const char *path);

	bool fileExists(const char *path);

	// high level file I/O
	bool putPEMOnPhone(const char *pemFile);
	bool putServicesFileOnPhone(const char *servicesFile);
	bool putFstabFileOnPhone(const char *fstabFile);
	bool putRingtoneOnPhone(const char *ringtoneFile, const char *ringtoneName = NULL, bool bInSystemDir = false);
	bool putWallpaperOnPhone(const char *wallpaperFile, const char *thumbnailFile, bool bInSystemDir = false);

	// This works, but it won't set the file permissions correctly since there doesn't appear
	// to be any way to do that using the MobileDevice library.  Use SSHHelper to do that.
	bool putApplicationOnPhone(const char *applicationDir);

	bool removeRingtone(const char *ringtoneFilename, bool bInSystemDir = false);
	bool removeWallpaper(const char *wallpaperFilename, bool bInSystemDir = false);
	bool removeApplication(const char *applicationFilename);

	bool ringtoneExists(const char *ringtoneFile, bool bInSystemDir = false);
	bool wallpaperExists(const char *wallpaperFile, bool bInSystemDir = false);
	bool applicationExists(const char *applicationName);

	bool getActivationFile(const char *dest);
	bool copyPhoneFilesystem(const char *dirpath, const char *dest, bool ignoreUserFiles);

	// public data members
	bool m_switchingToRestoreMode;
	bool m_inRestoreMode;
	bool m_inDFUMode;
	bool m_returningToJail;
	bool m_finishingJailbreak;
	bool m_performingNewJailbreak;
	bool m_recoveryOccurred;
	am_recovery_device *m_recoveryDevice;
	struct am_restore_device *m_restoreDevice;
	am_recovery_device *m_dfuDevice;
	am_device *m_iPhone;

private:
	// private constructor (use getInstance to instantiate)
	PhoneInteraction(void (*statusFunc)(const char*, bool),
					 void (*notifyFunc)(int, const char*),
					 bool bUsingPrivateFunctions);

	// functions used internally
	void subscribeToNotifications();	
	bool determineiTunesVersion();
	void setupPrivateFunctions();
	bool arePrivateFunctionsSetup();
	bool copyFilesystemRecursive(afc_connection *conn, const char *phoneBasepath,
								 const char *dirpath, const char *dest,
								 const char *basepath, bool ignoreUserFiles);
	bool removePathRecursive(const char *path);
	bool writeDataToFile(void *buf, int size, const char *file, int failureMsg = 0, int successMsg = 0);
	void recoveryModeStarted_dfu(struct am_recovery_device *rdev);
	CFDictionaryRef getUserRingtoneDictionary();

	// private data members
	bool m_connected;
	bool m_afcConnected;
	bool m_inRecoveryMode;
	bool m_jailbroken;
	bool m_enteringRecoveryMode;
	bool m_enteringDFUMode;
	bool m_waitingForRecovery;
	bool m_usingPrivateFunctions;
	afc_connection *m_hAFC;
	void (*m_statusFunc)(const char*, bool);
	void (*m_notifyFunc)(int, const char*);
	char *m_firmwarePath;
	bool m_privateFunctionsSetup;
	PIVersion m_iTunesVersion;
	char *m_firmwareVersion;
	char *m_productVersion;
	char *m_buildVersion;
	char *m_basebandVersion;
	char *m_servicesPath;

};
