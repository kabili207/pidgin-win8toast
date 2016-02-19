/**
 * @mainpage
 * @author  Kabili < kabili207@gmail.com
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * A plugin which interfaces the Logitech GamePanel LCD with Pidgin.
 * These LCDs are currently found in the G15 Keyboards and G13 Gamepads.
 *
 * The pidgin-libnotify plugin was used as a base for the interface with libpurple.
 * pidgin-libnotify is available at http://gaim-libnotify.sourceforge.net/
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */


#define PURPLE_PLUGINS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <plugin.h>
#include <notify.h>
#include <version.h>
#include <debug.h>
#include <util.h>
#include <privacy.h>
#include <glib.h>
#include <account.h>

#define _WIN32_WINNT 0x0602
#include <windows.h>
#include <shlwapi.h>
#include <Shobjidl.h>

#include "ToastNotification.h"

#ifndef G_GNUC_NULL_TERMINATED
#  if __GNUC__ >= 4
#    define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#  else
#    define G_GNUC_NULL_TERMINATED
#  endif
#endif

#define PLUGIN_ID       "gtk-win32-kabili207-wintoast"
#define PLUGIN_NAME     "Purple Toast"
#define PLUGIN_VERSION  "0.7"
#define SUMMARY         "Displays native Windows 8 toasts"
#define DESCRIPTION     "Windows 8 Toasts notifications"
#define AUTHOR          "Andrew Nagle <kabili@zyrenth.com>"
#define WEBSITE         "http://www.zyrenth.com"

#define PIDGIN_APPID   L"Pidgin.WinToastPlugin"
#define PIDGIN_APP_NAME L"pidgin"

static BOOL mEnabled = false;

static void
click_callback()
{

}

static void
dismiss_callback()
{

}

static gchar *
best_name(PurpleBuddy *buddy)
{
	if (buddy->alias) {
		return buddy->alias;
	}
	else if (buddy->server_alias) {
		return buddy->server_alias;
	}
	else {
		return buddy->name;
	}
}

static wchar_t*
gchar_to_wchar_t(const gchar* s)
{
	if (!s)
		return NULL;
	g_assert(sizeof(wchar_t) == sizeof(gunichar2));
	wchar_t* wc_s = reinterpret_cast<wchar_t*>(g_utf8_to_utf16(s, -1, NULL, NULL, NULL));
	return wc_s;
}

static void
display_toast(const gchar* title, const gchar* body, const gchar* image)
{
	purple_debug_info(PLUGIN_ID, "title: '%s', body: '%s'\n", title, body);

	wchar_t* wc_title = gchar_to_wchar_t(title);
	wchar_t* wc_body = gchar_to_wchar_t(body);
	wchar_t* wc_icon = gchar_to_wchar_t(image);

	BOOL retVal = DisplayToastNotification(wc_icon, wc_title, wc_body, PIDGIN_APP_NAME, (EventCallback)&click_callback, (EventCallback)&dismiss_callback);

	if (!retVal) {
		purple_debug_error(PLUGIN_ID, "Could not display notification\n");
	}

	g_free(wc_title);
	g_free(wc_body);
	g_free(wc_icon);

}

static void
notify_msg_sent(PurpleAccount *account, const gchar *sender, const gchar *message)
{
	PurpleBuddy *buddy = purple_find_buddy(account, sender);
	if (!buddy)
		return;

	char *title;
	title = g_strdup(best_name(buddy));
	char *body = purple_markup_strip_html(message);
	char *icon = purple_buddy_icon_get_full_path(buddy->icon);

	display_toast(title, body, icon);

	g_free(title);
	g_free(body);

}

static void
notify_new_message_cb(PurpleAccount *account, const gchar *sender, const gchar *message, int flags, gpointer data)
{

	notify_msg_sent(account, sender, message);
}

static void
notify_chat_nick(PurpleAccount *account, const gchar *sender, const gchar *message, PurpleConversation *conv, gpointer data)
{
	gchar *nick;

	nick = (gchar *)purple_conv_chat_get_nick(PURPLE_CONV_CHAT(conv));
	if (nick && !strcmp(sender, nick))
		return;

	if (!g_strstr_len(message, strlen(message), nick))
		return;

	notify_msg_sent(account, sender, message);
}

BOOL freeResult, runTimeLinkSuccess = FALSE;

static void
init_plugin(PurplePlugin *plugin)
{

}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	void *conv_handle;

	conv_handle = purple_conversations_get_handle();

	SetCurrentProcessExplicitAppUserModelID(PIDGIN_APPID);

	mEnabled = SetAppId();

	if (!mEnabled) {
		purple_debug_error(PLUGIN_ID, "Application ID not found\n");
	}
	else {
		purple_debug_info(PLUGIN_ID, "Application ID found\n");

		purple_signal_connect(conv_handle, "received-im-msg", plugin, PURPLE_CALLBACK(notify_new_message_cb), NULL);

		purple_signal_connect(conv_handle, "received-chat-msg", plugin, PURPLE_CALLBACK(notify_chat_nick), NULL);
	}

	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	void *conv_handle;

	conv_handle = purple_conversations_get_handle();

	if (mEnabled) {
		purple_signal_disconnect(conv_handle, "received-im-msg", plugin, PURPLE_CALLBACK(notify_new_message_cb));

		purple_signal_disconnect(conv_handle, "received-chat-msg", plugin, PURPLE_CALLBACK(notify_chat_nick));
	}

	return TRUE;
}


static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,        /* magic number */
	PURPLE_MAJOR_VERSION,       /* purple major */
	PURPLE_MINOR_VERSION,       /* purple minor */
	PURPLE_PLUGIN_STANDARD,     /* plugin type */
	NULL,                       /* UI requirement */
	0,                          /* flags */
	NULL,                       /* dependencies */
	PURPLE_PRIORITY_DEFAULT,    /* priority */

	PLUGIN_ID,                  /* id */
	PLUGIN_NAME,                /* name */
	PLUGIN_VERSION,             /* version */
	SUMMARY,                    /* summary */
	DESCRIPTION,                /* description */
	AUTHOR,                     /* author */
	WEBSITE,                    /* homepage */

	plugin_load,                /* load */
	plugin_unload,              /* unload */
	NULL,                       /* destroy */

	NULL,                       /* ui info */
	NULL,                       /* extra info */
	NULL,                       /* prefs info */
	NULL,                       /* actions */
	NULL,                       /* reserved */
	NULL,                       /* reserved */
	NULL,                       /* reserved */
	NULL                        /* reserved */
};

PURPLE_INIT_PLUGIN(purple-toast, init_plugin, info)
