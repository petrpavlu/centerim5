/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 *
 * This file is part of CenterIM.
 *
 * CenterIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CenterIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

#include "BuddyList.h"

#include "Log.h"
#include "Conf.h"

#include "CenterIM.h"

#include <cppconsui/ConsuiCurses.h>

#include <cppconsui/Keys.h>
#include <cppconsui/Window.h>
#include <cppconsui/TreeView.h>

//TODO remove stuff we dont need here
#include <libpurple/blist.h>
#include <libpurple/prefs.h>
#include <libpurple/core.h>
#include <libpurple/plugin.h>
#include <libpurple/util.h>
#include <libpurple/pounce.h>
#include <libpurple/debug.h>
#include <libpurple/savedstatuses.h>
#include "gettext.h"

#include <glibmm/main.h>

BuddyList* BuddyList::instance = NULL;

BuddyList* BuddyList::Instance(void)
{
	if (!instance) instance = new BuddyList();
	return instance;
}

void BuddyList::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

//TODO move this struct inside the buddylist object
static PurpleBlistUiOps centerim_blist_ui_ops =
{
	BuddyList::new_list_,
	BuddyList::new_node_,
	NULL, /* show */
	BuddyList::update_node_,
	BuddyList::remove_node_,
	BuddyList::destroy_list_,
	NULL, /* set_visible */
	BuddyList::request_add_buddy_,
	BuddyList::request_add_chat_,
	BuddyList::request_add_group_,
	NULL,
	NULL,
	NULL,
	NULL,
};

BuddyList::BuddyList()
: Window(0, 0, 80, 24)
{
	SetColorScheme("buddylist");

	conf = Conf::Instance();

	//TODO check if this has been moved to purple_blist_init
	//renove these lines if it was
	//as this will probably move to purple_init, the buddylist 
	//object should be available a lot more early and the uiops should
	//be set a lot more early. (all in all a lot of work)
	buddylist = purple_blist_new();
	buddylist->ui_data = this;
	purple_set_blist(buddylist);

	// load the pounces
	purple_pounces_load();

	/* setup the callbacks for the buddylist */
	purple_blist_set_ui_ops(&centerim_blist_ui_ops);

	Glib::signal_timeout().connect(sigc::mem_fun(this, &BuddyList::Load), 0);

	treeview = new TreeView(0, 0, width - 2, height - 2);
	AddWidget(*treeview);
	SetInputChild(*treeview);

	MoveResizeRect(conf->GetBuddyListDimensions());
}

bool BuddyList::Load(void)
{
	/* Loads the buddy list from ~/.cim/blist.xml. */
	purple_blist_load();

	return false;
}

BuddyList::~BuddyList()
{
	/* Schedule a save of the blist.xml file. 
	 * this is done automatically by libpurple
	 * when any change occurs, but lets do that anyway
	 */
	//purple_blist_schedule_save(); //TODO: will this go wrong?! (probably)
}

void BuddyList::Close(void)
{
}

void BuddyList::AddNode(BuddyListNode *node)
{
	BuddyListNode *parent = node->GetParentNode();
	node->ref = treeview->AddNode(parent ? parent->ref : treeview->Root(), node, NULL);
}

void BuddyList::UpdateNode(BuddyListNode *node)
{
	BuddyListNode *parent = node->GetParentNode();
	/* The parent could have changed, so re-parent the node */
	if (parent)
		treeview->SetParent(node->ref, parent->ref);

	node->Update();
}

void BuddyList::RemoveNode(BuddyListNode *node)
{
	//TODO check for subnodes (if this is a group for instance)
	treeview->DeleteNode(node->ref, false);
}

void BuddyList::new_list(PurpleBuddyList *list)
{
	if (buddylist != list) {
		//TODO if this happens, then the first todo in Load should
		//be checked again.
		LOG->Write(Log::Level_error, "Different Buddylist detected!\n");
	}
	if (buddylist->ui_data != this)
	{
		//TODO actually, this amounts to the same as the error above this one :).
		LOG->Write(Log::Level_error, "New Buddylist detected, but we only support one buddylist.\n");
	}
}

void BuddyList::new_node(PurpleBlistNode *node)
{
	BuddyListNode *bnode;

	if (!node->ui_data) {
		node->ui_data = bnode = BuddyListNode::CreateNode(node);
		AddNode((BuddyListNode*)node->ui_data);

		if (PURPLE_BLIST_NODE_IS_CONTACT(node)) {
			/* The core seems to expect the UI to add the buddies.
			 * (according to finch)
			 * */
			for (node = node->child; node; node = node->next)
				new_node(node);
		}
	}
}

void BuddyList::update_node(PurpleBuddyList *list, PurpleBlistNode *node)
{
	/* finch does this */
	g_return_if_fail(node != NULL);

	if (!node->ui_data) {
		//TODO remove when this never happens :) (yeah, try to catch that one! :)
		LOG->Write(Log::Level_error, "BuddyList::update called before BuddyList::new_node\n");
		new_node(node);
	}

	/* Update the node data */
	UpdateNode((BuddyListNode*)node->ui_data);

	/* if the node is a buddy, we also have to update the contact
	 * this buddy belongs to.
	 * */
	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		update_node(list, node->parent);
		//TODO is it possible for a nodes parent not to be a contact?
		//if so, then this call could use some checks
	}
}

void BuddyList::remove_node(PurpleBuddyList *list, PurpleBlistNode *node)
{
	BuddyListNode* bnode;

	if (!node->ui_data) return; /* nothing to remove */

	bnode = (BuddyListNode*)node->ui_data;
	RemoveNode(bnode);
	delete bnode;
}

void BuddyList::destroy_list(PurpleBuddyList *list)
{
}

void BuddyList::request_add_buddy(PurpleAccount *account, const char *username, const char *group, const char *alias)
{
	WindowManager::Instance()->Add(new AddBuddyWindow(account, username, group, alias));
}

void BuddyList::MoveResize(int newx, int newy, int neww, int newh)
{
	/* Let parent's Resize() renew data structures (including
	 * the area's of child widgets which will thus be done
	 * twice)
	 * */
	Window::MoveResize(newx, newy, neww, newh);

	/* resize all our widgets, in this case its only one widget
	 * here, w and h are the size of the container, which is 
	 * what we want. in most cases you would need to recalculate
	 * widget sizes based on window and/or container size.
	 * */
	treeview->MoveResize(0, 0, neww - 2, newh - 2);
}

void BuddyList::ScreenResized()
{
	MoveResizeRect(CenterIM::Instance().ScreenAreaSize(CenterIM::BuddyListArea));
}

BuddyList::AccountsBox::AccountsBox(int x, int y, PurpleAccount *account)
: ComboBox(x, y)
, selected(account)
{
	GList *i = purple_accounts_get_all();

	if (!selected && i)
		selected = (PurpleAccount *) i->data;

	for ( ; i; i = i->next) {
		PurpleAccount *account = (PurpleAccount *) i->data;
		gchar *label = g_strdup_printf("[%s] %s",
				purple_account_get_protocol_name(account),
				purple_account_get_username(account));
		AddOption(label, account);
		g_free(label);
	}

	UpdateText();

	signal_selection_changed.connect(
			sigc::mem_fun(this, &BuddyList::AccountsBox::OnAccountChanged));
}

void BuddyList::AccountsBox::UpdateText()
{
	gchar *label = g_strdup_printf("%s: [%s] %s", _("Account"),
			purple_account_get_protocol_name(selected),
			purple_account_get_username(selected));
	SetText(label);
	g_free(label);
}

void BuddyList::AccountsBox::OnAccountChanged(const ComboBox::ComboBoxEntry& new_entry)
{
	selected = (PurpleAccount *) new_entry.data;
	UpdateText();
}

BuddyList::NameButton::NameButton(int x, int y, bool alias, const gchar *val)
: Button(x, y, "")
, dialog(NULL)
{
	if (alias)
		text = _("Alias");
	else
		text = _("Buddy name");

	// value always points to an allocated string
	if (val)
		value = g_strdup(value);
	else
		value = g_strdup("");

	UpdateText();

	signal_activate.connect(sigc::mem_fun(this,
				&BuddyList::NameButton::OnActivate));
}

BuddyList::NameButton::~NameButton()
{
	g_free(value);
}

void BuddyList::NameButton::UpdateText()
{
	gchar *label = g_strdup_printf("%s: %s", text, value);
	SetText(label);
	g_free(label);
}

void BuddyList::NameButton::OnActivate()
{
	g_assert(!dialog);

	WindowManager *wm = WindowManager::Instance();

	dialog = new InputDialog(text, value);
	dialog->signal_response.connect(
			sigc::mem_fun(this, &BuddyList::NameButton::ResponseHandler));
	wm->Add(dialog);
}

void BuddyList::NameButton::ResponseHandler(Dialog::ResponseType response)
{
	g_assert(dialog);

	switch (response) {
		case Dialog::ResponseOK:
			g_free(value);
			value = g_strdup(dialog->GetText());

			UpdateText();
			break;
		default:
			break;
	}
	dialog = NULL;
}

BuddyList::GroupBox::GroupBox(int x, int y, const gchar *group)
: ComboBox(x, y)
{
	PurpleBlistNode *i = purple_blist_get_root();

	if (i == NULL) {
		AddOption(_("Buddies"));
		selected = g_strdup(_("Buddies"));
	}
	else {
		selected = g_strdup(purple_group_get_name(PURPLE_GROUP(i)));
		while (i) {
			if (PURPLE_BLIST_NODE_IS_GROUP(i))
				AddOption(purple_group_get_name(PURPLE_GROUP(i)));
			i = i->next;
		}
	}

	UpdateText();

	signal_selection_changed.connect(
			sigc::mem_fun(this, &BuddyList::GroupBox::OnAccountChanged));
}

BuddyList::GroupBox::~GroupBox()
{
	g_free(selected);
}

void BuddyList::GroupBox::UpdateText()
{
	gchar *label = g_strdup_printf("%s: %s", _("Group"), selected);
	SetText(label);
	g_free(label);
}

void BuddyList::GroupBox::OnAccountChanged(const ComboBox::ComboBoxEntry& new_entry)
{
	g_free(selected);
	selected = g_strdup(new_entry.GetText());
	UpdateText();
}

BuddyList::AddBuddyWindow::AddBuddyWindow(PurpleAccount *account,
		const char *username, const char *group, const char *alias)
: Window(0, 0, 50, 10)
{
	accounts_box = new AccountsBox(0, 0, account);
	name_button = new NameButton(0, 1, false, username);
	alias_button = new NameButton(0, 2, true, alias);
	group_box = new GroupBox(0, 3, group);
	line = new HorizontalLine(0, 4, width);

	menu = new HorizontalListBox(0, 5, width, 1);
	//menu->FocusCycle(Container::FocusCycleLocal);
	menu->AddItem(_("Add"), sigc::mem_fun(this, &BuddyList::AddBuddyWindow::Add));

	AddWidget(*accounts_box);
	AddWidget(*name_button);
	AddWidget(*alias_button);
	AddWidget(*group_box);
	AddWidget(*line);
	AddWidget(*menu);
}

void BuddyList::AddBuddyWindow::Add()
{
	PurpleAccount *account = accounts_box->GetSelected();
	const char *who = name_button->GetValue();
	const char *whoalias = alias_button->GetValue();
	const char *grp = group_box->GetSelected();

	PurpleGroup *g = purple_find_group(grp);
	PurpleBuddy *b = purple_find_buddy_in_group(account, who, g);

	if (!b) {
		b = purple_buddy_new(account, who, whoalias[0] != '\0' ? whoalias : NULL);
		purple_blist_add_buddy(b, NULL, g, NULL);
		purple_account_add_buddy(account, b);
	}

	Close();
}
