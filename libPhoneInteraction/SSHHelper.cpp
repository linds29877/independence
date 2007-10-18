/*
 *  SSHHelper.cpp
 *  libPhoneInteraction
 *
 *  Created by The Operator on 31/08/07.
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

#include "SSHHelper.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>


static const char *g_sshPath = "/usr/bin/ssh";
static const char *g_sftpPath = "/usr/bin/sftp";


static int modeToInt(mode_t mode)
{
	int retval = 0;

	if (mode & S_IXUSR) {
		retval += 100;
	}

	if (mode & S_IWUSR) {
		retval += 200;
	}

	if (mode & S_IRUSR) {
		retval += 400;
	}

	if (mode & S_IXGRP) {
		retval += 10;
	}

	if (mode & S_IWGRP) {
		retval += 20;
	}
	
	if (mode & S_IRGRP) {
		retval += 40;
	}
	
	if (mode & S_IXOTH) {
		retval += 1;
	}
	
	if (mode & S_IWOTH) {
		retval += 2;
	}
	
	if (mode & S_IROTH) {
		retval += 4;
	}
	
	return retval;
}

static char *escapeSpaces(const char *path)
{
	char *escapedPath;
	int numSpaces = 0;
	int index = 0;
	int len = strlen(path);

	// count the number of spaces
	while (index < len) {

		if (path[index++] == ' ') {
			numSpaces++;
		}

	}

	escapedPath = (char*)malloc(len+1+(numSpaces*2));
	index = 0;
	int index2 = 0;

	while (index < len) {

		if (path[index] == ' ') {
			strncpy(escapedPath+index2, "\\\\", 2);
			index2 += 2;
		}
		
		escapedPath[index2++] = path[index++];	
	}

	escapedPath[index2] = 0;
	return escapedPath;
}

static FILE *buildInitialSSHScript(const char *ipAddress, const char *password, char **filename)
{
	size_t len = 30 + strlen(g_sshPath) + strlen(ipAddress);
	size_t len2 = 15 + strlen(password);
	char cmd[len], cmd2[len2];
	char *tmpName = tmpnam(NULL);
	
	if (tmpName == NULL) {
		return NULL;
	}
	
	FILE *fp = fopen(tmpName, "w");
	
	if (fp == NULL) {
		return NULL;
	}
	
	fputs("#!/usr/bin/expect -f\n", fp);
	fputs("log_user 0\n", fp);
	fputs("set timeout 120\n\n", fp);
	snprintf(cmd, len, "if { ![spawn \"%s\" \"root@%s\"] } {\n", g_sshPath, ipAddress);
	fputs(cmd, fp);
	fputs("    exit -1\n", fp);
	fputs("}\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"(yes/no)?\"    { exp_send \"yes\n\"\n", fp);
	fputs("                       expect {\n", fp);
	fputs("                           timeout          { exit 1 }\n", fp);
	fputs("                           eof              { exit 1 }\n", fp);
	fputs("                           \"password:\"\n", fp);
	fputs("                       }\n", fp);
	fputs("                     }\n", fp);
	fputs("    \"failed.\"      { exit 3 }\n", fp);
	fputs("    \"password:\"\n", fp);
	fputs("}\n\n", fp);
	snprintf(cmd2, len2, "exp_send \"%s\n\"\n\n", password);
	fputs(cmd2, fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"again.\"     { exit 2 }\n", fp);
	fputs("    \"#\"\n", fp);
	fputs("}\n\n", fp);

	*filename = tmpName;
	return fp;
}

static FILE *buildInitialSFTPScript(const char *ipAddress, const char *password, char **filename)
{
	size_t len = 30 + strlen(g_sftpPath) + strlen(ipAddress);
	size_t len2 = 15 + strlen(password);
	char cmd[len], cmd2[len2];
	char *tmpName = tmpnam(NULL);
	
	if (tmpName == NULL) {
		return NULL;
	}
	
	FILE *fp = fopen(tmpName, "w");
	
	if (fp == NULL) {
		return NULL;
	}
	
	fputs("#!/usr/bin/expect -f\n", fp);
	fputs("log_user 0\n", fp);
	fputs("set timeout 120\n\n", fp);
	snprintf(cmd, len, "if { ![spawn \"%s\" \"root@%s\"] } {\n", g_sftpPath, ipAddress);
	fputs(cmd, fp);
	fputs("    exit -1\n", fp);
	fputs("}\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"(yes/no)?\"    { exp_send \"yes\n\"\n", fp);
	fputs("                       expect {\n", fp);
	fputs("                           timeout          { exit 1 }\n", fp);
	fputs("                           eof              { exit 1 }\n", fp);
	fputs("                           \"password:\"\n", fp);
	fputs("                       }\n", fp);
	fputs("                     }\n", fp);
	fputs("    \"failed.\"      { exit 3 }\n", fp);
	fputs("    \"password:\"\n", fp);
	fputs("}\n\n", fp);
	snprintf(cmd2, len2, "exp_send \"%s\n\"\n\n", password);
	fputs(cmd2, fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"again.\"     { exit 2 }\n", fp);
	fputs("    \">\"\n", fp);
	fputs("}\n\n", fp);

	*filename = tmpName;
	return fp;
}

static bool buildPermissionsScriptRecursive(FILE *fp, const char *srcPath, const char *destPath)
{
	struct stat st;

	if (stat(srcPath, &st) == -1) {
		return false;
	}

	if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)) {
		char *escapedDestPath = escapeSpaces(destPath);
		int mode = modeToInt(st.st_mode);
		int len = 25 + strlen(escapedDestPath);
		char cmd[len];

		snprintf(cmd, len, "exp_send \"chmod %d %s\n\"\n\n", mode, escapedDestPath);
		fputs(cmd, fp);
		fputs("expect {\n", fp);
		fputs("    timeout          { exit 1 }\n", fp);
		fputs("    eof              { exit 1 }\n", fp);
		fputs("    \"#\"\n", fp);
		fputs("}\n\n", fp);

		free(escapedDestPath);
	}

	if (S_ISDIR(st.st_mode)) {
		DIR *dp;

		if ( (dp = opendir(srcPath)) == NULL ) {
			return false;
		}

		struct dirent *dirp;
		char newSrc[PATH_MAX+1], newDest[PATH_MAX+1];
		int len = strlen(srcPath), len2 = strlen(destPath);

		strcpy(newSrc, srcPath);
		strcpy(newDest, destPath);

		if (newSrc[len-1] != '/') {
			strcat(newSrc, "/");
			len += 1;
		}

		if (newDest[len2-1] != '/') {
			strcat(newDest, "/");
			len2 += 1;
		}

		while ( (dirp = readdir(dp)) != NULL ) {

			if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) continue;

			strcpy(newSrc + len, dirp->d_name);
			strcpy(newDest + len2, dirp->d_name);

			if (!buildPermissionsScriptRecursive(fp, newSrc, newDest)) return false;

		}

	}

	return true;
}

int SSHHelper::copyPermissions(const char *srcPath, const char *destPath,
							   const char *ipAddress, const char *password,
							   bool bRestartSpringboard)
{
	char *filename;
	FILE *fp = buildInitialSSHScript(ipAddress, password, &filename);
	
	if (fp == NULL) {
		return -1;
	}

	if (!buildPermissionsScriptRecursive(fp, srcPath, destPath)) {
		fflush(fp);
		fclose(fp);
		remove(filename);
		return -1;
	}

	if (bRestartSpringboard) {
		fputs("exp_send \"launchctl stop com.apple.SpringBoard\n\"\n\n", fp);
		fputs("expect {\n", fp);
		fputs("    timeout          { exit 1 }\n", fp);
		fputs("    eof              { exit 1 }\n", fp);
		fputs("    \"#\"\n", fp);
		fputs("}\n\n", fp);
	}

	fputs("exp_send \"exit\n\"\n", fp);
	fputs("exit 0\n", fp);
	fflush(fp);
	fclose(fp);

	if (chmod(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		remove(filename);
		return -1;
	}

	int retval = system(filename);
	remove(filename);
	return retval;
}

int SSHHelper::restartSpringboard(const char *ipAddress, const char *password)
{
	char *filename;
	FILE *fp = buildInitialSSHScript(ipAddress, password, &filename);

	if (fp == NULL) {
		return -1;
	}

	fputs("exp_send \"launchctl stop com.apple.SpringBoard\n\"\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"#\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"exit\n\"\n", fp);
	fputs("exit 0\n", fp);
	fflush(fp);
	fclose(fp);
	
	if (chmod(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		remove(filename);
		return -1;
	}
	
	int retval = system(filename);
	remove(filename);
	return retval;
}

int SSHHelper::restartLockdown(const char *ipAddress, const char *password)
{
	char *filename;
	FILE *fp = buildInitialSSHScript(ipAddress, password, &filename);
	
	if (fp == NULL) {
		return -1;
	}
	
	fputs("exp_send \"launchctl stop com.apple.mobile.lockdown\n\"\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"#\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"exit\n\"\n", fp);
	fputs("exit 0\n", fp);
	fflush(fp);
	fclose(fp);
	
	if (chmod(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		remove(filename);
		return -1;
	}
	
	int retval = system(filename);
	remove(filename);
	return retval;
}

int SSHHelper::launchApplication(const char *ipAddress, const char *password,
								 const char *applicationID)
{
	char *filename;
	FILE *fp = buildInitialSSHScript(ipAddress, password, &filename);
	
	if (fp == NULL) {
		return -1;
	}
	
	int len = 31 + strlen(applicationID);
	char cmd[len];

	snprintf(cmd, len, "exp_send \"launchctl start %s\n\"\n\n", applicationID);
	fputs(cmd, fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \"#\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"exit\n\"\n", fp);
	fputs("exit 0\n", fp);
	fflush(fp);
	fclose(fp);
	
	if (chmod(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		remove(filename);
		return -1;
	}

	int retval = system(filename);
	remove(filename);
	return retval;
}

int SSHHelper::symlinkMediaToRoot(const char *ipAddress, const char *password)
{
	char *filename;
	FILE *fp = buildInitialSFTPScript(ipAddress, password, &filename);
	
	if (fp == NULL) {
		return -1;
	}

	fputs("exp_send \"rename /private/var/root/Media /private/var/root/Media.iNdependence\n\"\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \">\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"symlink / /private/var/root/Media\n\"\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \">\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"quit\n\"\n", fp);
	fputs("exit 0\n", fp);
	fflush(fp);
	fclose(fp);
	
	if (chmod(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		remove(filename);
		return -1;
	}
	
	int retval = system(filename);
	remove(filename);
	return retval;
}

int SSHHelper::removeMediaSymlink(const char *ipAddress, const char *password)
{
	char *filename;
	FILE *fp = buildInitialSFTPScript(ipAddress, password, &filename);
	
	if (fp == NULL) {
		return -1;
	}
	
	fputs("exp_send \"rm /private/var/root/Media\n\"\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \">\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"rename /private/var/root/Media.iNdependence /private/var/root/Media\n\"\n\n", fp);
	fputs("expect {\n", fp);
	fputs("    timeout          { exit 1 }\n", fp);
	fputs("    eof              { exit 1 }\n", fp);
	fputs("    \">\"\n", fp);
	fputs("}\n\n", fp);
	fputs("exp_send \"quit\n\"\n", fp);
	fputs("exit 0\n", fp);
	fflush(fp);
	fclose(fp);
	
	if (chmod(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		remove(filename);
		return -1;
	}
	
	int retval = system(filename);
	remove(filename);
	return retval;
}
